# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#release.rb
# Writes a .zip file containing a single usagi redistributable release.

require 'rubygems'
require 'zip'
require 'pathname'
require 'fileutils'

raise "Usage: release.rb output.zip platform_name" if ARGV.length < 2

OUTPUT_ZIP = ARGV[0]
PLATFORM   = ARGV[1]
REDISTRIBUTABLES = { "_libs/ctr" => "_libs",
                     "_includes" => "_includes",
                     "_tools"    => "_tools"}

Zip.setup do |c|
  c.on_exists_proc          = true
  c.continue_on_exists_proc = true
  c.unicode_names           = true
end

FileUtils.mkdir_p(File.dirname OUTPUT_ZIP)

Zip::File.open(OUTPUT_ZIP,Zip::File::CREATE) do |zipfile|
  REDISTRIBUTABLES.each do |src, dst|
    if File.directory?(src)
      Dir[File.join(src, '**', '**')].each do |file|                
        zipfile.add(file.sub(/^#{src}/,dst),file)
      end
    else
      zipfile.add(dst,src)
    end
  end
end
