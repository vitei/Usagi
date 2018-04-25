# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#combine_yaml.rb
#Combines yaml files

require 'optparse'
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

optparser.parse!

input_files = ARGV

if $input
  input_files = IO.readlines($input).map{|f| f.gsub('\\', '/').chomp }
end

input_yaml  = input_files.map {|f| data = YAML.load(File.read(f)) }
merged_hash = input_yaml.inject({}) { |merged, input| merged.deep_merge input }

out_stream = $out ? File.open($out, 'w') : $stdout
out_stream.puts merged_hash.to_yaml
