# Usagi Engine, Copyright © Vitei, Inc. 2013
require 'optparse'
require 'zlib'
require 'digest/sha1'

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

option_parser.parse!

if ARGV.length != 1
  raise "ERROR: No YAML input file specified!"
end

# Require PB codes
options[:require_dirs].each do |dir|
  $LOAD_PATH << dir
  Dir[dir + "/**/*.pb.rb"].each do |f|
    require f.sub(dir + "/", "")
  end
end

MESSAGE_DELIMITER = "\0"

def main(output, input, dir)
  list = []

  f = File.open( input, 'r' )
  f.each {|line|
    fileName = "#{line[0..-2]}"
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

  out = File.open(output, "wb")
  header.serialize(out) << MESSAGE_DELIMITER
  messages.each { |s| s.serialize(out) << MESSAGE_DELIMITER }
  out.close
end

main(options[:output], ARGV[0], options[:dir])