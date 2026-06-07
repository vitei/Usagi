# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#xml2vbt.rb
#Converts a XML file to a binary behavior tree file

require 'optparse'
require 'fileutils'

### CONVERTER CLASS

class XMLBehaviorTreeConverter
  class Constants
    NUL = "\0"
  end

  DEFAULT_PROJECT_BEHAVIOR_PROTO = 'Tank/AI/TankBehaviorCommon.pb.rb'
  DEFAULT_BEHAVIOR_TYPE_PREFIXES = {
    "behavior_" => "BehaviorType_",
    "tankbehavior_" => "TankBehaviorType_"
  }
  DEFAULT_DECORATOR_TYPE_PREFIXES = {
    "decorator_" => "DecoratorType_",
    "tankdecorator_" => "TankDecoratorType_"
  }

  def initialize(options = {})
    @options = options
    @behavior_type_prefixes = DEFAULT_BEHAVIOR_TYPE_PREFIXES.merge(options.fetch(:behavior_type_prefixes, {}))
    @decorator_type_prefixes = DEFAULT_DECORATOR_TYPE_PREFIXES.merge(options.fetch(:decorator_type_prefixes, {}))
  end

  def run(input, output)
    require 'nokogiri'

    @@xml = File.open(input, 'r') { |file| Nokogiri::XML(file) { |config| config.strict } }
    validate_tree(@@xml)
    load_converter_dependencies

    File.open(output, "wb") do |pb_file|
      @@pb_file = pb_file
      @@pb_file.binmode
      parse_tree(@@xml)
    end
  ensure
    @@xml = nil
    @@pb_file = nil
  end

  def load_converter_dependencies
    require_relative 'tracker'
    require_relative 'lib/entity_util'
    require 'Engine/AI/BehaviorTree/BehaviorCommon.pb.rb'

    if @options.fetch(:use_default_project_behavior_proto, true)
      require_project_behavior_proto(DEFAULT_PROJECT_BEHAVIOR_PROTO, true)
    end

    @options.fetch(:project_behavior_protos, []).each do |proto|
      require_project_behavior_proto(proto, false)
    end
  end

  def require_project_behavior_proto(proto, optional)
    require proto
  rescue LoadError => e
    raise e unless optional && missing_required_feature?(e, proto)
  end

  def missing_required_feature?(error, feature)
    (error.respond_to?(:path) && error.path == feature) ||
      error.message.include?(" -- #{feature}") ||
      error.message.include?("such file -- #{feature}")
  end

  def resolve_bt_pb(pb_type, context = nil)
    pb = find_bt_pb(pb_type)
    return pb unless pb.nil?

    raise unresolved_pb_message(pb_type, context)
  rescue NameError
    raise unresolved_pb_message(pb_type, context)
  end

  def resolve_bt_pb_enum(pb_type, context = nil)
    pb = find_bt_pb_enum(pb_type)
    return pb unless pb.nil?

    raise unresolved_pb_message(pb_type, context)
  rescue NameError
    raise unresolved_pb_message(pb_type, context)
  end

  def unresolved_pb_message(pb_type, context)
    message = "Could not resolve behavior tree protobuf type '#{pb_type}'"
    message += " for #{context}" if context
    message + ". Add --project-behavior-proto for project-specific behavior data."
  end

  def parse_tree(xml)

    # Get the start node.
    startNode = xml.xpath("//*[@start='true']").first

    currNode = startNode;
    while currNode != nil do
      output_node(xml, currNode)
      currNode = next_node(xml, currNode)
    end

  end

  def output_node(xml, current)

    type = get_behavior_type(current)

    case type
    when :composite
      # puts "composite"
      add_composite(current)
    when :decorator
      # puts "decorator"
      add_decorator(current)
    when :behavior
      # puts "behavior"
      add_action(current)
    else
      raise "Unrecognized XML behavior node prefix for #{current.name}. Add --behavior-prefix or --decorator-prefix for project-specific XML behavior nodes."
    end

  end

  def validate_tree(xml)
    raise "XML document is empty" if xml.root.nil?

    start_nodes = xml.xpath("//*[@start='true']")
    raise "XML behavior tree must contain one start='true' node" if start_nodes.empty?
    raise "XML behavior tree contains multiple start='true' nodes" if start_nodes.length > 1

    behavior_nodes = xml.xpath("//*").select { |node| get_behavior_type(node) != nil }
    raise "XML behavior tree contains no recognized behavior nodes. Add --behavior-prefix or --decorator-prefix for project-specific XML behavior nodes." if behavior_nodes.empty?
  end

  def add_composite(current)

    # this is the name expected by protobuf
    behaviorName = get_behavior_name(current)

    # set composite type
    composite_header = Usg::Ai::CompositeHeader.new
    composite_type = resolve_bt_pb("CompositeType_" + behaviorName, "composite #{behaviorName}")
    composite_header.compositeType = composite_type

    # set numChildren
    composite = resolve_bt_pb(behaviorName + "Header", "composite #{behaviorName}").new({
      :numChildren => get_num_children(current)
    })

    # set numSuccess
    if behaviorName == "Parallel"
      composite.numSuccess = get_num_success(current)
    end

    @@pb_file.print(composite_header.serialize_to_string + Constants::NUL)
    @@pb_file.print(composite.serialize_to_string + Constants::NUL)

  end

  def add_decorator(current)

    # add the base type header (telling the file that this is a decorator)
    composite_header = Usg::Ai::CompositeHeader.new
    composite_header.compositeType = Usg::Ai::CompositeType::Decorator
    @@pb_file.print(composite_header.serialize_to_string + Constants::NUL)

    # find out what type of decorator this is
    className = get_behavior_name(current)
    dec_type = resolve_bt_pb(className, "decorator #{className}")
    dec_header = Usg::Ai::DecoratorHeader.new
    dec_header.decoratorHeader = dec_type
    @@pb_file.print(dec_header.serialize_to_string + Constants::NUL)

    # @todo if the decorator has data, we need to get it... but, how?
    parse_behavior_data(current)

  end

  def add_action(current)

    # add the base type header (telling the file that this is a action)
    composite_header = Usg::Ai::CompositeHeader.new
    composite_header.compositeType = Usg::Ai::CompositeType::Behavior
    @@pb_file.print(composite_header.serialize_to_string + Constants::NUL)

    # find out what type of action this is
    className = get_behavior_name(current)
    bh_type = resolve_bt_pb(className, "action #{className}")
    bh_header = Usg::Ai::BehaviorHeader.new
    bh_header.behaviorType = bh_type
    @@pb_file.print(bh_header.serialize_to_string + Constants::NUL)

    # @todo if the action has data, we need to get it... but, how?
    parse_behavior_data(current)

  end

  def parse_behavior_data(current)

    # base attributes, to be ignored
    baseAttributes = [
      "name",
      "PositionX",
      "PositionY",
      "numChildren",
      "numSuccess",
      "start"
    ]
    attributes = current.attributes.reject {
      |key, value| baseAttributes.include?(key)
    }
    attributeNames = attributes.collect { |key, value| key }

    hasData = false
    if (attributes.length > 0)
      name = get_behavior_name(current)
      data_type = get_behavior_data_type_name(current)
      # puts (name)
      bh = resolve_bt_pb(data_type, "data for #{name}").new
      fields = bh.fields.collect { |k, v| v }.sort! {
        | x, y | x.tag <=> y.tag
      }
      if (fields.length > 0)
        fields.each {
          |field|
          exists = attributeNames.include?(field.name.to_s)
          if exists
            if (!attributes.has_key?(field.name.to_s))
              # Use default value
              value = field.default_value
            else
              value = attributes[field.name.to_s].value  
            end
            hasData = true
            fieldClass = field.class.to_s
            case fieldClass
            when "ProtocolBuffers::Field::EnumField"
              btType = resolve_bt_pb_enum(data_type, "enum data for #{name}")
              integerValue = btType.fields.find{|key,val| val.name.to_s == field.name.to_s }[1].value_to_name.find{|integerKey, stringName| stringName == value.to_s}[0]
              bh.attributes = { field.name.to_sym => integerValue }
            when "ProtocolBuffers::Field::BoolField"
              # print "\twriting " + (value == "true").to_s + " to " + field.name.to_s
              # puts
              bh.attributes = { field.name.to_sym => (value == "true") }
            when "ProtocolBuffers::Field::FloatField"
              # print "\twriting " + value.to_s + " to " + field.name.to_s
              # puts
              bh.attributes = { field.name.to_sym => value.to_f }
            when "ProtocolBuffers::Field::Int32Field"
              # print "\twriting " + value.to_s + " to " + field.name.to_s
              # puts
              bh.attributes = { field.name.to_sym => value.to_i }
            when "ProtocolBuffers::Field::StringField"
              # print "\twriting " + value.to_s + " to " + field.name.to_s
              # puts
              bh.attributes = { field.name.to_sym => value.to_s }
            else
              abort("couldn't write to field: " + fieldClass)
            end
          end
        }
      end
    end
    if hasData
      @@pb_file.print(bh.serialize_to_string + Constants::NUL)
    end

  end

  def get_behavior_name(current)
    behaviorName = current.name
    if behaviorName.start_with?("composite_")
      behaviorName = current.name.sub("composite_", "")
    else
      type_prefixes.each do |key, value|
        if behaviorName.start_with?(key)
          behaviorName = behaviorName.sub(key, value)
        end
      end
    end

    return behaviorName
  end

  def get_behavior_data_type_name(current)
    type_prefixes.each_key do |prefix|
      return current.name.sub(prefix, "") if current.name.start_with?(prefix)
    end

    nil
  end

  def type_prefixes
    @behavior_type_prefixes.merge(@decorator_type_prefixes)
  end

  def get(current)
    return current["numChildren"].to_i
  end
  def get_num_children(current)
    return current["numChildren"].to_i
  end

  def get_num_success(current)
    return current["numSuccess"].to_i
  end

  def get_behavior_type(current)
    return :composite if current.name.start_with?("composite_")
    return :behavior if @behavior_type_prefixes.keys.any? { |prefix| current.name.start_with?(prefix) }
    return :decorator if @decorator_type_prefixes.keys.any? { |prefix| current.name.start_with?(prefix) }

    nil
  end

  def next_node(xml, current, last = nil)

    childNodes = get_ordered_children(xml, current)

    # If we have no children, get the next node from our parent.
    if childNodes.length == 0
      name = current["name"]
      transition = xml.xpath("//xmlns:transition[@toState='" + name + "']").first
      parentNode = xml.xpath("//*[@name='" + transition["fromState"] + "']").first
      return next_node(xml, parentNode, current)
    elsif last == nil
      # If we weren't just at a child node, return our first child.
      return childNodes[0]
    else
      # Get index of the child node we came from.
      hash = Hash[childNodes.map.with_index.to_a]
      idx = hash[last]
      if idx < childNodes.length - 1
        # If there's a subsequent item, return that.
        return childNodes[idx+1]
      else
        # If there isn't...
        name = current["name"]
        transition = xml.xpath("//xmlns:transition[@toState='" + name + "']").first
        if transition == nil
          # If we have no parent, return nil
          return nil
        else
          # Get the next node from our parent
          parentNode = xml.xpath("//*[@name='" + transition["fromState"] + "']").first
          return next_node(xml, parentNode, current)
        end
      end

      return nil
    end
  end

  def get_ordered_children(xml, node)

    # Get the transitions from our node.
    name = node["name"]
    childNodes = []
    children = xml.xpath("//xmlns:transition[@fromState='" + name + "']")
    children.each {
      |child|
      name = child["toState"]
      childNode = xml.xpath("//*[@name='" + name + "']")
      raise "Transition references missing node: #{name}" if childNode.empty?
      childNodes.push(childNode.first)
    }

    # Sort the nodes we're going to, by their X-position.
    childNodes.sort! {
      | x, y | x["PositionX"].to_i <=> y["PositionX"].to_i
    }

    return childNodes
  end

  # => Member variables
  @@xml = nil
  def self.xml
    @@xml
  end

  @@pb_file = nil
  def self.pb_file
    @@pb_file
  end

  @@current_bh = nil
  def self.current_bh
    @@current_bh
  end
