# Usagi Engine, Copyright © Vitei, Inc. 2013
#!/usr/bin/ruby
#ccppc-wrapper.rb
#Wraps the Cafe compiler, moving the dependency into the right place after
#compilation is complete.

require 'fileutils'

CMDLINE = ARGV.join(" ")
DEP_OUT = CMDLINE.sub(/^.*-MF ([^ ]*).*$/, '\1')
DEP_IN = DEP_OUT.sub(/\.o\.d$/, ".d")

RTN = system(CMDLINE.sub(/-MF [^ ]*/, ''))

FileUtils.mv(DEP_IN, DEP_OUT) if RTN

$?.exitstatus
