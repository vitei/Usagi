# Usagi Engine, Copyright (c) Vitei, Inc. 2013
#!ruby
# deflist.rb
# Create a list mapping protobuf messages and enums to the files where
# they are defined.

require 'optparse'
require 'rake'

options = {:additions => []}

option_parser = OptionParser.new do |opts|
  opts.on('-a line', '--add line', 'Add line (can be done multiple times)') do |line|
    options[:additions] << line
  end

  opts.on('-f file', '--file file', 'Copy content of the specified file') do |file|
    options[:additional_file] = file
  end

  opts.on('-s dir', '--submodule dir', 'Submodule dir name') do |dir|
    options[:submodule_prefix] = dir
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

option_parser.parse!

raise "ERROR: No directories specified!" if ARGV.length < 1

DIRS = ARGV

DIRS.each do |d|
  raise "ERROR! Couldn't find directory '#{d}'" if ! File.directory? d
end

package_regexp = /^\s*package ([\w\.]+)\s*;/u
message_regexp = /^\s*(?:message|enum) ([A-Za-z]\w+)/u

protocol_definitions = FileList[DIRS.map{|d| "#{d}/**/*.proto"}]
protocol_definitions.exclude("**/Engine/ThirdParty/nanopb/**/*")
protocol_definitions.exclude("**/Engine/Core/usagipb.proto")

protocol_definitions.each do |f|
  current_file = File.open(f, 'r:UTF-8')
  content = current_file.read

  package_match = package_regexp.match(content)
  package = ""

  if package_match != nil
    package = package_match[1]
    clean_package = package.sub(/^[a-z]/) {|c| c.capitalize}
    clean_package.gsub!(/\.(\w)/) {"::#{$1.capitalize}"}
    package = clean_package + '::'
  end

  content.scan(message_regexp) do |match|
    puts "#{package}#{match[0]}|#{f}"
  end
end

if options.has_key?(:additional_file)
  lines = File.readlines(options[:additional_file])

  if options.has_key?(:submodule_prefix)
    lines = lines.map{|line| line.sub(/\|/, "|#{options[:submodule_prefix]}/") }
  end

  lines.each{|line| puts line}
end

options[:additions].each do |line|
  puts line
end
