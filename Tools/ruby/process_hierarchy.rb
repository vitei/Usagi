# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
# process_hierarchy.rb
# Converts a YAML hierarchy file to a binary file that contains a
# stream of components belonging to each entity. Parent-child
# relationships for each entity are also written to this file.

require 'erb'
require 'matrix'
require 'optparse'
require 'pathname'
require 'set'
require 'yaml'
require 'zlib'

require_relative 'lib/skeletonextractor'

$defaults = {}
$entities = []
$identifiers = Set.new
$options = {
  includes: Set.new(["."]),
  defaults: [],
  require_dirs: []
}

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options] infile"
  opts.on( '-Idir', 'Make all files in a directory available to inherit from' ) do |i|
    $options[:includes] << i
  end

  opts.on( '-Rdir', 'Require all *.pb.rb files in a directory' ) do |r|
    STDERR.puts "add " + r + " to req dirs"
    $options[:require_dirs] << r
  end

  opts.on( '-d file', 'Specifies the component default parameters file' ) do |f|
    $options[:defaults] << f
  end

  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $options[:output] = f
  end

  opts.on( '-m dir', 'Specifies the output directory for models' ) do |f|
    $options[:model_out] = f
  end

  opts.on( '-g', '--debug', 'Show debug output' ) do
    $options[:debug] = true
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
require_relative 'lib/matrix_util'

ENTITY_SRC = ARGV[0]

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
      [f.name, merge_with_defaults(f.proxy_class, defaultVals.deep_merge(passedVals))]
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
	  empty_val = f.repeated? ? [] : {}
	  value = val.has_key?(f.name) ? val[f.name] : empty_val
	  if value.is_a? Array
        fields = value.map{|v|fields_from_hash(p, v)}
        acc.deep_merge({f.name => fields.map{|f| p.new(f)}})
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

def symbolize_keys(hash)
  if hash
    return hash.hmap do |k, v|
      [k.to_sym, v.is_a?(Hash) ? symbolize_keys(v) : v]
    end
  end
end

def find_include(entity_name)
    relative_path = $options[:includes].find do |i|
      path = File.join(i, entity_name + ".yml")
      File.exists? path
    end

    raise "Couldn't find entity \"#{entity_name}\"" if !relative_path

    return File.join(relative_path, entity_name + ".yml")
end

def import(data, entity_name, already_loaded, override_data)
  if !already_loaded.has_key? entity_name
    already_loaded[entity_name] = :loading
    e = symbolize_keys(data)

    # the 'Overrides' keyword defines a list of hashes
    overrides = e.delete(:Overrides) || []

    overrides.each do |override|
      targetEntityID = override.delete("EntityWithID")
      raise "No target defined for override!" if targetEntityID == nil

      override_data[targetEntityID.to_sym] = import(override, "#{targetEntityID}_override", already_loaded, override_data).inject({}) { |acc, components| acc.deep_merge components }
    end

    inherits_from = e.delete(:Inherits) || []
    inherited_data = []

    inherits_from.each do |inherited_entity|
      filename = find_include(inherited_entity)
      Tracker::DepSet.instance.addDep(filename)
      content = File.open(filename, 'r') { |f| ERB.new(f.read).result }
      inherited_data << import(YAML.load(content), File.basename(filename, '.yml'), already_loaded, override_data)
    end

    override_data.each_pair do |name, data|
      next if ! e.has_key?(:Identifier) || e[:Identifier][:name].nil? || e[:Identifier][:name].to_sym != name
      e = e.deep_merge(data)
    end

    already_loaded[entity_name] = :loaded
    inherited_data.flatten << e
  else
    raise "Recursive inheritance detected trying to import \"#{entity_name}\"" if already_loaded[entity_name] == :loading
    []
  end
end

if $options[:defaults]
  $defaults = $options[:defaults].inject({}) do |acc, filename|
    File.open(filename , 'r') do |f|
      content = ERB.new(f.read).result
      acc.deep_merge(symbolize_keys( YAML.load(content) ))
    end
  end
end

# The count of entities we've processed so far. This is used
# to assign unique ID's to them.
$global_entity_count = 0

def get_unique_name(component_data)
  name = ''

  if component_data['Identifier']
    name = component_data['Identifier']['name']
  else
    name = "__#{$global_entity_count}"
    $global_entity_count += 1
  end

  return name
end

class Constants
  NUL = "\0"
  SKELETON_PATH = '_build/skel'
end

def create_bone_hierarchy(model_component)
  path = Pathname.new(Constants::SKELETON_PATH)
  modelDepPath = Pathname.new($options[:model_out])
  path = path.join(model_component[:name])
  modelDepPath = modelDepPath.join(model_component[:name])
  extensions = {'.vmdf' => '.vmdf.xml'}

  path = path.sub_ext(extensions[path.extname])
  Tracker::DepSet.instance.addDep(modelDepPath.to_path)

  if path.file?
    hierarchy = SkeletonExtractor::extract(path.to_path)
  else
    message = "WARNING! Model '#{modelDepPath.to_path}' not found!\n"
    warn message
    # The build system apparently allows us to fail silently the first time, and there is every chance
    # the model hasn't been processed yet
    exit
  end

  return hierarchy
end

