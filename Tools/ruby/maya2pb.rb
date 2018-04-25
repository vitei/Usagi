# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
# convert_instance.rb
# Convert a YAML file with instance data to a protocol buffer file

require 'optparse'
require 'yaml'
require 'zlib'

require_relative 'tracker'

require 'Engine/Maths/Maths.pb.rb'
require 'Engine/Physics/CollisionData.pb.rb'
require 'Engine/Scene/Model/InstanceSet.pb.rb'

##################
# option parsing #
##################

options = {}

option_parser = OptionParser.new do |opts|
  opts.banner = "Usage: read_instance.rb -o output_file input_file"
  opts.on('-v', '--verbose', 'Verbose output') { options[:verbose] = true }

  opts.on('--MF file', 'Write dependencies file for Ninja') do |f|
    options[:depfile] = f
  end

  opts.on('-o FILE', '--outfile FILE', 'Output filename') do |f|
    options[:output] = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

Tracker::addDepfileOption(options, option_parser)

option_parser.parse!

Tracker::writeDependenciesFile(options, options[:output]) if options[:output]

if ARGV.length != 1
  raise "ERROR: No YAML input file specified!"
end

##################
# functions      #
##################

def create_vector(node)
  return Usg::Vector3f.new(x: node[0], y: node[1], z: node[2])
end

def process_as_instances(data, options)
  group_messages = []

  data.each do |group|
    node_messages = []

    group['Nodes'].each do |n|
      message = Usg::Model::Instance.new(center: create_vector(n['translation']),
                                         rotation: create_vector(n['rotation']),
                                         scale: create_vector(n['scale']),
                                         name: group['name'])
      node_messages << message
    end

    sphere = Usg::Components::Sphere.new(centre: Usg::Vector3f.new(x: 0.0, y: 0.0, z: 0.0), radius: 0.0)
    instance_set = Usg::Model::InstanceSet.new(modelName: group['ModelName'],
                                               boundingSphere: sphere,
                                               instances: node_messages)
    group_messages << instance_set
  end

  FileWriter::write(Usg::Model::InstanceHeader.new(instanceSets: group_messages.length),
                    group_messages, options)
end

def process_as_points(data, options)
  messages = []

  data.each do |group|
    group['Nodes'].each do |n|
      defaultType = LocationType::NONE

      if n.has_key?('locationType')
        type = n['locationType'].to_sym
        defaultType = LocationType.const_get(type) if LocationType.constants.include?(type)
      end

      messages << Components::LocationComponent.new(translation: create_vector(n['translation']),
                                                    rotation: create_vector(n['rotation']),
                                                    scale: create_vector(n['scale']),
                                                    uNameHash: Zlib.crc32(n['name']),
                                                    uUserData: n['userdata'],
                                                    type: defaultType)
    end
  end

  FileWriter::write(LocationHeader.new(locationCount:messages.length), messages, options)
end

##################
# classes        #
##################

class FileWriter
  MESSAGE_DELIMITER = "\0"

  def self.write(header, sets, options = {})
    out = options[:output] ? File.open(options[:output], "wb") : STDOUT
    header.serialize(out) << MESSAGE_DELIMITER
    sets.each { |s| s.serialize(out) << MESSAGE_DELIMITER }
    out.close if options[:output]
  end
end

##################
# main body      #
##################

YAML_FILE = ARGV[0]
data = YAML.load(File.read(YAML_FILE))
raise "Root node should be an array" if not data.is_a? Array

if data[0]['Instance'] == true
  process_as_instances(data, options)
else
  process_as_points(data, options)
end