end

### THE ACTUAL SCRIPT
$options = {
  load_paths: [],
  project_behavior_protos: [],
  use_default_project_behavior_proto: true,
  behavior_type_prefixes: {},
  decorator_type_prefixes: {}
}

def add_type_prefix(mapping, value)
  xml_prefix, type_prefix = value.split(':', 2)
  raise OptionParser::InvalidArgument, value if xml_prefix.nil? || xml_prefix.empty? || type_prefix.nil? || type_prefix.empty?

  mapping[xml_prefix] = type_prefix
end

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options] infile"
  opts.on( '-I dir', 'Add a Ruby load path before requiring protobuf bindings' ) do |dir|
    $options[:load_paths] << dir
  end

  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $options[:output] = f
  end

  opts.on( '--project-behavior-proto require', 'Require a project behavior protobuf Ruby binding' ) do |feature|
    $options[:project_behavior_protos] << feature
  end

  opts.on( '--behavior-prefix xml:type', 'Map an XML action prefix to a protobuf enum prefix' ) do |value|
    add_type_prefix($options[:behavior_type_prefixes], value)
  end

  opts.on( '--decorator-prefix xml:type', 'Map an XML decorator prefix to a protobuf enum prefix' ) do |value|
    add_type_prefix($options[:decorator_type_prefixes], value)
  end

  opts.on( '--no-default-project-behavior-proto', 'Do not optionally require the legacy Tank behavior binding' ) do
    $options[:use_default_project_behavior_proto] = false
  end

  opts.on('--MF file', 'Write dependencies file for Ninja') do |f|
    $options[:depfile] = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

def create_output_parent_dir(output)
  parent = File.dirname(output)
  return if parent.nil? || parent == "."

  raise "Output parent exists and is not a directory: #{parent}" if File.exist?(parent) && !File.directory?(parent)

  FileUtils.mkdir_p(parent)
end

def configure_load_paths(load_paths)
  load_paths.each do |dir|
    raise "Load path does not exist: #{dir}" unless File.directory?(dir)
    $LOAD_PATH.unshift(dir) unless $LOAD_PATH.include?(dir)
  end
end

begin
  optparser.parse!
  raise "No input file specified" if ARGV.length == 0
  raise "Expected exactly one input file, got #{ARGV.length}" if ARGV.length != 1
  raise "No output file specified" if !$options[:output] || $options[:output].empty?

  SRC = ARGV[0]
  raise "Input file does not exist: #{SRC}" unless File.file?(SRC)

  configure_load_paths($options[:load_paths])
  create_output_parent_dir($options[:output])

  XMLBehaviorTreeConverter.new($options).run(SRC, $options[:output])
  Tracker::writeDependenciesFile($options, $options[:output])
rescue OptionParser::ParseError, LoadError, StandardError => e
  warn "#{File.basename($0)}: ERROR: #{e.message}"
  exit 1
end
