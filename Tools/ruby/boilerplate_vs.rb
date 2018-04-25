# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
# boilerplate_vs.rb
# Wrapper script for calling the boilerplate tool from Visual Studio builds

require 'open3'
require 'optparse'
require 'pathname'

$options = {}

optparser = OptionParser.new do |opts|
  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $options[:output] = f
  end

  opts.on( '-d file', 'Specifies the filename to put dependency information in' ) do |f|
    $options[:depfile] = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

optparser.parse!

raise "Error! Output file not specified!" if ! $options.has_key?(:output)

usagi_dir = Pathname.new(ENV['USAGI_DIR'])
BOILERPLATE_TOOL = "#{usagi_dir}/Tools/bin/systems-scanner.exe"
input_path = Pathname.new(ARGV[0])
input_file = input_path.relative_path_from(usagi_dir).to_path

BOILERPLATEFLAGS = "-I. -ITank -IEngine/ThirdParty/nanopb -I_includes " +
	           "-IEngine/ThirdParty/pthread-w32/include " +
	           "-I\"#{ENV["VS120COMNTOOLS"]}../../VC/include\" " +
	           "-I\"C:/Program Files (x86)/Windows Kits/8.1/Include/um\" " +
	           "-I\"C:/Program Files (x86)/Windows Kits/8.1/Include/shared\" " +
	           "-D_M_IA64 -DPLATFORM_PC -Dpid_t=int"

deps = $options[:depfile] ? "-d #{$options[:depfile]}" : ""
stdin, stdout, stderr = Open3.popen3("#{BOILERPLATE_TOOL} #{BOILERPLATEFLAGS} #{deps} -o #{$options[:output]} #{input_file}")

boilerplate_error = stderr.readlines[0]

if boilerplate_error != nil
  print "#{BOILERPLATE_TOOL} #{BOILERPLATEFLAGS} -H -o #{$options[:output]} #{input_file}"
  raise "Error! Boilerplate error: #{boilerplate_error}"
end
