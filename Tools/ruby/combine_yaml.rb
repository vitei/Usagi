# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#combine_yaml.rb
#Combines yaml files

require 'optparse'
require 'fileutils'
require 'yaml'

# Useful functions I wish hashes had..
# Copied and pasted from process_hierarchy.rb...
# should probably go in some utility file somewhere
class ::Hash
  def hmap(&block)
    Hash[self.map {|k, v| block.call(k,v) }.reject(&:empty?)]
  end

  # Deep merge found here: http://stackoverflow.com/questions/9381553/ruby-merge-nested-hash
  def deep_merge(second)
    merger = proc { |key, v1, v2| Hash === v1 && Hash === v2   ? v1.merge(v2, &merger) :
		                  Array === v1 && Array === v2 ? v1 | v2 :
				  [:undefined, nil, :nil].include?(v2) ? v1 : v2 }
    self.merge(second, &merger)
  end
end


optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options]"
  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $out = f
  end

  opts.on( '-f file', 'Input file listing files (one per line) to merge' ) do |i|
    $input = i
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

begin
  optparser.parse!
rescue OptionParser::ParseError => e
  abort "combine_yaml: #{e.message}"
end

input_files = ARGV

if $input
  abort "combine_yaml: input list not found: #{$input}" if !File.file?($input)

  input_files = IO.readlines($input).map{|f| f.gsub('\\', '/').chomp }.reject(&:empty?)
end

def load_yaml_file(filename)
  abort "combine_yaml: input file not found: #{filename}" if !File.file?(filename)

  data = YAML.load(File.read(filename))
  abort "combine_yaml: input file must be a YAML mapping: #{filename}" if !data.is_a?(Hash)

  data
rescue Psych::Exception => e
  abort "combine_yaml: invalid YAML in #{filename}: #{e.message}"
rescue SystemCallError => e
  abort "combine_yaml: unable to read #{filename}: #{e.message}"
end

input_yaml  = input_files.map {|f| load_yaml_file(f) }
merged_hash = input_yaml.inject({}) { |merged, input| merged.deep_merge input }

if $out
  output_dir = File.dirname($out)
  FileUtils.mkdir_p(output_dir) if output_dir && output_dir != '.'
end

begin
  out_stream = $out ? File.open($out, 'w') : $stdout
  out_stream.puts merged_hash.to_yaml
  out_stream.close if $out
rescue SystemCallError => e
  abort "combine_yaml: unable to write #{$out}: #{e.message}"
end
