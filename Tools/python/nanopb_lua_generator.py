#!/usr/bin/python

'''Generate lua serialiser code from nanopb-compatible protocol buffer files.'''
lua_generator_version = "luagenerator-0.1"

import sys
import zlib
import os.path

# Kinda nasty... hardcoding the path to nanopb's generator stuff.
# This is because I couldn't figure out how to set the PYTHONPATH
# variable externally in windows :-(
scriptdir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.abspath(scriptdir + '\\..\\..\\Engine\\ThirdParty\\nanopb\\generator'))

try:
    # Add some dummy imports to keep packaging tools happy.
    import google, distutils.util # bbfreeze seems to need these
    import pkg_resources # pyinstaller / protobuf 2.5 seem to need these
except:
    # Don't care, we will error out later if it is actually important.
    pass

try:
    import google.protobuf.text_format as text_format
    import google.protobuf.descriptor_pb2 as descriptor
except:
    sys.stderr.write('''
         *************************************************************
         *** Could not import the Google protobuf Python libraries ***
         *** Try installing package 'python-protobuf' or similar.  ***
         *************************************************************
    ''' + '\n')
    raise

try:
    import proto.nanopb_pb2 as nanopb_pb2
    import proto.plugin_pb2 as plugin_pb2
except:
    sys.stderr.write('''
         ********************************************************************
         *** Failed to import the protocol definitions for generator.     ***
         *** You have to run 'make' in the nanopb/generator/proto folder. ***
         ********************************************************************
    ''' + '\n')
    raise

# ---------------------------------------------------------------------------
#                     Generation of single fields
# ---------------------------------------------------------------------------

import re
import time

# Values are tuple (c type, pb type, encoded size, int_size_allowed)
FieldD = descriptor.FieldDescriptorProto
datatypes = {
    FieldD.TYPE_BOOL:       ('bool',     'BOOL',        1,  False),
    FieldD.TYPE_DOUBLE:     ('double',   'DOUBLE',      8,  False),
    FieldD.TYPE_FIXED32:    ('uint32_t', 'FIXED32',     4,  False),
    FieldD.TYPE_FIXED64:    ('uint64_t', 'FIXED64',     8,  False),
    FieldD.TYPE_FLOAT:      ('float',    'FLOAT',       4,  False),
    FieldD.TYPE_INT32:      ('int32_t',  'INT32',      10,  True),
    FieldD.TYPE_INT64:      ('int64_t',  'INT64',      10,  True),
    FieldD.TYPE_SFIXED32:   ('int32_t',  'SFIXED32',    4,  False),
    FieldD.TYPE_SFIXED64:   ('int64_t',  'SFIXED64',    8,  False),
    FieldD.TYPE_SINT32:     ('int32_t',  'SINT32',      5,  True),
    FieldD.TYPE_SINT64:     ('int64_t',  'SINT64',     10,  True),
    FieldD.TYPE_UINT32:     ('uint32_t', 'UINT32',      5,  True),
    FieldD.TYPE_UINT64:     ('uint64_t', 'UINT64',     10,  True)
}

# Integer size overrides (from .proto settings)
intsizes = {
    nanopb_pb2.IS_8:     'int8_t',
    nanopb_pb2.IS_16:    'int16_t',
    nanopb_pb2.IS_32:    'int32_t',
    nanopb_pb2.IS_64:    'int64_t',
}

class Names:
    '''Keeps a set of nested names and formats them to C identifier.'''
    def __init__(self, parts = (), namespaces = []):
        if isinstance(parts, Names):
            parts = parts.parts
            if not namespaces:
                namespaces = parts.namespaces
        self.parts = tuple(parts)
        self.namespaces = namespaces
    
    def __str__(self):
        return '_'.join(self.parts)

    def __add__(self, other):
        if isinstance(other, (str, unicode)):
            return Names(self.parts + (other,), self.namespaces)
        elif isinstance(other, tuple):
            return Names(self.parts + other, self.namespaces)
        else:
            raise ValueError("Name parts should be of type str")
    
    def __eq__(self, other):
        return isinstance(other, Names) and self.parts == other.parts

    def ns_prefix(self, delimiter = '::'):
        result = ''

        if self.namespaces:
            result = delimiter.join(self.namespaces) + delimiter

            if result.startswith('usg'):
                result = delimiter + result

        return result

def names_from_type_name(type_name):
    '''Parse Names() from FieldDescriptorProto type_name'''
    if type_name[0] != '.':
        raise NotImplementedError("Lookup of non-absolute type names is not supported")
    return Names(type_name[1:].split('.'))

def ns_prefix(ctype, delimiter = '::'):
    return ctype.ns_prefix(delimiter) if isinstance(ctype, Names) else ''

def varint_max_size(max_value):
    '''Returns the maximum number of bytes a varint can take when encoded.'''
    for i in range(1, 11):
        if (max_value >> (i * 7)) == 0:
            return i
    raise ValueError("Value too large for varint: " + str(max_value))

assert varint_max_size(0) == 1
assert varint_max_size(127) == 1
assert varint_max_size(128) == 2

class EncodedSize:
    '''Class used to represent the encoded size of a field or a message.
    Consists of a combination of symbolic sizes and integer sizes.'''
    def __init__(self, value = 0, symbols = []):
        if isinstance(value, (str, Names)):
            symbols = [ns_prefix(value) + str(value)]
            value = 0
        self.value = value
        self.symbols = symbols
    
    def __add__(self, other):
        if isinstance(other, (int, long)):
            return EncodedSize(self.value + other, self.symbols)
        elif isinstance(other, (str, Names)):
            return EncodedSize(self.value, self.symbols + [ns_prefix(other) + str(other)])
        elif isinstance(other, EncodedSize):
            return EncodedSize(self.value + other.value, self.symbols + other.symbols)
        else:
            raise ValueError("Cannot add size: " + repr(other))

    def __mul__(self, other):
        if isinstance(other, (int, long)):
            return EncodedSize(self.value * other, [str(other) + '*' + s for s in self.symbols])
        else:
            raise ValueError("Cannot multiply size: " + repr(other))

    def __str__(self):
        if not self.symbols:
            return str(self.value)
        else:
            return '(' + str(self.value) + ' + ' + ' + '.join(self.symbols) + ')'

    def upperlimit(self):
        if not self.symbols:
            return self.value
        else:
            return 2**32 - 1

