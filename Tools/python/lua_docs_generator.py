#!/usr/bin/python
# vim: set fileencoding=utf-8 :

'''Generate lua documentation from .proto definitions'''

import sys
import importlib
importlib.reload(sys)
sys.setdefaultencoding("utf-8")

import zlib
import os.path
import tempfile

#### IMPORT ALL THE MODULES (kinda hacky) ####

# Kinda nasty... hardcoding the path to nanopb's generator stuff.
# This is because I couldn't figure out how to set the PYTHONPATH
# variable externally in windows :-(
scriptdir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.abspath(scriptdir + '\\..\\..\\Engine\\ThirdParty\\nanopb\\generator\\proto'))
sys.path.append(os.path.abspath(scriptdir + '\\..\\..\\Engine\\ThirdParty\\nanopb\\generator'))
sys.path.append(os.path.abspath(scriptdir + '\\..\\..\\Engine\\Core'))

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
    import proto.plugin_pb2 as plugin_pb2
except:
    sys.stderr.write('''
         ********************************************************************
         *** Failed to import the protocol definitions for generator.     ***
         *** You have to run 'make' in the nanopb/generator/proto folder. ***
         ********************************************************************
    ''' + '\n')
    raise

try:
    import usagipb_pb2 as usagipb_pb2
except:
    sys.stderr.write('''
         **********************************************************************
         *** Failed to import the usagi protocol definitions for generator. ***
         **********************************************************************
    ''' + '\n')
    raise

#### NOW THE ACTUAL SCRIPT ####

from optparse import OptionParser
optparser = OptionParser(
    usage = "Usage: lua_docs_generator.py file.proto...")

def get_type_name(desc):
    FieldD = descriptor.FieldDescriptorProto
    datatypes = {
        FieldD.TYPE_BOOL:       '{{true}} 又は {{false}}',
        FieldD.TYPE_DOUBLE:     '実数',
        FieldD.TYPE_FLOAT:      '実数',
        FieldD.TYPE_INT32:      '整数',
        FieldD.TYPE_INT64:      '整数',
        FieldD.TYPE_SINT32:     '整数',
        FieldD.TYPE_SINT64:     '整数',
        FieldD.TYPE_UINT32:     '整数　正の数のみ',
        FieldD.TYPE_UINT64:     '整数　正の数のみ',
        FieldD.TYPE_STRING:     '文字列'
    }
    if desc.type in datatypes:
        return datatypes[desc.type]
    else:
        return '{{' + desc.type_name.encode('utf-8') + '}}'

class Field:
    def __init__(self, desc):
        self.name = desc.name.encode('utf-8')
        self.type = get_type_name(desc)

        usagi_options    = desc.options.Extensions[usagipb_pb2.usagi]
        nanopb_options   = desc.options.Extensions[usagipb_pb2.nanopb_pb2.nanopb]
        self.doc_en      = usagi_options.HasField("doc_en") and usagi_options.doc_en
        self.doc_jp      = usagi_options.HasField("doc_jp") and usagi_options.doc_jp

    def __str__(self):
        return '|{{' + self.name + '}}|' + self.type + '|' + (self.doc_jp or '') + '|'

class Event:
    def __init__(self, package, desc):
        self.name = desc.name.encode('utf-8')

        usagi_options    = desc.options.Extensions[usagipb_pb2.usagi_msg]
        nanopb_options   = desc.options.Extensions[usagipb_pb2.nanopb_pb2.nanopb_msgopt]
        self.lua_receive = nanopb_options.HasField("lua_receive") and nanopb_options.lua_receive
        self.lua_send    = nanopb_options.HasField("lua_send") and nanopb_options.lua_send
        self.doc_en      = usagi_options.HasField("doc_en") and usagi_options.doc_en
        self.doc_jp      = usagi_options.HasField("doc_jp") and usagi_options.doc_jp
        self.fields      = [Field(f) for f in desc.field]
        self.package     = package

    def __str__(self):
        return '\n'.join(self.generate_str())

    def generate_str(self):
        yield 'h5. {{' + self.package + '.' + self.name + '}}'
        if self.doc_jp:
            yield ''
            yield self.doc_jp
        yield ''
        has_documentation = any(f.doc_jp for f in self.fields)
        if has_documentation:
            yield '||フィールド名||値の型||説明||'
        else:
            yield '||フィールド名||値の型||'
        for f in self.fields:
            yield str(f)

def convert_to_file_descriptor(filename):
    PROTOC = os.path.abspath(scriptdir + '\\..\\bin\\protoc')

    # Note, you must run this script from the root of your project
    PROJECT_ROOT = os.path.abspath('.')
    USAGI_ROOT = os.path.abspath(scriptdir + '\\..\\..')
    NANOPB_PROTOS = os.path.abspath(scriptdir + '\\..\\..\\Engine\\ThirdParty\\nanopb\\generator\\proto')

    fd, tmppath = tempfile.mkstemp('.pb')
    cmd = '%s -I%s -I%s -I%s -o%s %s' % (PROTOC, PROJECT_ROOT, USAGI_ROOT, NANOPB_PROTOS, tmppath, os.path.abspath(filename))
    os.system(cmd)
    file = open(tmppath, 'rb')
    fdesc = file.read()
    file.close()
    os.close(fd)
    os.remove(tmppath)

    return fdesc

def extract_exposed_events(fdesc):
    fdesc = descriptor.FileDescriptorSet.FromString(fdesc).file[0]
    if hasattr(fdesc, 'message_type'):
        submsgs = fdesc.message_type
    else:
        submsgs = fdesc.nested_type

    for submsg in submsgs:
        event = Event(fdesc.package, submsg)
        if event.lua_receive or event.lua_send:
            yield event

def read_from_files(filenames):
    for filename in filenames:
        contents = open(filename).read()
        if 'lua_send' in contents or 'lua_receive' in contents:
            fdesc = convert_to_file_descriptor(filename)
            for event in extract_exposed_events(fdesc):
                yield event

options, filenames = optparser.parse_args()

if not filenames:
    optparser.print_help()
    sys.exit(1)

exposed_events = list(read_from_files(filenames))

receive_events = [e for e in exposed_events if e.lua_receive]
for e in receive_events:
    e.package = 'OnEvent'
print('h4. 受信できるイベント')
print('\n\n'.join(str(event) for event in receive_events))

send_events = [e for e in exposed_events if e.lua_send]
print('h4. 送信できるイベント')
print('\n\n'.join(str(event) for event in send_events))
