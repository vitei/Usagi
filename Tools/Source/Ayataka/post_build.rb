
require "fileutils"

USAGI_DIR=ENV["USAGI_DIR"]
EXE_PATH=ARGV[0]
DLL_PATH=ARGV[1]
FileUtils.cp( EXE_PATH, "#{USAGI_DIR}/Tools/bin/Ayataka.exe")
FileUtils.cp( DLL_PATH, "#{USAGI_DIR}/Tools/bin/libfbxsdk.dll")