class Enum:
    def __init__(self, names, desc, enum_options):
        '''desc is EnumDescriptorProto'''
        
        self.options = enum_options
        self.names = names + desc.name

        self.lua_generate = False
        if enum_options.HasField("lua_generate"):
            self.lua_generate = True
        
        if enum_options.long_names:
            self.values = [(self.names + x.name, x.number) for x in desc.value]            
        else:
            self.values = [(names + x.name, x.number) for x in desc.value] 
        
        self.value_longnames = [self.names + x.name for x in desc.value]
        self.count = len( self.values )

    def decl(self):
        fullname = ns_prefix(self.names) + str(self.names)
        if not fullname.startswith('::'):
            fullname = '::' + fullname

        result = 'template<>\n'
        result += 'struct LuaSerializer< %s >\n' % fullname
        result += '{\n'
        result += '\tstatic const bool GENERATE = %s;\n' % ("true" if self.lua_generate else "false")
        result += '\tstatic int PushToLua(lua_State* L, const %s& data);\n' % fullname
        result += '\tstatic int GetFromLua(lua_State* L, int index, %s* data);\n' % fullname
        result += '\tstatic void Init(lua_State* L);\n'
        result += '\tstatic const %s& CheckEnum(lua_State* L, int arg);\n' % fullname
        result += '\tstatic int ToString(lua_State* L);\n'
        result += '\tstatic int ToNumber(lua_State* L);\n'
        result += '\tstatic int Eq(lua_State* L);\n'
        result += '};\n'

        return result
    
    def __str__(self):
        fullname = ns_prefix(self.names) + str(self.names)
        if not fullname.startswith('::'):
            fullname = '::' + fullname

        luaname = ns_prefix(self.names, '.') + str(self.names)
        if luaname.startswith('.'):
            luaname = luaname[1:]

        metatablename = luaname

        result = 'int ::usg::LuaSerializer< %s >::PushToLua(lua_State* L, const %s& data)\n' % (fullname, fullname)
        result += '{\n'
        result += '\tbool bIsRegistered = luaL_getmetatable(L, "%s") != LUA_TNIL;\n' % metatablename
        result += '\tlua_pop(L, 1);\n'
        result += '\tASSERT(bIsRegistered);\n'
        result += '\n'
        result += '\t%s* dataPtr = (%s*)lua_newuserdata(L, sizeof(%s));\n' % (fullname, fullname, fullname)
        result += '\tluaL_setmetatable(L, "%s");\n' % metatablename
        result += '\t*dataPtr = data;\n'
        result += '\treturn 1;\n'
        result += '}\n'
        result += '\n'
        result += 'int ::usg::LuaSerializer< %s >::GetFromLua(lua_State* L, int index, %s* data)\n' % (fullname, fullname)
        result += '{\n'
        result += '\t*data = CheckEnum(L, index);\n'
        result += '\treturn 0;\n'
        result += '}\n'
        result += '\n'

        result += 'void ::usg::LuaSerializer< %s >::Init(lua_State* L)\n' % fullname
        result += '{\n'
        result += '\tbool alreadyRegistered = luaL_newmetatable(L, "%s") == 0;\n' % metatablename
        result += '\tif(!alreadyRegistered)\n'
        result += '\t{\n'
        result += '\t\tlua_pushcfunction(L, ToString);\n'
        result += '\t\tlua_setfield(L, -2, "__tostring");\n'
        result += '\t\tlua_pushcfunction(L, ToNumber);\n'
        result += '\t\tlua_setfield(L, -2, "__tonumber");\n'
        result += '\t\tlua_pushcfunction(L, Eq);\n'
        result += '\t\tlua_setfield(L, -2, "__eq");\n'
        result += '\n'
        result += '\t\tlua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);\n'
        result += '\n'
        (packages, num_packages) = self.push_packages()
        if num_packages > 0:
            result += packages
            result += '\n'
        result += ''.join([self.addglobal(f) for f in self.values])
        result += '\n'
        result += '\t\tlua_pop(L, %s);\n' % (num_packages + 1)
        result += '\t}\n'
        result += '\n'
        result += '\tlua_pop(L, 1);\n'
        result += '}\n'
        result += '\n'
        result += 'const %s& ::usg::LuaSerializer< %s >::CheckEnum(lua_State* L, int arg)\n' % (fullname, fullname)
        result += '{\n'
        result += '\treturn *(%s*)luaL_checkudata(L, arg, "%s");\n' % (fullname, metatablename)
        result += '}\n'
        result += '\n'
        result += 'int ::usg::LuaSerializer< %s >::ToString(lua_State* L)\n' % fullname
        result += '{\n'
        result += '\tconst %s value = CheckEnum(L, -1);\n' % fullname
        result += '\tswitch(value)\n'
        result += '\t{\n'
        result += ''.join([self.tostring_case(f) for f in self.aliased_values()])
        result += '\t\tdefault: lua_pushfstring(L, "%s: INVALID VALUE %%d", value);\n' % luaname
        result += '\t}\n'
        result += '\n'
        result += '\treturn 1;\n'
        result += '}\n'
        result += '\n'
        result += 'int ::usg::LuaSerializer< %s >::ToNumber(lua_State* L)\n' % fullname
        result += '{\n'
        result += '\tconst %s value = CheckEnum(L, -1);\n' % fullname
        result += '\tlua_pushinteger(L, value);\n'
        result += '\n'
        result += '\treturn 1;\n'
        result += '}\n'
        result += '\n'
        result += 'int ::usg::LuaSerializer< %s >::Eq(lua_State* L)\n' % fullname
        result += '{\n'
        result += '\tconst %s a = CheckEnum(L, -1);\n' % fullname
        result += '\tconst %s b = CheckEnum(L, -2);\n' % fullname
        result += '\tlua_pushboolean(L, a == b);\n'
        result += '\n'
        result += '\treturn 1;\n'
        result += '}\n'
        result += '\n'
        return result

    def aliased_values(self):
        value_map = {}
        for item in self.values:
            if item[1] in value_map:
                value_map[item[1]].append(item[0])
            else:
                value_map[item[1]] = [item[0]]
        output = []
        for n, v in value_map.iteritems():
            output.append((v, n))
        return output

    def tostring_case(self, (names, number)):
        firstname = ns_prefix(names[0]) + str(names[0])
        if not firstname.startswith('::'):
            firstname = '::' + firstname

        prefixed_names = []
        for name in names:
            prefix = ns_prefix(name, '.')
            if prefix.startswith('.'):
                prefix = prefix[1:]
            prefixed_names.append(prefix + str(name))
        result = '\t\tcase %s: lua_pushstring(L, "%s"); break;\n' % (firstname, '/'.join(prefixed_names))
        return result

    def push_packages(self):
        lua_identifier = ns_prefix(self.names, '.')
        if lua_identifier.startswith('.'):
            lua_identifier = lua_identifier[1:]

        result = '\t\tint result;\n'
        packages = lua_identifier.split('.')[:-1]
        for package in packages:
            result += '\t\tresult = lua_getfield(L, -1, "%s");\n' % package
            result += '\t\tif(result == LUA_TNIL) { lua_pop(L, 1); lua_newtable(L); lua_pushvalue(L, -1); lua_setfield(L, -3, "%s"); }\n' % package

        return (result, len(packages))

    def addglobal(self, (name, number)):
        fullname = ns_prefix(name) + str(name)
        if not fullname.startswith('::'):
            fullname = '::' + fullname
        value = str(name)
        return '\t\tif( !lua_hasfield(L, -1, "%s") ) { PushToLua(L, %s); lua_setfield(L, -2, "%s"); }\n' % (value, fullname, value)

