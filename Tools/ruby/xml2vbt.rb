# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#xml2vbt.rb
#Converts a XML file to a binary behavior tree file

require 'optparse'

require 'nokogiri'

require_relative 'tracker'

require_relative 'lib/entity_util'
require 'Engine/AI/BehaviorTree/BehaviorCommon.pb.rb'
require 'Tank/AI/TankBehaviorCommon.pb.rb'

### CONVERTER CLASS

class XMLBehaviorTreeConverter
  class Constants
    NUL = "\0"
  end
  def run(input, output)
    @@xml = Nokogiri::XML(File.open(input, 'r'))
    @@pb_file = File.open(output, "wb")
    @@pb_file.binmode
    parse_tree(@@xml)
    @@pb_file.close
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
    when "composite"
      # puts "composite"
      add_composite(current)
    when "decorator"
      # puts "decorator"
      add_decorator(current)
    when "tankdecorator"
      # puts "decorator"
      add_decorator(current)
    when "behavior"
      # puts "behavior"
      add_action(current)
    when "tankbehavior"
      # puts "behavior"
      add_action(current)
    end

  end

  def add_composite(current)

    # this is the name expected by protobuf
    behaviorName = get_behavior_name(current)

    # set composite type
    composite_header = Usg::Ai::CompositeHeader.new
    composite_type = find_bt_pb("CompositeType_" + behaviorName)
    composite_header.compositeType = composite_type

    # set numChildren
    composite = find_bt_pb(behaviorName + "Header").new({
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
    dec_type = find_bt_pb(className)
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
    bh_type = find_bt_pb(className)
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
      sub = name.split("_")
      # puts (name)
      bh = find_bt_pb(sub[1]).new
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
              btType = find_bt_pb_enum(sub[1])
              raise "Can not find protobuf " + sub[1] if btType == nil
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
      nameHash = {
        "behavior_" => "BehaviorType_",
        "decorator_" => "DecoratorType_",
        "tankbehavior_" => "TankBehaviorType_",
        "tankdecorator_" => "TankDecoratorType_"
      }

      nameHash.each do |key, value|
        if behaviorName.start_with?(key)
          behaviorName = behaviorName.sub(key, value)
        end
      end
    end

    return behaviorName
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
    type = current.name.split('_')[0]
    return type
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

XMLBehaviorTreeConverter.new().run(SRC, $options[:output])
