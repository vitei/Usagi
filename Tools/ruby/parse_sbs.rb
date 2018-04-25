# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
# parse_sbs.rb
# Parse Allegorithmic's .sbs files and generate a list of outputs.

require 'optparse'
require 'rexml/document'
require 'fileutils'

$options = {}

option_parser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options] infile"
  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $options[:output] = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

option_parser.parse!
raise "ERROR! No input file specified!" if ARGV.length == 0
$options[:input] = ARGV[0]
$options[:basename] = File.basename(ARGV[0]).sub('.sbs', '')

raise "ERROR! Input file doesn't exist!" if !File.exist?($options[:input])

generate = true
if File.size?($options[:output]) != nil then
  generate = (File.mtime($options[:output]) < File.mtime($options[:input]))
end
exit if !generate

# array of graphs
graphs = Array.new

# load xml file
inSBS = REXML::Document.new(File.new($options[:input], 'r'))
contentRoot = inSBS.elements["package/content"]
contentRoot.elements.each("graph") do |g|
  
  graph = Hash.new

  graph["url"] = g.elements["identifier/@v"].to_s  
  graph["graph"] = graph["url"]

  graph["outputs"] = Array.new
  g.elements.each("graphOutputs/graphoutput/identifier") do |i|
    graph["outputs"].push(i.attributes['v'].to_s)
  end

  graphs.push(graph)

end
#print "\n"
#print graphs

lines = ""
graphs.each do |g|

  # if there's only one output, use filename:
  # sbsarFilename_graphName
  if g["outputs"].count == 1 then
    o = g["outputs"][0]
    lines << "#{g['url']}\t#{o}\t#{$options[:basename]}_#{g['graph']}\n"

  # if there's multiple outputs, use filename:
  # sbsarFilename_graphName_outputName
  else
    g["outputs"].each do |o|
      lines << "#{g['url']}\t#{o}\t#{$options[:basename]}_#{g['graph']}_#{o}\n"
    end
  end

end

#print lines

# ensure output dir exists
outdir = File.dirname($options[:output])
FileUtils.mkdir_p outdir

# dump metadata to file
out = File.open($options[:output], 'w')
out.write(lines)
out.close