class Field:
    def __init__(self, local_namespaces, struct_name, desc, field_options, enums):
        '''desc is FieldDescriptorProto'''
        self.tag = desc.number
        self.struct_name = struct_name
        self.union_name = None
        self.name = desc.name
        self.default = None
        self.max_size = None
        self.max_count = None
        self.has_max_count = False
        self.array_decl = ""
        self.enc_size = None
        self.ctype = None
        self.decoder = None
        self.has_max_size = False

        breakdown = desc.type_name.split('.')
        tn = '.' + breakdown[-1]
        local_namespaces = breakdown[:-1]
        
        # Parse field options
        if field_options.HasField("max_size"):
            self.max_size = field_options.max_size
        
        if field_options.HasField("max_count"):
            self.max_count = field_options.max_count
        elif field_options.HasField("max_count_enum"):
            for enum in enums:
                if str(enum.names) == field_options.max_count_enum:
                    self.max_count = enum.count
                    break

            if self.max_count == None:
                raise Exception("Couldn't get count for " + field_options.max_count_enum)

        if field_options.HasField("has_max_count"):
            if self.max_count == None:
                raise Exception("has_max_count defined without max_count")
            self.has_max_count = field_options.has_max_count

        if field_options.HasField("decoder"):
            self.decoder = field_options.decoder

        if field_options.HasField("has_max_size"):
            if not field_options.HasField("max_size"):
                raise Exception("has_max_size defined without max_size")
            self.has_max_size = field_options.has_max_size

        if desc.HasField('default_value'):
            self.default = desc.default_value
           
        # Check field rules, i.e. required/optional/repeated.
        can_be_static = True
        if desc.label == FieldD.LABEL_REQUIRED:
            self.rules = 'REQUIRED'
        elif desc.label == FieldD.LABEL_OPTIONAL:
            self.rules = 'OPTIONAL'
        elif desc.label == FieldD.LABEL_REPEATED:
            self.rules = 'REPEATED'
            if self.max_count is None:
                can_be_static = False
            else:
                self.array_decl = '[%s_max_count]' % self.name
        else:
            raise NotImplementedError(desc.label)
        
        # Check if the field can be implemented with static allocation
        # i.e. whether the data size is known.
        if desc.type == FieldD.TYPE_STRING and self.max_size is None:
            can_be_static = False
        
        if desc.type == FieldD.TYPE_BYTES and self.max_size is None:
            can_be_static = False
        
        # Decide how the field data will be allocated
        if field_options.type == nanopb_pb2.FT_DEFAULT:
            if can_be_static:
                field_options.type = nanopb_pb2.FT_STATIC
            else:
                field_options.type = nanopb_pb2.FT_CALLBACK
        
        if field_options.type == nanopb_pb2.FT_STATIC and not can_be_static:
            raise Exception("Field %s is defined as static, but max_size or "
                            "max_count is not given." % self.name)
        
        if field_options.type == nanopb_pb2.FT_STATIC:
            if self.has_max_count:
                self.allocation = 'STATIC_MAX'
            elif self.has_max_size:
                self.allocation = 'STATIC_FIXED'
            else:
                self.allocation = 'STATIC'
        elif field_options.type == nanopb_pb2.FT_POINTER:
            self.allocation = 'POINTER'
        elif field_options.type == nanopb_pb2.FT_CALLBACK:
            self.allocation = 'CALLBACK'
        else:
            raise NotImplementedError(field_options.type)
        
        # Decide the C data type to use in the struct.
        if datatypes.has_key(desc.type):
            self.ctype, self.pbtype, self.enc_size, isa = datatypes[desc.type]

            # Override the field size if user wants to use smaller integers
            if isa and field_options.int_size != nanopb_pb2.IS_DEFAULT:
                self.ctype = intsizes[field_options.int_size]
                if desc.type == FieldD.TYPE_UINT32 or desc.type == FieldD.TYPE_UINT64:
                    self.ctype = 'u' + self.ctype;
        elif desc.type == FieldD.TYPE_ENUM:
            self.pbtype = 'ENUM'
            self.ctype = names_from_type_name(tn)
            self.ctype.namespaces = local_namespaces
            if self.default is not None:
                self.default = self.ctype + self.default
            self.enc_size = 5 # protoc rejects enum values > 32 bits
        elif desc.type == FieldD.TYPE_STRING:
            self.pbtype = 'STRING'
            self.ctype = 'char'
            if self.is_static_allocation():
                self.ctype = 'char'
                self.array_decl += '[%d]' % self.max_size
                self.enc_size = varint_max_size(self.max_size) + self.max_size
        elif desc.type == FieldD.TYPE_BYTES:
            self.pbtype = 'BYTES'

            if self.allocation == 'STATIC':
                self.ctype = self.struct_name + self.name + 't'
                self.enc_size = varint_max_size(self.max_size) + self.max_size
            elif self.allocation == 'STATIC_FIXED':
                self.ctype = 'uint8_t'
                self.array_decl += '[%d]' % self.max_size
                self.enc_size = varint_max_size(self.max_size) + self.max_size
            elif self.allocation == 'POINTER':
                self.ctype = 'pb_bytes_array_t' if not self.has_max_size else 'uint8_t'
        elif desc.type == FieldD.TYPE_MESSAGE:
            self.pbtype = 'MESSAGE'
            self.ctype = self.submsgname = names_from_type_name(tn)
            self.ctype.namespaces = self.submsgname.namespaces = local_namespaces
            self.enc_size = None # Needs to be filled in after the message type is available
        else:
            raise NotImplementedError(desc.type)
        
    def __cmp__(self, other):
        return cmp(self.tag, other.tag)
    
    def __str__(self):
        result = ''
        if self.max_count != None:
            result += '    enum { %s_max_count = %d };\n' % (self.name, self.max_count)

        if self.allocation == 'POINTER':
            if self.rules == 'REPEATED':
                result += '    pb_size_t ' + self.name + '_count;\n'
            
            if self.pbtype == 'MESSAGE':
                # Use struct definition, so recursive submessages are possible
                result += '    struct _%s *%s;' % (self.ctype, self.name)
            elif self.rules == 'REPEATED' and self.pbtype in ['STRING', 'BYTES']:
                # String/bytes arrays need to be defined as pointers to pointers
                result += '    %s%s **%s;' % (ns_prefix(self.ctype), self.ctype, self.name)
            else:
                result += '    %s%s *%s;' % (ns_prefix(self.ctype), self.ctype, self.name)
        elif self.allocation == 'CALLBACK':
            if self.decoder != None:
                if self.decoder == 'PBU8String':
                    result += '    ::usg::PBU8String<128> %s;' % (self.name)
                else:
                    prefixed_type = '%s%s' % (ns_prefix(self.ctype), self.ctype)
                    m = re.search('([^<]*)(<(.*)>)?[^>]*$', self.decoder)
                    decoder_delegate = m.group(1)
                    delegate_params = ', ' + m.group(3) if m.group(3) else ''
                    result += '    ::usg::ProtocolBufferVarLength< %s, ::usg::%s< %s%s > > %s;' % \
                        (prefixed_type, decoder_delegate, prefixed_type, delegate_params, self.name)
            else:
                result += '    pb_callback_t %s;' % self.name
        else:
            if self.rules == 'OPTIONAL' and self.is_static_allocation():
                result += '    bool has_' + self.name + ';\n'
            elif self.rules == 'REPEATED' and self.is_static_allocation() and not self.has_max_count:
                result += '    pb_size_t ' + self.name + '_count;\n'
            result += '    %s%s %s%s;' % (ns_prefix(self.ctype), self.ctype, self.name, self.array_decl)
        return result
    
    def register_get_callback(self):
        result = '\t\t\tlua_pushcfunction(L, push_%s);\n' % self.name
        result += '\t\t\tlua_setfield(L, -2, "%s");\n' % self.name
        return result

    def types(self):
        '''Return definitions for any special types this field might need.'''
        result = ''

        if self.pbtype == 'BYTES':
            if self.allocation == 'STATIC':
                result = 'typedef PB_BYTES_ARRAY_T(%d) %s;\n' % (self.max_size, self.ctype)

        return result
    
    def get_dependencies(self):
        '''Get list of type names used by this field.'''
        return [str(self.ctype)]

    def get_initializer(self, null_init, inner_init_only = False):
        '''Return literal expression for this field's default value.
        null_init: If True, initialize to a 0 value instead of default from .proto
        inner_init_only: If True, exclude initialization for any count/has fields
        '''

        inner_init = None
        if self.pbtype == 'MESSAGE':
            if null_init:
                inner_init = '%s_init_zero' % self.ctype
            else:
                inner_init = '%s_init_default' % self.ctype
        elif self.default is None or null_init:
            if self.pbtype == 'STRING':
                inner_init = '""'
            elif self.pbtype == 'BYTES':
                inner_init = '{0, {0}}'
            elif self.pbtype == 'ENUM':
                inner_init = '(%s)0' % self.ctype
            else:
                inner_init = '0'
        else:
            if self.pbtype == 'STRING':
                inner_init = self.default.encode('utf-8').encode('string_escape')
                inner_init = inner_init.replace('"', '\\"')
                inner_init = '"' + inner_init + '"'
            elif self.pbtype == 'BYTES':
                data = str(self.default).decode('string_escape')
                data = ['0x%02x' % ord(c) for c in data]
                if len(data) == 0:
                    inner_init = '{0, {0}}'
                else:
                    inner_init = '{%d, {%s}}' % (len(data), ','.join(data))
            elif self.pbtype in ['FIXED32', 'UINT32']:
                inner_init = str(self.default) + 'u'
            elif self.pbtype in ['FIXED64', 'UINT64']:
                inner_init = str(self.default) + 'ull'
            elif self.pbtype in ['SFIXED64', 'INT64']:
                inner_init = str(self.default) + 'll'
            elif self.pbtype in ['FLOAT']:
                inner_init = str(self.default)
                if '.' not in inner_init:
                    inner_init += '.'
                inner_init += 'f'
            else:
                inner_init = str(self.default)
        
        if inner_init_only:
            return inner_init

        outer_init = None
        if self.is_static_allocation():
            if self.rules == 'REPEATED':
                outer_init = '0, {'
                outer_init += ', '.join([inner_init] * self.max_count)
                outer_init += '}'
            elif self.rules == 'OPTIONAL':
                outer_init = 'false, ' + inner_init
            else:
                outer_init = inner_init
        elif self.allocation == 'POINTER':
            if self.rules == 'REPEATED':
                outer_init = '0, NULL'
            else:
                outer_init = 'NULL'
        elif self.allocation == 'CALLBACK':
            if self.pbtype == 'EXTENSION':
                outer_init = 'NULL'
            else:
                outer_init = '{{NULL}, NULL}'

        return outer_init

    def default_decl(self, declaration_only = False):
        '''Return definition for this field's default value.'''
        if self.default is None:
            return None

        ctype = self.ctype
        default = self.get_initializer(False, True)
        array_decl = ''
        
        if self.pbtype == 'STRING':
            if not self.is_static_allocation():
                return None # Not implemented
            array_decl = '[%d]' % self.max_size
        elif self.pbtype == 'BYTES':
            if not self.is_static_allocation():
                return None # Not implemented
        
        if declaration_only:
            return 'extern const %s %s_default%s;' % (ctype, self.struct_name + self.name, array_decl)
        else:
            return 'const %s %s_default%s = %s;' % (ctype, self.struct_name + self.name, array_decl, default)

    def is_static_allocation(self):
        return self.allocation == 'STATIC' \
            or self.allocation == 'STATIC_MAX' or self.allocation == 'STATIC_FIXED'

    def tags(self):
        '''Return the #define for the tag number of this field.'''
        identifier = '%s_%s_tag' % (self.struct_name, self.name)
        return '#define %-40s %d\n' % (identifier, self.tag)
    
    def pb_field_t(self, prev_field_name):
        '''Return the pb_field_t initializer to use in the constant array.
        prev_field_name is the name of the previous field or None.
        '''

        if self.rules == 'ONEOF':
            result = '    PB_ONEOF_FIELD(%s, ' % self.union_name
        else:
            result = '    PB_FIELD('

        result += '%3d, ' % self.tag
        result += '%-8s, ' % self.pbtype
        result += '%s, ' % self.rules
        result += '%-12s, ' % self.allocation
        result += '%s, ' % ("FIRST" if not prev_field_name else "OTHER")
        result += '%s, ' % self.struct_name
        result += '%s, ' % self.name
        result += '%s, ' % (prev_field_name or self.name)
        
        if self.pbtype == 'MESSAGE':
            result += '&%s%s_fields)' % (ns_prefix(self.submsgname), self.submsgname)
        elif self.default is None:
            result += '0)'
        elif self.pbtype in ['BYTES', 'STRING'] and not self.is_static_allocation():
            result += '0)' # Arbitrary size default values not implemented
        elif self.rules == 'OPTEXT':
            result += '0)' # Default value for extensions is not implemented
        else:
            result += '&%s_default)' % (self.struct_name + self.name)
        
        return result
    
    def largest_field_value(self):
        '''Determine if this field needs 16bit or 32bit pb_field_t structure to compile properly.
        Returns numeric value or a C-expression for assert.'''
        if self.pbtype == 'MESSAGE':
            if self.rules == 'REPEATED' and self.is_static_allocation():
                return 'pb_membersize(%s, %s[0])' % (self.struct_name, self.name)
            elif self.rules == 'ONEOF':
                return 'pb_membersize(%s, %s.%s)' % (self.struct_name, self.union_name, self.name)
            else:
                return 'pb_membersize(%s, %s)' % (self.struct_name, self.name)

        return max(self.tag, self.max_size, self.max_count)        

    def encoded_size(self, allmsgs):
        '''Return the maximum size that this field can take when encoded,
        including the field tag. If the size cannot be determined, returns
        None.'''
        
        if not self.is_static_allocation():
            return None
        
        if self.pbtype == 'MESSAGE':
            for msg in allmsgs:
                if msg.name == self.submsgname:
                    encsize = msg.encoded_size(allmsgs)
                    if encsize is None:
                        return None # Submessage size is indeterminate
                        
                    # Include submessage length prefix
                    encsize += varint_max_size(encsize.upperlimit())
                    break
            else:
                # Submessage cannot be found, this currently occurs when
                # the submessage type is defined in a different file.
                # Instead of direct numeric value, reference the size that
                # has been #defined in the other file.
                encsize = EncodedSize(self.submsgname + 'size')

                # We will have to make a conservative assumption on the length
                # prefix size, though.
                encsize += 5

        elif self.enc_size is None:
            raise RuntimeError("Could not determine encoded size for %s.%s"
                               % (self.struct_name, self.name))
        else:
            encsize = EncodedSize(self.enc_size)
        
        encsize += varint_max_size(self.tag << 3) # Tag + wire type

        if self.rules == 'REPEATED':
            # Decoders must be always able to handle unpacked arrays.
            # Therefore we have to reserve space for it, even though
            # we emit packed arrays ourselves.
            encsize *= self.max_count
        
        return encsize