# if both a matrix component and a transform component are defined,
# and the matrix component has valid data, copy the position and
# rotation information into the transform component
def copy_matrix_to_transform(entity)
  hasValidMatrix = entity.has_key?(:MatrixComponent) && ! entity[:MatrixComponent].nil? &&
    entity[:MatrixComponent].has_key?(:matrix) &&
    entity[:MatrixComponent][:matrix].has_key?(:m) &&
    entity[:MatrixComponent][:matrix][:m].is_a?(Array) &&
    entity[:MatrixComponent][:matrix][:m].length == 16
  hasTransform = entity.has_key?(:TransformComponent)

  if !hasValidMatrix || !hasTransform
    return
  end

  matrix = entity[:MatrixComponent][:matrix][:m]
  position = {:x => matrix[12], :y => matrix[13], :z => matrix[14]}

  rotationMatrix = Matrix.build(4, 4) {|row, col| matrix[(row * 4) + col]}
  quaternion = rotationMatrix.rotation_matrix_to_quaternion()
  rotation = quaternion.each_with_object({}) {|(k,v), h| h[k.to_sym] = v}

  entity[:TransformComponent] = {:position => position, :rotation => rotation}
end

def process_entity(data, entity_name, override_data = {})
  already_loaded = {}
  children = []
  initializer_events = []

  e = import(data, entity_name, already_loaded, override_data).inject({}) do |acc, components|
    children = components.delete(:Children) if components.has_key?(:Children)
    initializer_events = components.delete(:InitializerEvents) if components.has_key?(:InitializerEvents)
    acc.deep_merge components
  end

  if $options[:debug]
    STDERR.puts "processing entity: #{entity_name}"
    STDERR.puts " data: #{e}"
  end

  copy_matrix_to_transform(e)

  if e.has_key?(:ModelComponent) and not e[:ModelComponent].nil?
    if e[:ModelComponent].has_key?(:name)
      hierarchy = create_bone_hierarchy(e[:ModelComponent])
      if not hierarchy.nil?
        children << hierarchy
      end
    end
  end

  # check for duplicate ID's, except for bone ID's
  if e.has_key?(:Identifier) && ! e.has_key?(:BoneComponent)
    if $identifiers.include?(e[:Identifier][:name])
      message = "WARNING! Identifier '#{e[:Identifier][:name]}' has already been used!\n"
      message += " Entity data: #{e}"
      warn message
    else
      $identifiers.add(e[:Identifier][:name])
    end
  end

  children.flatten!
  componentCount = 0

  component_data = e.inject([]) do |acc, (ctype, cvalue)|
    # syntactic sugar to translate '~Merge' into 'Processor.Merge'
    ctype = :'Processor.Merge' if ctype == :'~Merge'

    protocol = find_pb(ctype)
    raise("Component type not defined: #{ctype}") if !protocol

    fieldsHash = fields_from_hash(protocol, symbolize_keys(cvalue))
    cdata = protocol.new(fieldsHash).serialize_to_string
    crc = Zlib.crc32(ctype.to_s)
    cid = Usg::ComponentHeader.new( id: crc, byteLength: cdata.length + 1).serialize_to_string
    componentCount += 1

    acc << cid + Constants::NUL + cdata + Constants::NUL
  end

  component_data = [] if component_data.nil?

  entity_header = Usg::EntityHeader.new(componentCount: componentCount,
                                        childEntityCount: children.length,
                                        initializerEventCount: initializer_events.length)

  initializer_event_data = initializer_events.inject([]) do |acc, evt|
    add = nil
    evt.each do |evt_name_str, evt_data|
      evt_name = evt_name_str.to_sym
      protocol = find_pb_event(evt_name)
      raise("Event type not defined: #{evt_name}") if !protocol

      fieldsHash = fields_from_hash(protocol, symbolize_keys(evt_data))

      edata = protocol.new(fieldsHash).serialize_to_string

      crc = Zlib.crc32(evt_name.to_s)
      eid = Usg::InitializerEventHeader.new( id: crc, byteLength: edata.length + 1, check: 777).serialize_to_string

      add = eid + Constants::NUL + edata + Constants::NUL
    end
    acc << add
  end
  initializer_event_data = [] if initializer_event_data.nil?

  data_of_children = children.inject([]) do |acc, child|
    name = get_unique_name(child)

    if $options[:debug]
      message = "  child #{name}"

      if e.has_key?(:Identifier) and e[:Identifier][:name]
        message += " (parent == #{e[:Identifier][:name]}):\n"
      end

      STDERR.puts message
    end

    childE = process_entity(child, name, override_data)
    acc << childE
  end

  outputElements = [entity_header.serialize_to_string,
                    component_data.join(''), initializer_event_data.join(''), data_of_children.join('')]

  return outputElements.join(Constants::NUL)
end

def write_file(entities)
  header = Usg::HierarchyHeader.new(entityCount: entities.length)
  out_stream = $options[:output] ? File.open($options[:output], "wb") : $stdout
  out_stream.binmode
  header.serialize(out_stream) << Constants::NUL
  entities.map{|e| out_stream.print e }
  out_stream.close if $options[:output]
end

content = File.open(ENTITY_SRC, 'r') { |f| ERB.new(f.read).result }
data = YAML.load(content)
entities = []

if data.is_a? Hash
  # the file contains a single entity
  entity_name = get_unique_name(data)
  entities<< process_entity(data, entity_name)
elsif data.is_a? Array
  # the file contains a list of entities
  data.each do |entity_data|
    $already_loaded = {}
    # clear the set of identifiers, becuause each node should
    # be processed independently
    $identifiers.clear
    entities << process_entity(entity_data, 'Placeholder')
  end
end

write_file(entities)

Tracker::writeDependenciesFile($options, $options[:output])
