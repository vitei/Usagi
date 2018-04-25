# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#boilerplate.rb
#Uses the boilerplate tool to scan headers for Systems information.

require 'erb'
require 'zlib'
require 'yaml'
require 'open3'
require 'tempfile'
require 'pathname'

require 'optparse'

$submodule_dir = nil
$project_dir = nil

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options]"
  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $out = f
  end

  opts.on( '-d file', 'Specifies the filename to put dependency information in' ) do |f|
    $depfile = f
  end

  opts.on( '-s dir', 'Specifies submodule directory prefix' ) do |s|
    $submodule_dir = s
  end

  opts.on( '-p dir', 'Specifies project directory' ) do |p|
    $project_dir = p
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

optparser.parse!

ALL_HEADERS = ARGV
INCLUDES = ALL_HEADERS.map { |h| "#include \"#{h}\"" }
includes = Tempfile.new(["systems", ".h"])

begin
  includes.write(INCLUDES.join("\n"))

  if Gem.win_platform?
    boilerplate_tool = "Tools\\bin\\systems-scanner.exe"

    boilerplateflags = ""
    boilerplateflags = "-I#{$project_dir} " + boilerplateflags if !$project_dir.nil?
    include_dirs = [".", "Engine/ThirdParty/nanopb", "_includes", "Engine/ThirdParty/pthread-w32/include"]

    if !$submodule_dir.nil?
      boilerplate_tool = "#{$submodule_dir}\\" + boilerplate_tool
      boilerplateflags = "-I. -I_includes " + boilerplateflags + "-I#{$submodule_dir} "
      boilerplateflags += include_dirs.select{|d| d != "."}.map{|d| "-I#{$submodule_dir}/#{d} "}.join("")
    else
      boilerplateflags += include_dirs.map{|d| "-I#{d} "}.join("")
    end

    boilerplateflags = boilerplateflags +
                       "-I\"#{ENV["VS120COMNTOOLS"]}../../VC/include\" " +
                       "-I\"C:/Program Files (x86)/Windows Kits/8.1/Include/um\" " +
                       "-I\"C:/Program Files (x86)/Windows Kits/8.1/Include/shared\" " +
                       "-D_M_IA64 -DPLATFORM_PC -Dpid_t=int"
  else
    boilerplate_tool = "Tools/bin/systems-scanner"
    boilerplateflags = "-I. -ITank -IEngine/ThirdParty/nanopb -I_includes -Iusr/local/opt/llvm/include -D_M_IA64 -DPLATFORM_OSX"
  end
  includes.close

  deps = $depfile ? "-d#{$depfile}" : ""
  stdin, stdout, stderr = Open3.popen3("#{boilerplate_tool} #{boilerplateflags} #{deps} -o #{$out} \"#{includes.path}\"")

  boilerplate_error = stderr.readlines[0]

  if boilerplate_error != nil
    print "#{boilerplate_tool} #{boilerplateflags} -H -o #{$out} #{input_file}"
    raise "Error! Boilerplate error: #{boilerplate_error}"
  end
ensure
  includes.close
  includes.unlink
end
