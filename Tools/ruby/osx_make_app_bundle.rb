# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#Makes an OS X application bundle from the supplied executable and romfiles
#directory.

require 'optparse'
require 'fileutils'

$options = {}

OptionParser.new do |opts|
	opts.on( '-o', '--output FILE', 'Write app to FILE' ) do |o|
		$options[:output] = o
	end

	opts.on( '-r', '--romfiles DIR', 'Copy romfiles from DIR' ) do |d|
		$options[:romfiles] = d
	end

	opts.on( '-p', '--project NAME', 'Set project name to NAME' ) do |n|
		$options[:project] = n
	end

	opts.on( '-h', '--help', 'Display this screen' ) do
		puts opts
		exit
	end
end.parse!

raise "Please specify an output filename with -o" if !$options[:output]
raise "Please specify project name with -p"       if !$options[:project]

EXECUTABLE=ARGV.shift
EXECUTABLE_DIR="#{File.dirname($options[:output])}"
DYLIB_DIR="#{EXECUTABLE_DIR}/../Libraries/"
DYLIBBUNDLER="Tools/bin/dylibbundler"

FileUtils.mkdir_p(EXECUTABLE_DIR)
FileUtils.cp_r(EXECUTABLE, EXECUTABLE_DIR)

`#{DYLIBBUNDLER} -od -b -x #{EXECUTABLE_DIR}/#{File.basename(EXECUTABLE)} -d #{DYLIB_DIR}`

File.open("#{EXECUTABLE_DIR}/../Info.plist", "w") do |f|
	f.puts <<END_PLIST_OUTPUT
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>BuildMachineOSBuild</key>
	<string>13C64</string>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleExecutable</key>
	<string>#{$options[:project]}_JP</string>
	<key>CFBundleIdentifier</key>
	<string>vitei.#{$options[:project]}_JP</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>#{$options[:project]}_JP</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0</string>
	<key>CFBundleSignature</key>
	<string>????</string>
	<key>CFBundleVersion</key>
	<string>1</string>
	<key>DTCompiler</key>
	<string>com.apple.compilers.llvm.clang.1_0</string>
	<key>DTPlatformBuild</key>
	<string>5B130a</string>
	<key>DTPlatformVersion</key>
	<string>GM</string>
	<key>DTSDKBuild</key>
	<string>13C64</string>
	<key>DTSDKName</key>
	<string>macosx10.9</string>
	<key>DTXcode</key>
	<string>0510</string>
	<key>DTXcodeBuild</key>
	<string>5B130a</string>
	<key>LSMinimumSystemVersion</key>
	<string>10.9</string>
	<key>NSHumanReadableCopyright</key>
	<string>Copyright (c) 2014 Vitei. All rights reserved.</string>
	<key>NSMainNibFile</key>
	<string>MainMenu</string>
	<key>NSPrincipalClass</key>
	<string>NSApplication</string>
</dict>
</plist>
END_PLIST_OUTPUT
end

if $options[:romfiles]
  ROMFILES_DIR="#{EXECUTABLE_DIR}/../Resources"
  SRC_FILES = Dir.glob("#{$options[:romfiles]}/**/*")
  SRC_FILES.each do |src|
    if not File.directory? src
      out = "#{ROMFILES_DIR}#{src.sub($options[:romfiles], "")}"
      FileUtils.mkdir_p(File.dirname(out))
      FileUtils.cp(src, out)
    end
  end
end