class ExtensionRange(Field):
    def __init__(self, struct_name, range_start, field_options):
        '''Implements a special pb_extension_t* field in an extensible message
        structure. The range_start signifies the index at which the extensions
        start. Not necessarily all tags above this are extensions, it is merely
        a speed optimization.
        '''
        self.tag = range_start
        self.struct_name = struct_name
        self.name = 'extensions'
        self.pbtype = 'EXTENSION'
        self.rules = 'OPTIONAL'
        self.allocation = 'CALLBACK'
        self.ctype = 'pb_extension_t'
        self.array_decl = ''
        self.default = None
        self.max_size = 0
        self.max_count = 0
        self.has_max_count = False
        self.decoder = None
        self.has_max_size = False
        
    def __str__(self):
        return '    pb_extension_t *extensions;'
    
    def types(self):
        return ''
    
    def tags(self):
        return ''
    
    def encoded_size(self, allmsgs):
        # We exclude extensions from the count, because they cannot be known
        # until runtime. Other option would be to return None here, but this
        # way the value remains useful if extensions are not used.
        return EncodedSize(0)

class ExtensionField(Field):
    def __init__(self, struct_name, desc, field_options):
        self.fullname = struct_name + desc.name
        if not self.fullname.startswith('::'):
            self.fullname = '::' + self.fullname

        self.extendee_name = names_from_type_name(desc.extendee)
        Field.__init__(self, self.fullname + 'struct', desc, field_options)
        
        if self.rules != 'OPTIONAL':
            self.skip = True
        else:
            self.skip = False
            self.rules = 'OPTEXT'

    def tags(self):
        '''Return the #define for the tag number of this field.'''
        identifier = '%s_tag' % self.fullname
        return '#define %-40s %d\n' % (identifier, self.tag)

    def extension_decl(self):
        '''Declaration of the extension type in the .pb.h file'''
        if self.skip:
            msg = '/* Extension field %s was skipped because only "optional"\n' % self.fullname
            msg +='   type of extension fields is currently supported. */\n'
            return msg
        
        return ('extern const pb_extension_type_t %s; /* field type: %s */\n' %
            (self.fullname, str(self).strip()))

    def extension_def(self):
        '''Definition of the extension type in the .pb.c file'''

        if self.skip:
            return ''

        result  = 'typedef struct {\n'
        result += str(self)
        result += '\n} %s;\n\n' % self.struct_name
        result += ('static const pb_field_t %s_field = \n  %s;\n\n' %
                    (self.fullname, self.pb_field_t(None)))
        result += 'const pb_extension_type_t %s = {\n' % self.fullname
        result += '    NULL,\n'
        result += '    NULL,\n'
        result += '    &%s_field\n' % self.fullname
        result += '};\n'
        return result


# ---------------------------------------------------------------------------
#                   Generation of oneofs (unions)
# ---------------------------------------------------------------------------

class OneOf(Field):
    def __init__(self, struct_name, oneof_desc):
        self.struct_name = struct_name
        self.name = oneof_desc.name
        self.ctype = 'union'
        self.fields = []

    def add_field(self, field):
        if field.allocation == 'CALLBACK':
            raise Exception("Callback fields inside of oneof are not supported"
                            + " (field %s)" % field.name)

        field.union_name = self.name
        field.rules = 'ONEOF'
        self.fields.append(field)
        self.fields.sort(key = lambda f: f.tag)

        # Sort by the lowest tag number inside union
        self.tag = min([f.tag for f in self.fields])

    def __cmp__(self, other):
        return cmp(self.tag, other.tag)

    def __str__(self):
        result = ''
        if self.fields:
            result += '    pb_size_t which_' + self.name + ";\n"
            result += '    union {\n'
            for f in self.fields:
                result += '    ' + str(f).replace('\n', '\n    ') + '\n'
            result += '    } ' + self.name + ';'
        return result

    def types(self):
        return ''.join([f.types() for f in self.fields])

    def get_dependencies(self):
        deps = []
        for f in self.fields:
            deps += f.get_dependencies()
        return deps

    def get_initializer(self, null_init):
        return '0, {' + self.fields[0].get_initializer(null_init) + '}'

    def default_decl(self, declaration_only = False):
        return None

    def tags(self):
        return '\n'.join([f.tags() for f in self.fields])

    def pb_field_t(self, prev_field_name):
        prev_field_name = prev_field_name or self.name
        result = ',\n'.join([f.pb_field_t(prev_field_name) for f in self.fields])
        return result

    def largest_field_value(self):
        return max([f.largest_field_value() for f in self.fields])

    def encoded_size(self, allmsgs):
        largest = EncodedSize(0)
        for f in self.fields:
            size = f.encoded_size(allmsgs)
            if size is None:
                return None
            elif size.symbols:
                return None # Cannot resolve maximum of symbols
            elif size.value > largest.value:
                largest = size

        return largest

# ---------------------------------------------------------------------------
#                   Generation of messages (structures)
# ---------------------------------------------------------------------------


