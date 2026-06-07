# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
# convert_instance.rb
# Convert a YAML file with instance data to a protocol buffer file

require 'optparse'
require 'fileutils'
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

def fail_with_message(message, option_parser = nil)
  warn message
  warn option_parser if option_parser
  exit 1
end

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

begin
  option_parser.parse!
rescue OptionParser::ParseError => e
  fail_with_message("ERROR: #{e.message}", option_parser)
end

if ARGV.length != 1
  fail_with_message("ERROR: Expected exactly one YAML input file, got #{ARGV.length}.", option_parser)
end

##################
# functions      #
##################

def ensure_output_directory(filename)
  output_dir = File.dirname(filename)
  return if output_dir.nil? || output_dir == '.'

  FileUtils.mkdir_p(output_dir)
rescue SystemCallError => e
  fail_with_message("ERROR: Could not create output directory '#{output_dir}' for '#{filename}': #{e.message}")
end

def load_yaml_file(filename)
  YAML.load(File.read(filename))
rescue Psych::Exception => e
  fail_with_message("ERROR: Could not parse YAML file '#{filename}': #{e.message}")
rescue SystemCallError => e
  fail_with_message("ERROR: Could not read YAML file '#{filename}': #{e.message}")
end

def validate_root_node(data, filename)
  if !data.is_a?(Array)
    fail_with_message("ERROR: Root node in '#{filename}' should be an array.")
  end

  if data.empty? || data.any? { |group| !group.is_a?(Hash) }
    fail_with_message("ERROR: Root node in '#{filename}' should be a non-empty array of mappings.")
  end
end

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
    if options[:output]
      ensure_output_directory(options[:output])
      out = File.open(options[:output], "wb")
    else
      out = STDOUT
    end

    header.serialize(out) << MESSAGE_DELIMITER
    sets.each { |s| s.serialize(out) << MESSAGE_DELIMITER }
    out.close if options[:output]
  rescue SystemCallError => e
    fail_with_message("ERROR: Could not write output file '#{options[:output]}': #{e.message}")
  end
end

##################
# main body      #
##################

YAML_FILE = ARGV[0]
data = load_yaml_file(YAML_FILE)
validate_root_node(data, YAML_FILE)

if data[0]['Instance'] == true
  process_as_instances(data, options)
else
  process_as_points(data, options)
end

Tracker::writeDependenciesFile(options, options[:output]) if options[:output]
