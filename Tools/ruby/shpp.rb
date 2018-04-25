# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#shpp.rb
#Shader preprocessor -- embeds #include files and allows you to add #defines
#Also spits out a dependencies file

require 'optparse'
require 'set'

$options = {
  includedirs: [],
  defines:     {},
}

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options] infile"
  opts.on( '-Idir', 'Add directory to include path' ) do |i|
    $options[:includedirs] << i
  end

  opts.on( '-Dmacro[=defn]', 'Predefine macro as defn.  If no definition is passed, define as 1' ) do |d|
    v, x = d.split("=")
    $options[:defines].merge!({v => x ? x : "1"})
  end

  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $options[:output] = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

optparser.parse!

if ARGV.length != 1
  puts optparser
  exit
end

if !$options[:output]
  puts "Please specify an output file for the conversion"
  puts optparser
  exit
end

$input_file      = ARGV[0]
$version         = "#version 330"
$output_contents = ""
$included_files  = Set.new

$options[:includedirs].unshift(File.dirname($input_file))

def get_includepath_from_line(l)
  localPath = l.sub(/.*#include /, "").gsub(/"/, "").strip
  $options[:includedirs].each do |dir|
    f = "#{dir}/#{localPath}"
    return f if File.exists? f
  end

  raise "Include file not found: #{localPath}"
end

def parse_include(f)
  if !$included_files.include? f
    $included_files << f
    File.open(f, 'r').each_line do |l|
      if l.match(/#include/)
        includeFile = get_includepath_from_line l
        parse_include includeFile
      else
        $output_contents += l
      end
    end
  end
end

File.open($input_file, 'r').each_line do |l|
  if l.match(/^#version/)
    $version = l
  elsif l.match(/#include/)
    includeFile = get_includepath_from_line l
    parse_include includeFile
  else
    $output_contents += l
  end
end

File.open($options[:output], 'w') do |f|
  f.puts $version
  $options[:defines].each{ |k, v| f.puts "#define #{k} #{v}" }
  f.puts $output_contents
end