class Message:
    def __init__(self, names, desc, message_options, enums):
        self.name = names
        self.fields = []
        self.oneofs = {}
        no_unions = []

        if hasattr(desc, 'oneof_decl'):
            for i, f in enumerate(desc.oneof_decl):
                oneof_options = get_nanopb_suboptions(desc, message_options, self.name + f.name)
                if oneof_options.no_unions:
                    no_unions.append(i) # No union, but add fields normally
                elif oneof_options.type == nanopb_pb2.FT_IGNORE:
                    pass # No union and skip fields also
                else:
                    oneof = OneOf(self.name, f)
                    self.oneofs[i] = oneof
                    self.fields.append(oneof)

        self.from_header = None
        if message_options.HasField("from_header"):
            self.from_header = message_options.from_header

        self.lua_receive = False
        if message_options.HasField("lua_receive"):
            self.lua_receive = message_options.lua_receive

        self.lua_send = False
        if message_options.HasField("lua_send"):
            self.lua_send = message_options.lua_send

        self.lua_generate = False
        if self.lua_send or self.lua_receive or message_options.HasField("lua_generate"):
            self.lua_generate = True

        for f in desc.field:
            field_options = get_nanopb_suboptions(f, message_options, self.name + f.name)
            if field_options.type == nanopb_pb2.FT_IGNORE:
                continue

            field = Field(names.namespaces, self.name, f, field_options, enums)
            if (hasattr(f, 'oneof_index') and
                f.HasField('oneof_index') and
                f.oneof_index not in no_unions):
                if f.oneof_index in self.oneofs:
                    self.oneofs[f.oneof_index].add_field(field)
            else:
                self.fields.append(field)
        
        if len(desc.extension_range) > 0:
            field_options = get_nanopb_suboptions(desc, message_options, self.name + 'extensions')
            range_start = min([r.start for r in desc.extension_range])
            if field_options.type != nanopb_pb2.FT_IGNORE:
                self.fields.append(ExtensionRange(self.name, range_start, field_options))
        
        self.packed = message_options.packed_struct
        self.ordered_fields = self.fields[:]
        self.ordered_fields.sort()

    def get_dependencies(self):
        '''Get list of type names that this structure refers to.'''
        deps = []
        for f in self.fields:
            deps += f.get_dependencies()
        return deps
    
    def decl(self):
        fullname = ns_prefix(self.name) + str(self.name)
        if not fullname.startswith('::'):
            fullname = '::' + fullname

        result = 'template<>\n'
        result += 'struct LuaSerializer< %s >\n' % fullname
        result += '{\n'
        result += '\tstatic const bool RECEIVE = %s;\n' % ("true" if self.lua_receive else "false")
        result += '\tstatic const bool SEND    = %s;\n' % ("true" if self.lua_send else "false")
        result += '\tstatic const bool GENERATE = %s;\n' % ("true" if self.lua_generate else "false")
        result += '\tstatic int PushToLua(lua_State* L, const %s& data);\n' % fullname
        result += '\tstatic bool PushMetatable(lua_State* L);\n'
        result += '\tstatic int GetFromLua(lua_State* L, int index, %s* data);\n' % fullname
        result += '\tstatic void Init(lua_State* L);\n'
        result += '\tstatic %s& CheckMessage(lua_State* L, int arg);\n' % fullname
        result += '\tstatic int Index(lua_State* L);\n'
        result += '\tstatic int NewIndex(lua_State* L);\n'
        result += '\tstatic int Call(lua_State* L);\n'
        result += '};\n'

        return result

    def __str__(self):
        fullname = ns_prefix(self.name) + str(self.name)
        if not fullname.startswith('::'):
            fullname = '::' + fullname

        metatablename = ns_prefix(self.name, '.') + str(self.name)
        if metatablename.startswith('.'):
            metatablename = metatablename[1:]



        result = 'int ::usg::LuaSerializer< %s >::PushToLua(lua_State* L, const %s& data)\n' % (fullname, fullname)
        result += '{\n'
        result += '\tbool bIsRegistered = luaL_getmetatable(L, "%s") != LUA_TNIL;\n' % metatablename
        result += '\tlua_pop(L, 1);\n'
        result += '\tASSERT(bIsRegistered);\n'
        result += '\n'
        result += '\t%s* dataPtr = (%s*)lua_newuserdata(L, sizeof(%s));\n' % (fullname, fullname, fullname)
        result += '\tluaL_setmetatable(L, "%s");\n' % metatablename
        result += '\tmemcpy(dataPtr, &data, sizeof(%s));\n' % fullname
        result += '\treturn 1;\n'
        result += '}\n'
        result += '\n'

        if self.lua_send:
            result += 'template<>\n'
            result += 'void ::usg::RegisterLuaEventTransmitters< %s >(lua_State* L)\n' % (fullname)
            result += '{\n'
            result += '\tbool bIsRegistered = LuaSerializer< %s >::PushMetatable(L);\n' % (fullname)
            result += '\tASSERT(bIsRegistered);\n'
            result += '\tlua_pushcfunction(L, LuaEvents< %s >::Send);\n' % (fullname)
            result += '\tlua_setfield(L, -2, "Send");\n'
            result += '\tlua_pushcfunction(L, LuaEvents< %s >::SendToEntity);\n' % (fullname)
            result += '\tlua_setfield(L, -2, "SendToEntity");\n'
            result += '\tlua_pushcfunction(L, LuaEvents< %s >::SendOverNetwork);\n' % (fullname)
            result += '\tlua_setfield(L, -2, "SendOverNetwork");\n'
            result += '\tlua_pushcfunction(L, LuaEvents< %s >::SendToNetworkEntity);\n' % (fullname)
            result += '\tlua_setfield(L, -2, "SendToNetworkEntity");\n'
            result += '\tlua_pop(L, 1);\n'
            result += '}\n'
            result += '\n'

        result += 'bool ::usg::LuaSerializer< %s >::PushMetatable(lua_State* L)\n' % fullname
        result += '{\n'
        result += '\treturn luaL_getmetatable(L, "%s") != LUA_TNIL;\n' % metatablename
        result += '}\n'
        result += '\n'
        result += 'int ::usg::LuaSerializer< %s >::GetFromLua(lua_State* L, int index, %s* data)\n' % (fullname, fullname)
        result += '{\n'
        result += '\tconst %s& msg = CheckMessage(L, index);\n' % fullname
        result += '\tmemcpy(data, &msg, sizeof(%s));\n' % fullname
        result += '\treturn 0;\n'
        result += '}\n'
        result += '\n'
        result += 'void ::usg::LuaSerializer< %s >::Init(lua_State* L)\n' % fullname
        result += '{\n'
        result += '\tbool alreadyRegistered = luaL_newmetatable(L, "%s") == 0;\n' % metatablename
        result += '\tif(!alreadyRegistered)\n'
        result += '\t{\n'
        result += '\t\tlua_pushcfunction(L, Index);\n'
        result += '\t\tlua_setfield(L, -2, "__index");\n'
        result += '\t\tlua_pushcfunction(L, NewIndex);\n'
        result += '\t\tlua_setfield(L, -2, "__newindex");\n'
        result += '\t\tlua_pushcfunction(L, Call);\n'
        result += '\t\tlua_setfield(L, -2, "__call");\n'
        result += '\t\tlua_pushboolean(L, true);\n'
        result += '\t\tlua_setfield(L, -2, "__is_pb_message");\n'
        result += '\n'
        result += '\t\tlua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);\n'
        result += '\n'
        (packages, num_packages) = self.push_packages()
        if num_packages > 0:
            result += packages
            result += '\n'
        result += self.addconstructor(fullname, metatablename)
        result += '\n'
        result += '\t\tlua_pop(L, %s);\n' % (num_packages + 1)
        fields = filter(None, list(set([self.init_type(t) for t in (f.ctype for f in self.ordered_fields)])))
        if fields:
            result += '\n\t\t'
            result += '\n\t\t'.join(fields)
            result += '\n'
        result += '\t}\n'
        result += '\tlua_pop(L, 1);\n'
        result += '}\n'
        result += '\n'
        result += '%s& ::usg::LuaSerializer< %s >::CheckMessage(lua_State* L, int arg)\n' % (fullname, fullname)
        result += '{\n'
        result += '\treturn *(%s*)luaL_checkudata(L, arg, "%s");\n' % (fullname, metatablename)
        result += '}\n'
        result += '\n'
        result += 'int ::usg::LuaSerializer< %s >::Index(lua_State* L)\n' % fullname
        result += '{\n'
        result += '\tconst %s& value_in = CheckMessage(L, 1);\n' % fullname
        result += '\tsize_t keyLen = 0;\n'
        result += '\tconst char* key = lua_tolstring( L, 2, &keyLen );\n\n\t'
        if len(self.ordered_fields) > 0:
            result += '\n\telse '.join([self.index_field(f) for f in self.ordered_fields])
            result += '\n\telse'
            result += '\n'
            result += '\t{\n'
            result += '\t\tbool bIsRegistered = PushMetatable(L);\n'
            result += '\t\tASSERT(bIsRegistered);\n'
            result += '\n'
            result += '\t\tlua_pushvalue(L, 2);\n'
            result += '\t\tlua_gettable(L, -2);\n'
            result += '\n'
            result += '\t\tlua_remove(L, 3);\n'
            result += '\t}\n'
        else:
            result += 'bool bIsRegistered = PushMetatable(L);\n'
            result += '\tASSERT(bIsRegistered);\n'
            result += '\n'
            result += '\tlua_pushvalue(L, 2);\n'
            result += '\tlua_gettable(L, -2);\n'
            result += '\n'
            result += '\tlua_remove(L, 3);\n'
        result += '\n'
        result += '\treturn 1;\n'
        result += '}\n'
        result += '\n'
        result += 'int ::usg::LuaSerializer< %s >::NewIndex(lua_State* L)\n' % fullname
        result += '{\n'
        result += '\t%s& value = CheckMessage(L, 1);\n' % fullname
        result += '\tsize_t keyLen = 0;\n'
        result += '\tconst char* key = lua_tolstring( L, 2, &keyLen );\n\n\t'
        if len(self.ordered_fields) > 0:
            result += '\n\telse '.join([self.new_index_field(f) for f in self.ordered_fields])
            result += '\n\telse\n\t{\n\t\tASSERT(false && "Field not found!");\n\t}\n'
        else:
            result += '\tASSERT(false && "Attempting to index a record with no fields!");\n'
        result += '\n'
        result += '\treturn 0;\n'
        result += '}\n'
        result += '\n'
        result += 'int ::usg::LuaSerializer< %s >::Call(lua_State* L)\n' % fullname
        result += '{\n'
        result += '\tconst %s& value_in = CheckMessage(L, 1);\n' % fullname
        result += '\t%s* value_out = (%s*)lua_newuserdata(L, sizeof(%s));\n' % (fullname, fullname, fullname)
        result += '\tmemcpy(value_out, &value_in, sizeof(%s));\n' % fullname
        result += '\n'
        result += '\tint has_metatable = lua_getmetatable(L, 1);\n'
        result += '\tASSERT(has_metatable != 0);\n'
        result += '\tlua_setmetatable(L, 3);\n'
        result += '\n'
        result += ''.join([self.set_field(f) for f in self.ordered_fields])
        result += '\n'
        result += '\treturn 1;\n'
        result += '}\n'
        result += '\n'

        return result

    def init_type(self, t):
        typename = str(t)
        simple_types = ['bool', 'char', 'float', 'double',
                        'uint32_t', 'int32_t', 'uint64_t', 'int64_t']
        if typename not in simple_types:
            typename = ns_prefix(t) + typename
            if not typename.startswith('::'):
                typename = '::' + typename
            return "::usg::InitLuaType< %s >(L);" % typename

    def index_field(self, field):
        result  = 'if(keyLen == sizeof("%s")-1 && !memcmp(key, "%s", sizeof("%s")-1))\n' % (field.name, field.name, field.name)
        result += '\t{\n'
        if field.rules == 'OPTIONAL':
            result += '\t\tif(value_in.has_%s)\n' % field.name
            result += '\t\t{\n'
            result += '\t\t\t::usg::PushToLua(L, value_in.%s);\n' % field.name
            result += '\t\t}\n'
            result += '\t\telse\n'
            result += '\t\t{\n'
            result += '\t\t\tlua_pushnil(L);\n'
            result += '\t\t}\n'
        else:
            if field.rules == 'REPEATED' and field.max_count != None:
                if field.has_max_count:
                    result += '\t\tlua_pushnil(L);\n'
                else:
                    result += '\t\tlua_pushinteger(L, value_in.%s_count);\n' % field.name
            result += '\t\t::usg::PushToLua(L, value_in.%s);\n' % field.name
        result += '\t}'
        return result

    def new_index_field(self, field):
        result  = 'if(keyLen == sizeof("%s")-1 && !memcmp(key, "%s", sizeof("%s")-1))\n' % (field.name, field.name, field.name)
        result += '\t{\n'
        if field.rules == 'OPTIONAL':
            result += '\t\tif(!lua_isnil(L, -1))\n'
            result += '\t\t{\n'
            result += '\t\t\tvalue.has_%s = true;\n' % field.name
            result += '\t\t\t::usg::GetFromLua(L, -1, &value.%s);\n' % field.name
            result += '\t\t}\n'
            result += '\t\telse\n'
            result += '\t\t{\n'
            result += '\t\t\tvalue.has_%s = false;\n' % field.name
            result += '\t\t}\n'
        else:
            result += '\t\t'
            if field.rules == 'REPEATED' and field.max_count != None and not field.has_max_count:
                result += 'value.%s_count = ' % field.name
            result += '::usg::GetFromLua(L, 3, &value.%s);\n' % field.name
        result += '\t}'
        return result

    def set_field(self, field):
        result  = '\tif( lua_getfield(L, 2, "%s") != LUA_TNIL )\n' % field.name
        result += '\t{\n\t\t'
        if field.rules == 'OPTIONAL':
            result += 'value_out->has_%s = true;\n\t\t' % field.name
        elif field.rules == 'REPEATED' and field.max_count != None and not field.has_max_count:
            result += 'value_out->%s_count = ' % field.name
        result += '::usg::GetFromLua(L, -1, &value_out->%s);\n' % field.name
        result += '\t}\n'
        result += '\tlua_pop(L, 1);\n'
        return result

    def addconstructor(self, fullname, metatablename):
        result  = '\t\tif( !lua_hasfield(L, -1, "%s") )\n' % self.name
        result += '\t\t{\n'
        result += '\t\t\t%s* dataPtr = (%s*)lua_newuserdata(L, sizeof(%s));\n' % (fullname, fullname, fullname)
        result += '\t\t\tluaL_setmetatable(L, "%s");\n' % metatablename
        result += '\t\t\tProtocolBufferFields< %s >::Init(dataPtr);\n' % fullname
        result += '\t\t\tlua_setfield(L, -2, "%s");\n' % self.name
        result += '\t\t}\n' % self.name

        return result

    # TODO: Reduce code duplication with enums
    def push_packages(self):
        lua_identifier = ns_prefix(self.name, '.')
        if lua_identifier.startswith('.'):
            lua_identifier = lua_identifier[1:]

        result = '\t\tint result;\n'
        packages = lua_identifier.split('.')[:-1]
        for package in packages:
            result += '\t\tresult = lua_getfield(L, -1, "%s");\n' % package
            result += '\t\tif(result == LUA_TNIL) { lua_pop(L, 1); lua_newtable(L); lua_pushvalue(L, -1); lua_setfield(L, -3, "%s"); }\n' % package

        return (result, len(packages))

    def types(self):
        return ''.join([f.types() for f in self.fields])

    def get_initializer(self, null_init):
        if not self.ordered_fields:
            return '{0}'
    
        parts = []
        for field in self.ordered_fields:
            parts.append(field.get_initializer(null_init))
        return '{' + ', '.join(parts) + '}'
    
    def default_decl(self, declaration_only = False):
        result = ""
        for field in self.fields:
            default = field.default_decl(declaration_only)
            if default is not None:
                result += default + '\n'
        return result

    def count_required_fields(self):
        '''Returns number of required fields inside this message'''
        count = 0
        for f in self.fields:
            if not isinstance(f, OneOf):
                if f.rules == 'REQUIRED':
                    count += 1
        return count

    def count_all_fields(self):
        count = 0
        for f in self.fields:
            if isinstance(f, OneOf):
                count += len(f.fields)
            else:
                count += 1
        return count

    def fields_declaration(self):
        result = 'extern const pb_field_t %s_fields[%d];' % (self.name, self.count_all_fields() + 1)
        return result

    def fields_template_specialization_propagate(self, func):
        for field in self.fields:
            if field.allocation == 'CALLBACK' and field.decoder != None:
                yield '\t\tmsg->%s.%s();' % (field.name, func)
            elif field.pbtype == 'MESSAGE':
                if field.rules == 'REPEATED' and field.is_static_allocation():
                    yield '\t\tfor(size_t i = 0; i < %d; i++)' % (field.max_count)
                    yield '\t\t\t%sPB(&msg->%s[i]);' % (func, field.name)
                else:
                    yield '\t\t%sPB(&msg->%s);' % (func, field.name)

    def fields_template_specialization(self, namespaces):
        ns = '::'.join(namespaces) + ('::' if namespaces else '')
        crc =  zlib.crc32(str(self.name))
        crc += 2**32 if crc < 0 else 0
        result = 'template <>\nstruct ProtocolBufferFields< ::%s%s > {\n' % (ns, self.name)
        result += '\tenum { ID = %dU };\n' % (crc)
        result += '\tstatic const pb_field_t * Spec() { return ::%s%s_fields; };\n' % (ns, self.name)
        result += '\tstatic void PreLoad(::%s%s *msg) {\n' % (ns, self.name)
        result += '\n'.join(self.fields_template_specialization_propagate("PreLoad"))
        result += '\n\t}\n'
        result += '\tstatic void PostLoad(::%s%s *msg) {\n' % (ns, self.name)
        result += '\n'.join(self.fields_template_specialization_propagate("PostLoad"))
        result += '\n\t}\n'
        result += '\tstatic void PreSave(::%s%s *msg) {\n' % (ns, self.name)
        result += '\n'.join(self.fields_template_specialization_propagate("PreSave"))
        result += '\n\t}\n'
        result += '\tstatic void PostSave(::%s%s *msg) {\n' % (ns, self.name)
        result += '\n'.join(self.fields_template_specialization_propagate("PostSave"))
        result += '\n\t}\n'
        result += '};'
        return result

    def fields_definition(self):
        result = 'const pb_field_t %s_fields[%d] = {\n' % (self.name, self.count_all_fields() + 1)
        
        prev = None
        for field in self.ordered_fields:
            result += field.pb_field_t(prev)
            result += ',\n'
            if isinstance(field, OneOf):
                prev = field.name + '.' + field.fields[-1].name
            else:
                prev = field.name
        
        result += '    PB_LAST_FIELD\n};'
        return result

    def init_function_declaration(self):
        result = 'void %s_init(%s*);' % (self.name, self.name)
        return result

    def init_function_definition(self):
        result = 'void %s_init(%s *self)\n' % (self.name, self.name)
        result += '{\n'
        for field in self.ordered_fields:
            array_suffix = ""
            is_c_string = field.pbtype == 'STRING' and field.decoder == None
            has_default = field.default != None or field.pbtype == 'ENUM'
            skip_init = field.pbtype != 'MESSAGE' and not is_c_string and not has_default
            if field.rules == 'REPEATED':
                if field.has_max_count:
                    if not skip_init:
                        result += '    for(int i = 0; i < %s::%s_max_count; ++i)\n    ' % (self.name, field.name)
                        array_suffix = "[i]"
                else:
                    if field.allocation != 'CALLBACK':
                        result += '    self->%s_count = 0;\n' % (field.name)
                    skip_init = True
            elif field.rules == 'OPTIONAL':
                result += '    self->has_%s = false;\n' % ( field.name )

            if not skip_init:
                if field.pbtype == 'MESSAGE':
                    result += '    %s_init(&self->%s%s);\n' % (field.submsgname, field.name, array_suffix)
                elif is_c_string:
                    result += '    self->%s%s[0] = \'\\0\';\n' % (field.name, array_suffix)
                elif field.pbtype == 'ENUM':
                    result += '    self->%s%s = (%s%s)0;\n' % (field.name, array_suffix, ns_prefix(field.ctype), field.ctype)
                else:
                    result += '    self->%s%s = %s_default;\n' % (field.name, array_suffix, field.struct_name + field.name)
        result += '}\n'
        return result

    def encoded_size(self, allmsgs):
        '''Return the maximum size that this message can take when encoded.
        If the size cannot be determined, returns None.
        '''
        size = EncodedSize(0)
        for field in self.fields:
            fsize = field.encoded_size(allmsgs)
            if fsize is None:
                return None
            size += fsize
        
        return size


