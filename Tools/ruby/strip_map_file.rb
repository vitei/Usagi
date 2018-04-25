#!ruby
# Strips out unnecessary lines from the CTR mapfile

require 'optparse'

OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options] infile"

  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $out = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end.parse!

OUT = $out ? File.open($out, 'w') : STDOUT
HEADERS = "Base Addr    Size         Type   Attr      Idx    E Section Name        Object"
regex = Regexp.new(/(0x\h+)\s+(0x\h+)\s+(\w+)\s+(\w+)\s+(\d+)\s+(\S+)\s+(\S+)/)

ARGF.each_line do |l|
  stripped = l.strip
  next if stripped.empty?

  is_headers = stripped == HEADERS
  if !$scanning_on
    $scanning_on |= is_headers
  else
    match = regex.match(l)
    next if is_headers || match.nil?

    address = match[1]
    size = match[2].to_i(16)
    area = match[3]
    permissions = match[4]
    id = match[5]
    function = match[6]
    file = match[7]

    next if area != 'Code'

    function_name = ''

    if function.start_with?('i._ZN')
      index = 5
      f = function[index..-1]
      start_index = 0

      while true
        length = f.to_i(10)

        break if length == 0

        if length > 9
          index += 2
        else
          index += 1
        end

        if !function_name.empty?
          function_name += '::'
        end

        function_name += function[index, length]
        index += length
        f = function[index..-1]
      end
    elsif function.start_with?('i._Z')
      index = 4
      f = function[index..-1]
      length = f.to_i(10)

      if length > 9
        index += 2
      else
        index += 1
      end

      function_name += function[index, length]
    elsif function.start_with?('i.')
      function_name = function[2..-1]
    end

    next if function_name.empty?

    OUT.puts sprintf("0x%08x %d %s %s\n", address, size, function_name, file)
  end
end

OUT.close if $out
