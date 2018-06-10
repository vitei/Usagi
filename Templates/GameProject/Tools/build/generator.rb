# encoding: UTF-8
require 'set'
require 'pathname'
require 'yaml'

SUBMODULE_DIR = File.join('../..', 'Usagi')
require_relative File.join(SUBMODULE_DIR, 'Tools/build/generator_util.rb')
require_relative File.join(SUBMODULE_DIR, 'Tools/build/platform.win.rb')
require_relative File.join(SUBMODULE_DIR, 'Tools/build/game_generator.rb')


NINJA_FILE = ARGV[5]
$engine = BuildConfig.new(ENV['USAGI_DIR'], ARGV)

case $engine.target_platform
#FIXME: Add other platforms
when 'win'
  $platform = PlatformWin.new
end

n = Ninja::File.new()

#####################################################################
# Housekeeping                                                      #
#####################################################################

clean_protocol_ruby_classes($engine, false)

#####################################################################
# Variables                                                         #
#####################################################################
GeneratorUtil.define_variables($engine, $platform, n)
GeneratorUtil.define_ruby_variables($engine, $platform, n, $engine.protocol_ruby_output_dir(false))
GeneratorUtil.define_protoc_includes_variable(n, 'Usagi')

#####################################################################
# Rules                                                             #
#####################################################################
GeneratorUtil.define_base_rules($engine, $platform, n)
GeneratorUtil.define_system_rules($engine, $platform, n)
GeneratorUtil.define_data_rules($engine, $platform, n)

case $engine.target_platform
when 'win'
  GeneratorUtil.define_pc_rules($engine, $platform, n)
end

#####################################################################
# Build Statements                                                  #
#####################################################################
GeneratorUtil.create_build_id($engine, n, $engine.build_id_header, $engine.project)
protocol_headers = $engine.protocol_headers(false)
generated_headers = protocol_headers + [$engine.build_id_header]
includes = project_includes($engine, $platform, n)
generated_headers.push(*includes)

win_bp_cpp = []
boilerplate_obj, bp_generated_headers = boilerplate($engine, n, generated_headers, win_bp_cpp)

project_obj = project_code($engine, $platform, n, generated_headers + bp_generated_headers)
project_exe($engine, $platform, n, project_obj + boilerplate_obj)
data_deps = process_data($engine, $platform, n)
# Additional project specific data dependencies would be added here

SHOULD_BUILD_PROJECTS = $engine.target_platform == 'win'

if SHOULD_BUILD_PROJECTS
  projects($engine, n, generated_headers + bp_generated_headers, win_bp_cpp, data_deps)
end

puts NINJA_FILE
n.save(NINJA_FILE)
