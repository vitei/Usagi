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

optparser.parse!
raise "ERROR: No input file specified!" if ARGV.length != 1

$options[:require_dirs].each do |dir|
  $LOAD_PATH << dir
  Dir[dir + "/**/*.pb.rb"].each do |f|
    require f.sub(dir + "/", "")
  end
end

SRC = ARGV[0]

content = File.open(SRC, 'r') { |f| ERB.new(f.read).result }

FileUtils.mkdir_p(File.dirname($options[:output])) if $options[:output]
out_stream = $options[:output] ? File.open($options[:output], "w") : $stdout
out_stream.print content

if $options[:output]
  out_stream.close
end
