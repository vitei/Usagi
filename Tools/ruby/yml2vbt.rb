# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#yml2vbt.rb
#Converts a YAML file to a binary behavior tree file

require 'yaml'
require 'optparse'

require_relative 'tracker'

require_relative 'lib/entity_util'
require 'Engine/AI/BehaviorTree/BehaviorCommon.pb.rb'
require 'Tank/AI/TankBehaviorCommon.pb.rb'

### CONVERTER CLASS

class BehaviorTreeConverter
  class Constants
    NUL = "\0"
  end

  def run(input, output)
    content = File.open(input, 'r')
    yaml = YAML.load(content)
    @@pb_file = File.open(output, 'wb')
    @@pb_file.binmode
    parse_tree(yaml)
    @@pb_file.close
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
    sub = name.split("_")
    bh = find_bt_pb(sub[1]).new
    behaviorData.each {
      |key,value|
      # some of the data we have are protobuf classes
      if value.is_a? String
        data = find_bt_pb(value) if value.is_a?String
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

  def add_composite
    className = get_behavior_name
    composite_header = Usg::Ai::CompositeHeader.new
    composite_type = find_bt_pb("CompositeType_" + className)
    composite_header.compositeType = composite_type
    composite = find_bt_pb(className + "Header").new({:numChildren => get_num_children})
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
    dec_type = find_bt_pb(className)
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
    bh_type = find_bt_pb(className)
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
$options = {}

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options] infile"
  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $options[:output] = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

Tracker::addDepfileOption($options, optparser)

optparser.parse!
raise "ERROR: No input file specified!" if ARGV.length != 1

Tracker::writeDependenciesFile($options, $options[:output]) if $options.has_key?(:output)

SRC = ARGV[0]

BehaviorTreeConverter.new().run(SRC, $options[:output])
#BehaviorTreeConverter.new().run("C:\\Users\\Olof\\Desktop\\behaviorTree.yml", "C:\\Users\\Olof\\Desktop\\behaviorTree.pb")
