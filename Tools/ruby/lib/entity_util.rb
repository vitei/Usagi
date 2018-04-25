# Usagi Engine, Copyright © Vitei, Inc. 2013

require 'set'

def validate_attributes(message, attributes)
  defined_fields = message.fields.values.map{|f| f.name}.to_set
  data_fields = attributes.keys.to_set
  undefined_fields = data_fields - defined_fields

  if undefined_fields.empty?
    return
  end

  raise "Error! The following fields are undefined for protocol buffer '#{message.fully_qualified_name}': #{undefined_fields.to_a.join(', ')}"
end

def find_pb_in_ns(obj, pb_type_ns)
  x, *xs = pb_type_ns

  if x
    find_pb_in_ns(obj.const_get(x), xs) if obj.const_defined?(x)
  else
    Tracker::DepSet.instance.addDep(obj.to_s, true)

    obj
  end
end

def find_pb_event(pb_type)
  pb_type_ns = pb_type.to_s.split(".").map{ |t| t.to_sym }

  find_pb_in_ns(Object.const_get(:Events),          pb_type_ns) ||
  find_pb_in_ns(Usg::const_get(:Events),            pb_type_ns)
end

def find_pb(pb_type)
  pb_type_ns = pb_type.to_s.split(".").map{ |t| t.to_sym }

  find_pb_in_ns(Object,                             pb_type_ns) ||
  find_pb_in_ns(Object.const_get(:Usg),             pb_type_ns) ||
  find_pb_in_ns(Object.const_get(:Components),      pb_type_ns) ||
  find_pb_in_ns(Usg::const_get(:Ai),                pb_type_ns) ||
  find_pb_in_ns(Usg::const_get(:Components),        pb_type_ns) ||
  find_pb_in_ns(Usg::Ai.const_get(:Pb),             pb_type_ns) ||
  find_pb_in_ns(Usg::Ai::const_get(:Components),    pb_type_ns)
end

def find_bt_pb_enum(pb_type)
  pb_type_ns = pb_type.to_s.split(".").map{ |t| t.to_sym }
  find_pb_in_ns(Object,                             pb_type_ns) ||
  find_pb_in_ns(Object.const_get(:Usg),             pb_type_ns) ||
  find_pb_in_ns(Usg::const_get(:Ai),                pb_type_ns) ||
  find_pb_in_ns(Usg::const_get(:Components),        pb_type_ns) ||
  find_pb_in_ns(Usg::Ai.const_get(:Pb),             pb_type_ns) ||
  find_pb_in_ns(Usg::Ai::const_get(:Components),    pb_type_ns)
end

def find_bt_pb(pbType)
  s = pbType.split("_")
  pb = nil
  pb = find_pb_in_ns(Usg::Ai.const_get(s[0].to_sym), pbType.to_sym) if s.length > 1

  pb                                       ||
  find_pb_in_ns(Object, s)                 ||
  find_pb_in_ns(Object.const_get(:Usg), s) ||
  find_pb_in_ns(Usg::const_get(:Ai), s)
end