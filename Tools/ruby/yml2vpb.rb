# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#yml2vpb.rb
#Converts a YAML file to a protocol-buffer based binary file.
#The source YAML is expected to be a hash with one member,
#the key of which is the name of the protocol buffer message
#and the value is its data.

require 'optparse'

require 'set'
require 'erb'
require 'yaml'
require 'zlib'

$defaults = {}
$options = {
  require_dirs: [],
  defaults: []
}

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options] infile"
  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $options[:output] = f
  end

  opts.on( '-d file', 'Specifies the default parameters file' ) do |f|
    $options[:defaults] << f
  end

  opts.on( '-Rdir', 'Require all *.pb.rb files in a directory' ) do |r|
    $options[:require_dirs] << r
  end

  opts.on('--MF file', 'Write dependencies file for Ninja') do |f|
    $options[:depfile] = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

optparser.parse!
raise "ERROR: No input file specified!" if ARGV.length != 1

$options[:require_dirs].each do |dir|
  $LOAD_PATH << dir
  Dir[dir + "/**/*.pb.rb"].each do |f|
    require f.sub(dir + "/", "")
  end
end

require_relative 'tracker'
require_relative 'lib/entity_util'

SRC = ARGV[0]

# Useful function I wish hashes had.. should probably go in some utility file somewhere
class Hash
  def hmap(&block)
    Hash[self.map {|k, v| block.call(k,v) }]
  end
end

def extract_fields(f, val)
  if val.is_a? Array
    val.map do |v|
      fields = fields_from_hash(f.proxy_class, v)
      f.proxy_class.new(fields)
    end
  else
    fields = fields_from_hash(f.proxy_class, val)
    f.proxy_class.new(fields)
  end
end

# Useful function I wish hashes had.. should probably go in some utility file somewhere
class ::Hash
  def hmap(&block)
    Hash[self.map {|k, v| block.call(k,v) }.reject(&:empty?)]
  end

  # Deep merge found here: http://stackoverflow.com/questions/9381553/ruby-merge-nested-hash
  def deep_merge(second)
    merger = proc { |key, v1, v2| Hash === v1 && Hash === v2 ? v1.merge(v2, &merger) : [:undefined, nil, :nil].include?(v2) ? v1 : v2 }
    self.merge(second, &merger)
  end
end

def merge_with_defaults(protocol, hash)
  protocol.fields.hmap do |_, f|
    if !hash.has_key?(f.name)
      if f.repeated?
        next [f.name, []]
      elsif f.otype == :optional
        next []
      end
    end

    if f.is_a? ProtocolBuffers::Field::MessageField
      pname = f.proxy_class.name.gsub(/^.*::/, '').to_sym
      defaultVals = $defaults[pname] ? $defaults[pname] : {}
      passedVals = hash[f.name] ? hash[f.name] : {}
      if passedVals.is_a? Array
        [f.name, passedVals.map{|v| merge_with_defaults(f.proxy_class, defaultVals.deep_merge(v))}]
      else
        [f.name, merge_with_defaults(f.proxy_class, defaultVals.deep_merge(passedVals))]
      end
    else
      [f.name, hash.has_key?(f.name) ? hash[f.name] : f.default_value]
    end
  end
end

def fields_from_hash(protocol, cvalue)
  pname = protocol.name.gsub(/^.*::/, '').to_sym
  defaultVals = $defaults[pname] ? $defaults[pname] : {}
  attributes = defaultVals.deep_merge(cvalue ? cvalue : {})

  validate_attributes(protocol, attributes)
  val = merge_with_defaults(protocol, attributes)

  protocol.fields.inject({}) do |acc, (_, f)|
    if f.is_a? ProtocolBuffers::Field::MessageField
      p = f.proxy_class
      empty_val = f.repeated? ? [] : f.otype == :optional ? nil : {}
      value = val.has_key?(f.name) ? val[f.name] : empty_val
      if value.is_a? Array
        fields = value.map{|v|fields_from_hash(p, v)}
        acc.deep_merge({f.name => fields.map{|f| p.new(f)}})
      elsif value == nil
        acc.merge({f.name => nil})
      else
        fields = fields_from_hash(p, value)
        acc.deep_merge({f.name => p.new(fields)})
      end
    else
      if val.has_key?(f.name) || f.otype != :optional
        value = val.has_key?(f.name) ? val[f.name] : f.default_value
        value = "" if f.is_a?(ProtocolBuffers::Field::StringField) && value.nil?
        acc.deep_merge({f.name => value})
      else
        acc
      end
    end
  end
end

def symbolize(v)
  v.is_a?(Hash) ? symbolize_keys(v) :
    v.is_a?(Array) ? v.map{ |v1| symbolize(v1) } :
      v
end

def symbolize_keys(hash)
  if hash
    return hash.hmap do |k, v|
      [k.to_sym, symbolize(v)]
    end
  end
end

class Constants
  NUL = "\0"
end

def process(pb_type, pb_vals)
  protocol = find_pb(pb_type)
  raise "Protocol Buffer not defined: #{pb_type}" if !protocol

  fieldsHash = fields_from_hash(protocol, symbolize_keys(pb_vals))
  protocol.new(fieldsHash)
end

if $options[:defaults]
  $defaults = $options[:defaults].inject({}) do |acc, filename|
    File.open(filename , 'r') do |f|
      content = ERB.new(f.read).result
      acc.deep_merge(symbolize_keys( YAML.load(content) ))
    end
  end
end

content = File.open(SRC, "r:UTF-8") { |f| ERB.new(f.read).result }

out_stream = $options[:output] ? File.open($options[:output], "wb:UTF-8") : $stdout
out_stream.binmode

YAML.load_stream(content).each do |data|
  raise "Root node should be a hash" if not data.is_a? Hash
  raise "File should contain a single protocol buffer" if data.length != 1

  pb_type, pb_vals = data.to_a[0]
  pb = process(pb_type, pb_vals)
  out_data = pb.serialize_to_string

  out_stream.print (out_data + Constants::NUL)
end

if $options[:output]
  out_stream.close
  Tracker::writeDependenciesFile($options, $options[:output])
end