# ---------------------------------------------------------------------------
#                    Processing of entire .proto files
# ---------------------------------------------------------------------------


def iterate_messages(desc, names = Names()):
    '''Recursively find all messages. For each, yield name, DescriptorProto.'''
    if hasattr(desc, 'message_type'):
        submsgs = desc.message_type
    else:
        submsgs = desc.nested_type
    
    for submsg in submsgs:
        sub_names = names + submsg.name
        yield sub_names, submsg
        
        for x in iterate_messages(submsg, sub_names):
            yield x

def iterate_extensions(desc, names = Names()):
    '''Recursively find all extensions.
    For each, yield name, FieldDescriptorProto.
    '''
    for extension in desc.extension:
        yield names, extension

    for subname, subdesc in iterate_messages(desc, names):
        for extension in subdesc.extension:
            yield subname, extension

def parse_file(fdesc, file_options):
    '''Takes a FileDescriptorProto and returns tuple (enums, messages, extensions).'''
    
    enums = []
    messages = []
    extensions = []

    base_name = Names((), fdesc.package.split('.'))
    
    for enum in fdesc.enum_type:
        enum_options = get_nanopb_suboptions(enum, file_options, base_name + enum.name)
        generate = enum_options.HasField("lua_generate")
        if not generate:
            continue
        enums.append(Enum(base_name, enum, enum_options))
    
    for names, message in iterate_messages(fdesc, base_name):
        message_options = get_nanopb_suboptions(message, file_options, names)

        generate = message_options.HasField("lua_receive") or message_options.HasField("lua_send") or message_options.HasField("lua_generate")
        if message_options.skip_message or not generate:
            continue
        
        for enum in message.enum_type:
            enum_options = get_nanopb_suboptions(enum, message_options, names + enum.name)
            enums.append(Enum(names, enum, enum_options))

        messages.append(Message(names, message, message_options, enums))
    
    for names, extension in iterate_extensions(fdesc, base_name):
        field_options = get_nanopb_suboptions(extension, file_options, names + extension.name)
        if field_options.type != nanopb_pb2.FT_IGNORE:
            extensions.append(ExtensionField(names, extension, field_options))
    
    # Fix field default values where enum short names are used.
    for enum in enums:
        if not enum.options.long_names:
            for message in messages:
                for field in message.fields:
                    if field.default in enum.value_longnames:
                        idx = enum.value_longnames.index(field.default)
                        field.default = enum.values[idx][0]
    
    return enums, messages, extensions

