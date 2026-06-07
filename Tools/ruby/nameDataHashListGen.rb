# Usagi Engine, Copyright © Vitei, Inc. 2013
require 'optparse'
require 'zlib'
require 'digest/sha1'
require 'fileutils'

MESSAGE_DELIMITER = "\0"

def abort_with_error(message)
  $stderr.puts "ERROR: #{message}"
  exit 1
end

def validate_readable_file(path, description)
  abort_with_error("#{description} not specified") if path.nil? || path.empty?
  abort_with_error("#{description} does not exist: #{path}") unless File.exist?(path)
  abort_with_error("#{description} is not a file: #{path}") unless File.file?(path)
  abort_with_error("#{description} is not readable: #{path}") unless File.readable?(path)
end

def validate_directory(path, description)
  abort_with_error("#{description} not specified") if path.nil? || path.empty?
  abort_with_error("#{description} does not exist: #{path}") unless File.exist?(path)
  abort_with_error("#{description} is not a directory: #{path}") unless File.directory?(path)
end

def create_output_parent(output)
  abort_with_error("Output filename not specified") if output.nil? || output.empty?

  parent = File.dirname(output)
  return if parent.nil? || parent.empty? || parent == '.'

  if File.exist?(parent)
    abort_with_error("Output parent is not a directory: #{parent}") unless File.directory?(parent)
    return
  end

  FileUtils.mkdir_p(parent)
rescue SystemCallError => e
  abort_with_error("Could not create output directory #{parent}: #{e.message}")
end

def validate_listed_data_files(input, dir)
  current_path = input
  File.foreach(input) do |line|
    file_name = line.chomp
    current_path = File.join(dir, file_name)
    validate_readable_file(current_path, "Listed data file")
    File.open(current_path, 'rb') {}
  end
rescue SystemCallError => e
  abort_with_error("Could not read file #{current_path}: #{e.message}")
end

def require_protobufs(require_dirs)
  require_dirs.each do |dir|
    $LOAD_PATH << dir
    Dir[File.join(dir, '**', '*.pb.rb')].each do |f|
      require f.sub(%r{\A#{Regexp.escape(dir)}[\\/]}, '')
    end
  end
rescue LoadError => e
  abort_with_error("Could not require protobuf file: #{e.message}")
end

##################
# option parsing #
##################

options = {
  require_dirs: []
}

option_parser = OptionParser.new do |opts|
  opts.banner = "Usage: "
  opts.on('-v', '--verbose', 'Verbose output') { options[:verbose] = true }

  opts.on('-o FILE', '--outfile FILE', 'Output filename') do |f|
    options[:output] = f
  end

  opts.on('-d DIR', '--dir DIR', 'Root directory') do |f|
    options[:dir] = f
  end

  opts.on( '-Rdir', 'Require all *.pb.rb files in a directory' ) do |r|
    options[:require_dirs] << r
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

begin
  option_parser.parse!
rescue OptionParser::ParseError => e
  abort_with_error(e.message)
end

abort_with_error("Expected exactly one input list file") if ARGV.length != 1

input = ARGV[0]

abort_with_error("Output filename not specified") if options[:output].nil? || options[:output].empty?
abort_with_error("Root directory not specified") if options[:dir].nil? || options[:dir].empty?

validate_directory(options[:dir], "Root directory")
options[:require_dirs].each do |dir|
  validate_directory(dir, "Require directory")
end

validate_readable_file(input, "Input list")
validate_listed_data_files(input, options[:dir])
create_output_parent(options[:output])

# Require PB codes after all require directories have been validated.
require_protobufs(options[:require_dirs])

def main(output, input, dir)
  list = []

  File.open( input, 'r' ) do |f|
    f.each {|line|
      fileName = line.chomp
      filePath = "#{dir}/#{fileName}"
      crc = Zlib.crc32(fileName, 0)
      # dataHash = Digest::SHA1.hexdigest(File.open(filePath, "rb").read)
      dataHash = Zlib.crc32(File.open(filePath, "rb").read, 0)
      # print "#{filePath} #{crc} #{dataHash}\n"

      temp = {}
      temp[:name] = fileName
      temp[:crc] = crc
      temp[:dataHash] = dataHash

      list.push( temp )
    }
  end

  list.sort! {|a, b|
    a[:crc] <=> b[:crc]
  }

  messages = []

  list.each {|elem|
    # print "#{elem[:crc]} #{elem[:name]} #{elem[:dataHash]}\n"
    hash = Usg::NameDataHash.new( nameCRC: elem[:crc], dataHash: elem[:dataHash])
    messages << hash
  }

  header = Usg::NameDataHashHeader.new( hashNum: list.length )

  begin
    out = File.open(output, "wb")
    header.serialize(out) << MESSAGE_DELIMITER
    messages.each { |s| s.serialize(out) << MESSAGE_DELIMITER }
    out.close
  rescue SystemCallError => e
    out.close if out && !out.closed?
    abort_with_error("Could not write output file #{output}: #{e.message}")
  end
end

main(options[:output], input, options[:dir])
