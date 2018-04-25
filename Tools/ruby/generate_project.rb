# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
# generate_project.rb
# Generates a Visual Studio project file

require_relative 'tracker'

require 'digest'
require 'erb'
require 'fileutils'
require 'optparse'
require 'pathname'
require 'rake'
require 'set'
require 'tempfile'

##################
# option parsing #
##################

options = {}

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options]"

  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    options[:output] = f
  end

  opts.on( '-f', 'Fail script when the project file is updated' ) do
    options[:fail_on_update] = true
  end

  opts.on( '-g guid', 'Set GUID for use in the template') do |g|
    options[:guid] = g
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

Tracker::addDepfileOption(options, optparser)
optparser.parse!

if ARGV.length < 3
  STDERR.puts "ERROR! The template file, sources file, and project root must be specified."
  exit
end

if !options.has_key?(:output)
  STDERR.puts "ERROR! You must specify an output file with the -o option!"
  exit
end

##################
# functions      #
##################

def create_file_list(filename)
  sources_list = File.readlines(filename).map{|line| line.chomp}
  sources_list.map!{|s| to_windows_path(s)}
end

def adjust_project_path(list, project_root)
  list.map!{|s| s.gsub(/^#{Regexp.escape(project_root)}\\?/, '')}
end

def list_directories(path)
  list = FileList[File.join(path, "**/*")].exclude do |p|
    !File.directory?(p)
  end

  path_obj = Pathname.new(path)
  list.map{|p| Pathname.new(p).relative_path_from(path_obj).to_path}
end

def to_windows_path(path)
  path.tr('/', '\\')
end

def unique_id(path)
  hex = Digest::MD5.hexdigest(path)
  sprintf("%s-%s-%s-%s-%s", hex[0..7], hex[8..11], hex[12..15], hex[16..19], hex[20..31])
end

def add_to_locations(locations, files, filters)
  files.each do |s|
    dir = File.dirname(s)

    next if !filters.has_key?(dir)

    locations[s] = dir
  end

  locations
end

def to_filter_file(path)
  path.sub(/\.(.+)proj/, '.\1proj.filters')
end

def build_filter_list(project_root)
  if !File.directory?(project_root)
    STDERR.puts "ERROR! Couldn't access the project root path (#{project_root})!"
    exit
  end

  dirs = list_directories(project_root).map{|d| to_windows_path(d)}
  Hash[ dirs.collect{|d| [d, unique_id(d)]} ]
end

def write_dependencies(sources_file, options)
  suffices = ['.headers', '.boilerplate', '.proto']

  suffices.map do |suffix|
    file = sources_file.sub('.sources', suffix)

    next if ! File.exists? file

    Tracker::DepSet.instance.addDep(file)
  end

  Tracker::writeDependenciesFile(options, options[:output])
end

def write_temporary_files(argv, project_file, filters_file, options)
  sources_file = argv[0]
  template_file = argv[1]
  project_root = to_windows_path(argv[2])
  project_dir = "#{project_root}\\"
  project_guid = options.fetch(:guid, '')

  sources = adjust_project_path(create_file_list(sources_file), project_root).sort
  headers_file = sources_file.gsub(/\.sources/, '.headers')
  headers = adjust_project_path(create_file_list(headers_file), project_root).sort

  boilerplate_sources = []
  boilerplate_file = sources_file.gsub(/\.sources/, '.boilerplate')

  if File.exists?(boilerplate_file)
    boilerplate_sources = create_file_list(boilerplate_file).sort
  end

  proto_file = sources_file.gsub(/\.sources/, '.proto')
  proto = create_file_list(proto_file).sort
  proto_relative = proto.map{|s| s.sub(/^#{Regexp.escape(project_root)}\\/, '')}
  proto_base = proto_relative.map{|p| p.sub('.proto', '.pb')}
  proto_sources = proto_base.map{|p| p + ".cpp"}
  proto_headers = proto_base.map{|p| p + ".h"}
  proto_csharp = proto_base.map{|p| p + ".cs"}

  proto_lua_base = proto_relative.map{|p| p.sub('.proto', '.lua')}

  lua_cpp_files = proto_lua_base.map{|p| p + ".cpp"}
  lua_cpp_files.reject! do |f|
    # Skip Lua CPP which contain nothing but the precompiled header include statement
    proto_dir = "_build\\proto\\"
    fn = proto_dir + project_dir + f
    exists = File.exists?(fn)
    size = if exists then File.size(fn) else 0 end
    comp = ('#include "Engine/Common/Common.h"').length
    exists and size <= comp
  end
  proto_sources += lua_cpp_files

  lua_header_files = proto_lua_base.map{|p| p + ".h"}
  lua_header_files.reject! do |f|
    proto_dir = "_includes\\"
    fn = proto_dir + project_dir + f
    exists = File.exists?(fn)
    size = if exists then File.size(fn) else 0 end
    exists and size == 0
  end
  proto_headers += lua_header_files



  project_output = ERB.new(File.read(template_file), nil, '>').result(binding())
  project_file.puts project_output

  # mapping of files to the correct filter
  filters = build_filter_list(argv[2])
  locations = add_to_locations({}, sources, filters)
  locations = add_to_locations(locations, headers, filters)
  locations = add_to_locations(locations, boilerplate_sources, filters)
  locations = add_to_locations(locations, proto_relative, filters)
  locations = add_to_locations(locations, proto_sources, filters)
  locations = add_to_locations(locations, proto_headers, filters)
  locations = add_to_locations(locations, proto_csharp, filters)
  filter_output = ERB.new(File.read(to_filter_file(template_file)), nil, '>').result(binding())
  filters_file.puts filter_output
end

def should_replace_files?(current, temporary)
  if ! File.exists? current
    return true
  end

  temporary.rewind
  temporary_lines = temporary.readlines
  current_lines = File.open(current, 'r') {|f| f.readlines }
  has_identical_lines = current_lines == temporary_lines

  if has_identical_lines
    return false
  end

  current_set = Set.new(current_lines)
  temporary_set = Set.new(temporary_lines)
  local_differences = current_set - temporary_set

  # Check for additions in the current version of the project file.
  # This will miss deletions, but those are likely to be rarer and
  # harder to detect without doing an actual diff.
  if local_differences.length > 0
    STDERR.puts "The following local modifications to #{current} will be overwritten:"

    local_differences.each do |d|
      STDERR.puts d
    end

    return true
  end

  additions = temporary_set - current_set

  if additions.length > 0
    additions.each do |a|
      STDERR.puts a
    end

    return true
  end

  false
end

##################
# main body      #
##################

basename = File.basename(options[:output])
temp_project = Tempfile.new(basename)
temp_filters = Tempfile.new(to_filter_file(basename))

begin
  write_temporary_files(ARGV, temp_project, temp_filters, options)
  write_dependencies(ARGV[0], options)

  if should_replace_files?(options[:output], temp_project)
    # close temporary files before attempting to copy them
    temp_filters.close
    temp_project.close

    FileUtils.mkdir_p(File.dirname(options[:output]))
    FileUtils.cp_r(temp_project.path, options[:output])
    FileUtils.cp_r(temp_filters.path, to_filter_file(options[:output]))

    if options.has_key?(:fail_on_update)
      # When we've overwritten the project file, we deliberately exit
      # with an error code to fail the visual studio build
      abort "Updated project file #{options[:output]}"
    end
  end
ensure
  temp_project.close
  temp_filters.close
end