def toposort2(data):
    '''Topological sort.
    From http://code.activestate.com/recipes/577413-topological-sort/
    This function is under the MIT license.
    '''
    for k, v in data.items():
        v.discard(k) # Ignore self dependencies
    extra_items_in_deps = reduce(set.union, data.values(), set()) - set(data.keys())
    data.update(dict([(item, set()) for item in extra_items_in_deps]))
    while True:
        ordered = set(item for item,dep in data.items() if not dep)
        if not ordered:
            break
        for item in sorted(ordered):
            yield item
        data = dict([(item, (dep - ordered)) for item,dep in data.items()
                if item not in ordered])
    assert not data, "A cyclic dependency exists amongst %r" % data

def sort_dependencies(messages):
    '''Sort a list of Messages based on dependencies.'''
    dependencies = {}
    message_by_name = {}
    for message in messages:
        dependencies[str(message.name)] = set(message.get_dependencies())
        message_by_name[str(message.name)] = message
    
    for msgname in toposort2(dependencies):
        if msgname in message_by_name:
            yield message_by_name[msgname]

def make_identifier(headername):
    '''Make #ifndef identifier that contains uppercase A-Z and digits 0-9'''
    result = ""
    for c in headername.upper():
        if c.isalnum():
            result += c
        else:
            result += '_'
    return result

def generate_header(filename, namespaces, dependencies, headername, enums, messages, extensions, options):
    '''Generate content for a header file.
    Generates strings, which should be concatenated and stored to file.
    '''

    if len(enums) == 0 and len(messages)==0:
        return

    yield '/* Enum count %d */\n\n' % len(enums)
    yield '/* Message count %d */\n\n' % len(messages)    
    
    yield '/* Automatically generated lua serialization header */\n'
    if options.notimestamp:
        yield '/* Generated by %s */\n\n' % (lua_generator_version)
    else:
        yield '/* Generated by %s at %s. */\n\n' % (lua_generator_version, time.asctime())
    
    symbol = make_identifier(headername)
    yield '#ifndef LUA_%s_INCLUDED\n' % symbol
    yield '#define LUA_%s_INCLUDED\n' % symbol
    try:
        yield options.libformat % ('Engine/Framework/Script/LuaSerialization.h')
    except TypeError:
        # no %s specified - use whatever was passed in as options.libformat
        yield options.libformat
    yield '\n'
    
    for dependency in dependencies:
        noext = os.path.splitext(dependency)[0]
        yield options.genformat % (noext + options.extension + '.h')

    # Include the generated .pb.h file for this header
    noext = os.path.splitext(filename)[0]
    yield options.genformat % (noext + '.pb.h')
    yield '\n'

    yield '#if PB_PROTO_HEADER_VERSION != 30\n'
    yield '#error Regenerate this file with the current version of lua generator.\n'
    yield '#endif\n'
    yield '\n'

    yield 'namespace usg {\n\n'

    for msg in sort_dependencies(messages):
        yield msg.decl()

    yield '\n'

    for enum in enums:
        yield enum.decl()

    yield '}\n'
    
    # End of header
    yield '\n#endif\n'

def generate_source(filename, namespaces, dependencies, headername, sourcename, enums, messages, extensions, options):
    '''Generate content for a source file.
    Generates strings, which should be concatenated and stored to file.
    '''
    if len(enums) == 0 and len(messages)==0:
        yield '#include "Engine/Common/Common.h"'
        return

    yield '/* Enum count %d */\n\n' % len(enums)
    yield '/* Message count %d */\n\n' % len(messages)

    yield '/* Automatically generated lua serialization source */\n'
    if options.notimestamp:
        yield '/* Generated by %s */\n\n' % (lua_generator_version)
    else:
        yield '/* Generated by %s at %s. */\n\n' % (lua_generator_version, time.asctime())

    yield '#include "Engine/Common/Common.h"\n'
    yield '#include "%s"\n\n' % headername
    yield '#include "Engine/Framework/EventManager.h"\n'
    yield '#include "Engine/Framework/Script/LuaEvents.h"\n'

    for msg in sort_dependencies(messages):
        yield str(msg) + '\n'

    for enum in enums:
        yield str(enum) + '\n\n'

# ---------------------------------------------------------------------------
#                    Options parsing for the .proto files
# ---------------------------------------------------------------------------

