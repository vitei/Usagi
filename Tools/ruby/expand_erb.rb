# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#expand_yml.rb
#Expands a yml file containing embedded Ruby code into plain yml

require 'optparse'

require 'set'
require 'erb'
require 'zlib'
require 'fileutils'

$options = {
  require_dirs: []
}

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options] infile"
  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $options[:output] = f
  end

  opts.on( '-Rdir', 'Require all *.pb.rb files in a directory' ) do |r|
    $options[:require_dirs] << r
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

begin
  optparser.parse!
rescue OptionParser::ParseError => e
  abort "expand_erb: #{e.message}"
end

abort "expand_erb: expected exactly one input file" if ARGV.length != 1

SRC = ARGV[0]

abort "expand_erb: input file not found: #{SRC}" if !File.file?(SRC)

$options[:require_dirs].each do |dir|
  abort "expand_erb: require directory not found: #{dir}" if !File.directory?(dir)
end

$options[:require_dirs].each do |dir|
  $LOAD_PATH << dir
  Dir[dir + "/**/*.pb.rb"].each do |f|
    require f.sub(dir + "/", "")
  end
end

begin
  content = File.open(SRC, 'r') { |f| ERB.new(f.read).result }

  if $options[:output]
    output_dir = File.dirname($options[:output])
    FileUtils.mkdir_p(output_dir) if output_dir && output_dir != '.'
  end

  out_stream = $options[:output] ? File.open($options[:output], "w") : $stdout
  out_stream.print content
  out_stream.close if $options[:output]
rescue StandardError => e
  abort "expand_erb: #{SRC}: #{e.message}"
end
