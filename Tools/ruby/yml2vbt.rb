# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#yml2vbt.rb
#Converts a YAML file to a binary behavior tree file

require 'yaml'
require 'optparse'
require 'fileutils'

### CONVERTER CLASS

class BehaviorTreeConverter
  class Constants
    NUL = "\0"
  end

  VALID_BEHAVIOR_TYPES = ["Composite", "Decorator", "Action"]
  DEFAULT_PROJECT_BEHAVIOR_PROTO = 'Tank/AI/TankBehaviorCommon.pb.rb'

  def initialize(options = {})
    @options = options
  end

  def run(input, output)
    yaml = File.open(input, 'r') { |content| YAML.load(content) }
    validate_tree(yaml)
    load_converter_dependencies

    File.open(output, 'wb') do |pb_file|
      @@pb_file = pb_file
      @@pb_file.binmode
      parse_tree(yaml)
    end
  ensure
    @@pb_file = nil
  end

  def parse_tree(behaviorTree)
    behaviorTree.each {
      |behavior|
      set_current_behavior(behavior)
      type = get_behavior_type

      if type == "Composite"
        add_composite
      elsif type == "Decorator"
        add_decorator
      elsif type == "Action"
        add_action
      end
    }
  end

  def get_num_children
    @@current_bh["numChildren"]
  end

  def get_num_success
    @@current_bh["numSuccess"]
  end

  def get_behavior_name
    @@current_bh["name"]
  end

  def set_current_behavior(behavior)
    @@current_bh = behavior["Behavior"]
  end

  def get_behavior_type
    @@current_bh["type"]
  end

  def has_data
    @@current_bh["hasData"]
  end

  def parse_behavior_data(name)
    behaviorData = get_behavior_data
    sub = name.split("_", 2)
    data_type = sub[1]
    raise "Can not infer behavior data protobuf type from #{name}" if data_type.nil? || data_type.empty?

    bh = resolve_bt_pb(data_type, "data for #{name}").new
    behaviorData.each {
      |key,value|
      # some of the data we have are protobuf classes
      if value.is_a? String
        data = resolve_bt_pb(value, "data field #{key}")
        bh.attributes = { key.to_sym => data }
      else
        bh.attributes = { key.to_sym => value }
      end
    }
    @@pb_file.print(bh.serialize_to_string + Constants::NUL)
  end

  # Some behaviors or decorators will have associated data alongside logic (eg: distance measurements, min/max angles etc)
  def get_behavior_data
    @@current_bh["data"]
  end

  def validate_tree(behaviorTree)
    raise "YAML root must be a sequence of behavior entries" unless behaviorTree.is_a?(Array)

    behaviorTree.each_with_index do |behavior, index|
      raise "Behavior entry #{index} must be a map" unless behavior.is_a?(Hash)

      current = behavior["Behavior"]
      raise "Behavior entry #{index} must contain a Behavior map" unless current.is_a?(Hash)

      type = current["type"]
      raise "Behavior entry #{index} has invalid type #{type.inspect}" unless VALID_BEHAVIOR_TYPES.include?(type)
      raise "Behavior entry #{index} is missing name" if current["name"].nil? || current["name"].to_s.empty?

      if type == "Composite"
        raise "Composite behavior #{current["name"]} is missing numChildren" if current["numChildren"].nil?
        raise "Parallel behavior #{current["name"]} is missing numSuccess" if current["name"] == "Parallel" && current["numSuccess"].nil?
      end

      if current["hasData"] == true && !current["data"].is_a?(Hash)
        raise "Behavior #{current["name"]} declares hasData but data is not a map"
      end
    end
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

  def unresolved_pb_message(pb_type, context)
    message = "Could not resolve behavior tree protobuf type '#{pb_type}'"
    message += " for #{context}" if context
    message + ". Add --project-behavior-proto for project-specific behavior data."
  end

  def add_composite
    className = get_behavior_name
    composite_header = Usg::Ai::CompositeHeader.new
    composite_type = resolve_bt_pb("CompositeType_" + className, "composite #{className}")
    composite_header.compositeType = composite_type
    composite = resolve_bt_pb(className + "Header", "composite #{className}").new({:numChildren => get_num_children})
    if className == "Parallel"
      composite.numSuccess = get_num_success
    end
    @@pb_file.print(composite_header.serialize_to_string + Constants::NUL)
    @@pb_file.print(composite.serialize_to_string + Constants::NUL)
  end

  def add_decorator
    #first add the base type header (telling the file that this is a decorator)
    composite_header = Usg::Ai::CompositeHeader.new
    composite_header.compositeType = Usg::Ai::CompositeType::Decorator
    @@pb_file.print(composite_header.serialize_to_string + Constants::NUL)

    #then we need to find out what type of decorator this is
    className = get_behavior_name
    dec_type = resolve_bt_pb(className, "decorator #{className}")
    dec_header = Usg::Ai::DecoratorHeader.new
    dec_header.decoratorHeader = dec_type
    @@pb_file.print(dec_header.serialize_to_string + Constants::NUL)

    if has_data == true
      #if the decorator has data, we need to get it
      parse_behavior_data(className)
    end
  end

  def add_action
    #first add the base type header (telling the file that this is an action (behavior))
    composite_header = Usg::Ai::CompositeHeader.new
    composite_header.compositeType = Usg::Ai::CompositeType::Behavior
    @@pb_file.print(composite_header.serialize_to_string + Constants::NUL)

    #then we need to find out what type of action this is
    className = get_behavior_name
    bh_type = resolve_bt_pb(className, "action #{className}")
    bh_header = Usg::Ai::BehaviorHeader.new
    bh_header.behaviorType = bh_type
    @@pb_file.print(bh_header.serialize_to_string + Constants::NUL)

    if has_data == true
      #if the action has data, we need to get it
      parse_behavior_data(className)
    end
  end

  # => Member variables
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
  use_default_project_behavior_proto: true
}

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

  BehaviorTreeConverter.new($options).run(SRC, $options[:output])
  Tracker::writeDependenciesFile($options, $options[:output])
rescue OptionParser::ParseError, LoadError, StandardError => e
  warn "#{File.basename($0)}: ERROR: #{e.message}"
  exit 1
end
#BehaviorTreeConverter.new().run("C:\\Users\\Olof\\Desktop\\behaviorTree.yml", "C:\\Users\\Olof\\Desktop\\behaviorTree.pb")
