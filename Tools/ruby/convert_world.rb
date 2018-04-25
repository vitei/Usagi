# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#convert_world.rb
#Reads a world definition from YAML input, and outputs the binary world data.

require 'optparse'

require 'erb'
require 'yaml'
require 'zlib'

Dir[File.dirname(__FILE__) + "/Protocols/*"].each do |f|
  proto_file = File.join ["Protocols", File.basename(f)]
  require_relative proto_file
end

$defaults = {}
$templates = {}
$entities = []
$options = {
  templatedirs: []
}

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options] infile"
  opts.on( '-Tdir', 'Add all files from a directory to the available templates' ) do |i|
    $options[:templatedirs] << i
  end

  opts.on( '-d file', 'Specifies the component default parameters file' ) do |f|
    $options[:defaults] = f
  end

  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $options[:output] = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

optparser.parse!

def loadYaml(stream)
  # In the case of multiple documents in a single, file, merges them all into
  # one document
  YAML.load_stream(ERB.new(stream.read).result).inject(:merge)
end

if $options[:defaults]
  $defaults = File.open($options[:defaults], 'r') { |f| loadYaml(f) }
end

$options[:templatedirs].each do |dir|
  Dir.entries(dir).select {|f| !File.directory? f}.each do |f|
    template_name = File.basename(f, ".yml").to_sym
    template = File.open("#{dir}/#{f}", 'r') {|s| loadYaml(s) }
    $templates[template_name] = template
  end
end

def symbolize_keys(hash)
  return hash.inject({}) do |acc,x|
    v = x[1].is_a?(Hash) ? symbolize_keys(x[1]) : x[1]
    acc.merge({x[0].to_sym => v})
  end
end

def generate_entity(name, components = {})
  entity_sym = name.to_sym
  if $templates.has_key? entity_sym
    template = $templates[entity_sym]
    template.merge(components).map do |k, v|
      merged_values = $defaults[k] ? $defaults[k].merge(v ? v : {}) : v
      fixed_keys = symbolize_keys(merged_values) if merged_values
      [k, fixed_keys]
    end
  else
    raise "Undefined entity type: #{name}"
  end
end

loadYaml(ARGF).each do |e|
  if e.is_a? Hash
    e.each do |type, components|
      $entities << generate_entity(type, components)
    end
  elsif e.is_a? String
    $entities << generate_entity(e)
  else
    raise "Unsupported entity specification: #{e.inspect}"
  end
end

def fields_from_hash(protocol, cvalue)
  protocol.fields.inject({}) do |acc, v|
    _, f = v
    if f.is_a? ProtocolBuffers::Field::MessageField then
      p = f.proxy_class
      value = cvalue.has_key?(f.name) ? cvalue[f.name] : {}
      fields = fields_from_hash(p, value)
      acc.merge({f.name => p.new(fields)})
    else
      value = cvalue.has_key?(f.name) ? cvalue[f.name] : f.default_value
      acc.merge({f.name => value})
    end
  end
end

out_stream = $options[:output] ? File.open($options[:output], "wb") : $stdout
out_stream.binmode

$entities.each do |e|
  component_data = e.inject([]) do |acc, c|
    ctype, cvalue = c
    protocol = Object.const_get(ctype)
    cid = [Zlib.crc32(ctype)].pack("V")
    cdata = protocol.new(fields_from_hash(protocol, cvalue)).serialize_to_string
    acc << cid + cdata
  end

  total_component_length = component_data.map{|d|d.length}.inject(:+)
  metahdr = [0, total_component_length].pack("VV")

  out_stream.print metahdr
  component_data.each{ |c| out_stream.print c }
end

out_stream.close if $options[:output]