from fnmatch import fnmatch

def read_options_file(infile):
    '''Parse a separate options file to list:
        [(namemask, options), ...]
    '''
    results = []
    for line in infile:
        line = line.strip()
        if not line or line.startswith('//') or line.startswith('#'):
            continue
        
        parts = line.split(None, 1)
        opts = nanopb_pb2.NanoPBOptions()
        text_format.Merge(parts[1], opts)
        results.append((parts[0], opts))

    return results

class Globals:
    '''Ugly global variables, should find a good way to pass these.'''
    verbose_options = False
    separate_options = []
    matched_namemasks = set()

def get_nanopb_suboptions(subdesc, options, name):
    '''Get copy of options, and merge information from subdesc.'''
    new_options = nanopb_pb2.NanoPBOptions()
    new_options.CopyFrom(options)
    
    # Handle options defined in a separate file
    dotname = '.'.join(name.parts)
    for namemask, options in Globals.separate_options:
        if fnmatch(dotname, namemask):
            Globals.matched_namemasks.add(namemask)
            new_options.MergeFrom(options)
    
    # Handle options defined in .proto
    if isinstance(subdesc.options, descriptor.FieldOptions):
        ext_type = nanopb_pb2.nanopb
    elif isinstance(subdesc.options, descriptor.FileOptions):
        ext_type = nanopb_pb2.nanopb_fileopt
    elif isinstance(subdesc.options, descriptor.MessageOptions):
        ext_type = nanopb_pb2.nanopb_msgopt
    elif isinstance(subdesc.options, descriptor.EnumOptions):
        ext_type = nanopb_pb2.nanopb_enumopt
    else:
        raise Exception("Unknown options type")
    
    if subdesc.options.HasExtension(ext_type):
        ext = subdesc.options.Extensions[ext_type]
        new_options.MergeFrom(ext)
    
    if Globals.verbose_options:
        sys.stderr.write("Options for " + dotname + ": ")
        sys.stderr.write(text_format.MessageToString(new_options) + "\n")
    
    return new_options


# ---------------------------------------------------------------------------
#                         Command line interface
# ---------------------------------------------------------------------------

import sys
import os.path    
from optparse import OptionParser

optparser = OptionParser(
    usage = "Usage: nanopb_generator.py [options] file.pb ...",
    epilog = "Compile file.pb from file.proto by: 'protoc -ofile.pb file.proto'. " +
             "Output will be written to file.pb.h and file.pb.c.")
optparser.add_option("-x", dest="exclude", metavar="FILE", action="append", default=[],
    help="Exclude file from generated #include list.")
optparser.add_option("-e", "--extension", dest="extension", metavar="EXTENSION", default=".lua",
    help="Set extension to use instead of '.lua' for generated files. [default: %default]")
optparser.add_option("-f", "--options-file", dest="options_file", metavar="FILE", default="%s.options",
    help="Set name of a separate generator options file.")
optparser.add_option("-Q", "--generated-include-format", dest="genformat",
    metavar="FORMAT", default='#include "%s"\n',
    help="Set format string to use for including other .pb.h files. [default: %default]")
optparser.add_option("-L", "--library-include-format", dest="libformat",
    metavar="FORMAT", default='#include <%s>\n',
    help="Set format string to use for including the nanopb pb.h header. [default: %default]")
optparser.add_option("-T", "--no-timestamp", dest="notimestamp", action="store_true", default=False,
    help="Don't add timestamp to .pb.h and .pb.c preambles")
optparser.add_option("-q", "--quiet", dest="quiet", action="store_true", default=False,
    help="Don't print anything except errors.")
optparser.add_option("-v", "--verbose", dest="verbose", action="store_true", default=False,
    help="Print more information.")
optparser.add_option("-s", dest="settings", metavar="OPTION:VALUE", action="append", default=[],
    help="Set generator option (max_size, max_count etc.).")

def process_file(filename, fdesc, options):
    '''Process a single file.
    filename: The full path to the .proto or .pb source file, as string.
    fdesc: The loaded FileDescriptorSet, or None to read from the input file.
    options: Command line options as they come from OptionsParser.
    
    Returns a dict:
        {'headername': Name of header file,
         'headerdata': Data for the .h header file,
         'sourcename': Name of source file,
         'sourcedata': Data for the .cpp source file,
        }
    '''
    toplevel_options = nanopb_pb2.NanoPBOptions()
    for s in options.settings:
        text_format.Merge(s, toplevel_options)
    
    if not fdesc:
        data = open(filename, 'rb').read()
        fdesc = descriptor.FileDescriptorSet.FromString(data).file[0]
    
    # Check if there is a separate .options file
    had_abspath = False
    try:
        optfilename = options.options_file % os.path.splitext(filename)[0]
    except TypeError:
        # No %s specified, use the filename as-is
        optfilename = options.options_file
        had_abspath = True

    if os.path.isfile(optfilename):
        if options.verbose:
            sys.stderr.write('Reading options from ' + optfilename + '\n')

        Globals.separate_options = read_options_file(open(optfilename, "rU"))
    else:
        # If we are given a full filename and it does not exist, give an error.
        # However, don't give error when we automatically look for .options file
        # with the same name as .proto.
        if options.verbose or had_abspath:
            sys.stderr.write('Options file not found: ' + optfilename)

        Globals.separate_options = []

    Globals.matched_namemasks = set()
    
    # Parse the file
    file_options = get_nanopb_suboptions(fdesc, toplevel_options, Names([filename], fdesc.package.split('.')))
    enums, messages, extensions = parse_file(fdesc, file_options)

    # Decide the file names
    noext = os.path.splitext(filename)[0]
    headername = noext + options.extension + '.h'
    headerbasename = os.path.basename(headername)
    sourcename = noext + options.extension + '.cpp'
    sourcebasename = os.path.basename(sourcename)
    
    # List of .proto files that should not be included in the C header file
    # even if they are mentioned in the source .proto.
    excludes = ['nanopb.proto', 'google/protobuf/descriptor.proto', 'Engine/Core/usagipb.proto'] + options.exclude
    dependencies = [d for d in fdesc.dependency if d not in excludes]

    namespaces = []
    if fdesc.package:
        namespaces = fdesc.package.split('.')
    
    headerdata = ''.join(generate_header(filename, namespaces, dependencies, headerbasename,
                                         enums, messages, extensions, options))
    sourcedata = ''.join(generate_source(filename, namespaces, dependencies, headerbasename, sourcebasename,
                                         enums, messages, extensions, options))

    # Check if there were any lines in .options that did not match a member
    unmatched = [n for n,o in Globals.separate_options if n not in Globals.matched_namemasks]
    if unmatched and not options.quiet:
        sys.stderr.write("Following patterns in " + optfilename + " did not match any fields: "
                         + ', '.join(unmatched) + "\n")
        if not Globals.verbose_options:
            sys.stderr.write("Use  protoc --nanopb-out=-v:.   to see a list of the field names.\n")

    return {'headername': headername, 'headerdata': headerdata, 'sourcename': sourcename, 'sourcedata': sourcedata}
    
def main_cli():
    '''Main function when invoked directly from the command line.'''
    
    options, filenames = optparser.parse_args()
    
    if not filenames:
        optparser.print_help()
        sys.exit(1)
    
    if options.quiet:
        options.verbose = False

    Globals.verbose_options = options.verbose
    
    for filename in filenames:
        results = process_file(filename, None, options)
        
        if not options.quiet:
            sys.stderr.write("Writing to " + results['headername'] + "\n")
    
        open(results['headername'], 'w').write(results['headerdata'])
        open(results['sourcename'], 'w').write(results['sourcedata'])

def main_plugin():
    '''Main function when invoked as a protoc plugin.'''

    import sys
    if sys.platform == "win32":
        import os, msvcrt
        # Set stdin and stdout to binary mode
        msvcrt.setmode(sys.stdin.fileno(), os.O_BINARY)
        msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)
    
    data = sys.stdin.read()
    request = plugin_pb2.CodeGeneratorRequest.FromString(data)
    
    import shlex
    args = shlex.split(request.parameter)
    options, dummy = optparser.parse_args(args)
    
    Globals.verbose_options = options.verbose
    
    response = plugin_pb2.CodeGeneratorResponse()
    
    for filename in request.file_to_generate:
        for fdesc in request.proto_file:
            if fdesc.name == filename:
                results = process_file(filename, fdesc, options)
                
                f = response.file.add()
                f.name = results['headername']
                f.content = results['headerdata']

                f = response.file.add()
                f.name = results['sourcename']
                f.content = results['sourcedata']

    sys.stdout.write(response.SerializeToString())

if __name__ == '__main__':
    # Check if we are running as a plugin under protoc
    if 'protoc-gen-' in sys.argv[0] or '--protoc-plugin' in sys.argv:
        main_plugin()
    else:
        main_cli()

