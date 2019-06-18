# Usagi Engine, Copyright (c) Vitei, Inc. 2013
require_relative 'generator_util.rb'
require_relative 'platform.win.rb'

NINJA_FILE = ARGV[5]
$engine = BuildConfig.new(ENV['USAGI_DIR'], ARGV)

case $engine.target_platform
when 'win'
  $platform = PlatformWin.new
end

n = Ninja::File.new()

#####################################################################
# Engine-Specific Functions                                         #
#####################################################################
def build_corepbs_dll(config, n)
  coreproto_files = FileList['Engine/Core/usagipb.proto', 'Engine/ThirdParty/nanopb/generator/proto/nanopb.proto']
  write_file_list(coreproto_files, config.csharp_engine_root, "", '.coreproto.txt')
  coreproto_pbcs = GeneratorUtil.csharp_protobuf(config, n, coreproto_files)
  pbcs_extension_project = pbcs_extension_dll(config, n)
    
  # get path to dll
  dllpath = GeneratorUtil.get_msbuild_targetpath(pbcs_extension_project[0],
      "Debug", "CorePBs")
    
  # determine whether dll needs rebuilding
  rebuildDLL = false

  if File.exist?(dllpath)
    dlltime = File.mtime(dllpath)

    # dll exists, do a mtime comparison with coreproto_files
    coreproto_files.each do |coreproto_prereq|
      ltime = File.mtime(coreproto_prereq)
      rebuildDLL = File.mtime(coreproto_prereq) > File.mtime(dllpath)
      break if rebuildDLL
    end
  else
    # dll doesn't exist, build it
    rebuildDLL = true
  end

  # rebuild dll if necessary
  if rebuildDLL
    dllpath = GeneratorUtil.invoke_msbuild(
                                           config,
                                           n,
                                           pbcs_extension_project[0],
                                           coreproto_pbcs,
                                           "Debug",
                                           "CorePBs"
                                           )
  end

  n.alias('corepbs_dll', dllpath)
  n.defaults('corepbs_dll')

  dllpath
end

def libs(config, platform, n, bp_objects, order_only_deps)
  pbs = []
  pbs_csharp = []
  # We only build C# files for our Windows build, so we shouldn't build CorePBs on others.
  dllpath = config.target_platform == 'win' ? build_corepbs_dll(config, n) : nil

  # TODO: If we ever want a command-line NX build, we'll have to check
  # to see VISUAL_STUDIO_BUILD is defined in the environment when the
  # platform is 'nx'
  should_skip_compilation = config.target_platform == 'win' ||
    config.target_platform == 'nxe' || config.target_platform == 'nx'

  libs = config.libs().map do |lib|
    source_root = "Engine/#{lib}"
    pch_file = nil
    pch_source_file = config.pch_source_file(source_root)

#    if config.target_platform == 'ctr' && File.exist?(pch_source_file)
#      pch_file = config.pch_file(source_root, lib)
#      n.build('pch', {pch_file => pch_source_file})
#      order_only_deps << pch_file
#    end

    sources = get_sourcelist(source_root, config.target_platform,
                             platform.underscore_dirs_whitelist)
    platform_sources = get_platform_sourcelist(source_root, config.target_platform, "Engine/",
                             platform.underscore_dirs_whitelist)

    targets = gen_source_targets(sources, config, platform)
    standard_targets = targets.reject{|c, o| c.end_with?('.pb.cpp') || c.end_with?('.lua.cpp')}
    write_file_list(standard_targets.keys, config.code_working_dir, source_root)

    platform_targets = gen_source_targets(platform_sources, config, platform)
    write_file_list(platform_targets.keys, config.code_working_dir, source_root, ".platform.txt")

    proto_sources = get_sourcelist(source_root, config.target_platform)
    proto_files = proto_sources.select{|s| s.end_with?('.proto')}
    write_file_list(proto_files, config.code_working_dir, source_root, '.proto.txt')

    pb_cpp = GeneratorUtil.cpp_protobuf(config, n, proto_files)
    pbs.concat pb_cpp
    pb_lua = GeneratorUtil.lua_protobuf(config, n, proto_files)
    pbs.concat pb_lua
    pb_rb = GeneratorUtil.ruby_protobuf(config, n, proto_files)
    pbs.concat pb_rb
    pb_py = GeneratorUtil.python_protobuf(config, n, proto_files)
    pbs.concat pb_py
    pb_csharp = GeneratorUtil.csharp_protobuf(config, n, proto_files, dllpath)
    pbs_csharp.concat(pb_csharp);

    next if should_skip_compilation

    GeneratorUtil.compile_source(config, n, standard_targets, order_only_deps, pch_file)
    proto_targets = targets.select{|c, o| c.end_with?('.pb.cpp') || c.end_with?('.lua.cpp')}
    GeneratorUtil.compile_source(config, n, proto_targets, order_only_deps, pch_file)

    libOut = config.get_lib_filename lib
    FileUtils.mkdir_p(File.dirname libOut)
    lib_bp_obj = bp_objects.select{|o| o.start_with? "#{config.code_working_dir}/#{source_root}"}
    n.build('ar', {libOut => targets.values + lib_bp_obj})

    libOut
  end
  #if config.target_platform == 'win'
  n.alias('pbs_csharp', pbs_csharp)    
  pbs.concat(pbs_csharp)
  #end

  n.alias('pb', pbs)
  n.alias('libs', libs)
end

def includes(config, platform, n)
  all_includes = FileList[]

 # Should be doing away with this as we no longer have prebuilt engine distributions
  # Include this file even on non-windows builds because the
  # boilerplate tool needs it to produce correct results
  if config.target_platform != 'win'
    win_common_h = "Engine/Common/_win/Common_ps.h"
    includes_win_common_h = "#{config.includes_output_dir}/Engine/Common/_win/Common_ps.h"
    all_includes.include(includes_win_common_h)
    GeneratorUtil.copy_files(n, {win_common_h => includes_win_common_h})
  end

  libs = config.libs().map do |lib|
    include_root = "Engine/#{lib}"

    headers = get_sourcelist(include_root, config.target_platform,
                             platform.underscore_dirs_whitelist, true)

    platform_headers = get_platform_sourcelist(include_root, config.target_platform, "Engine/",
                             platform.underscore_dirs_whitelist, true)

    write_file_list(headers, config.code_working_dir, include_root, '.headers.txt')
    write_file_list(platform_headers, config.code_working_dir, include_root, '.platformheaders.txt')

    pairs = headers.map {|h| [h, "#{config.includes_output_dir}/#{h}"] }
    targets = Hash[pairs]

    GeneratorUtil.copy_files(n, targets)

    all_includes.include(targets.values)
  end

  all_includes.to_a
end

# This is a utility method that defines data about engine projects.
# The first entry is the short name, which is also the directory name
# under the Engine directory.  The second entry is the relative path
# to the project file that will be created.  The third entry is the
# project GUID, which is used by Visual Studio. (The solution file
# must use thse GUIDs.)
def vs_project_tuples
  [
   ['AI', 'Engine/AI/AI.vcxproj', 'FB917B79-9C70-45F8-8674-1428B7895167'],
   ['Audio', 'Engine/Audio/Audio.vcxproj', '2219A034-B5EF-44FF-B7DC-981CB58711B5'],
   ['Common', 'Engine/Common/Common.vcxproj', '81E3ED0A-3D5F-4689-BDB6-DDF15B06EEE4'],
   ['Core', 'Engine/Core/Core.vcxproj', '4F44A822-08D7-424A-8D40-72677D8346A4'],
   ['Debug', 'Engine/Debug/Debug.vcxproj', '505A7767-C568-4303-8979-116FF110C8FF'],
   ['Game', 'Engine/Game/Game.vcxproj', '1DA383D2-7811-445A-AACC-CA255B994732'],
   ['Framework', 'Engine/Framework/Framework.vcxproj', '1DA383D2-7811-445A-AACC-CA255B994731'],
   ['GUI', 'Engine/GUI/GUI.vcxproj', 'C918F277-3782-4FAD-A501-B5F861A1C5C0'],
   ['Graphics', 'Engine/Graphics/Graphics.vcxproj', 'A73A2DDE-2075-496C-9770-21395924E08B'],
   ['HID', 'Engine/HID/HID.vcxproj', '6C1FFE72-4FE6-4890-A961-425CAC514F62'],
   ['Layout', 'Engine/Layout/Layout.vcxproj', 'F9B13ED7-9CF4-4151-9CB4-3B7EDBAE1566'],
   ['Maths', 'Engine/Maths/Maths.vcxproj', '258C9D89-38FB-4B1E-81E1-6D32AC81FF17'],
   ['Memory', 'Engine/Memory/Memory.vcxproj', '1C319D5B-0060-4653-9000-2ED6C92ACAEF'],
   ['Network', 'Engine/Network/Network.vcxproj', 'F68E2BF2-E65E-422F-BBED-249E476331A0'],
   ['Particles', 'Engine/Particles/Particles.vcxproj', '10466B50-9371-4F53-815B-A024DD74C7E7'],
   ['Physics', 'Engine/Physics/Physics.vcxproj', '41E78C13-BDDA-4803-9FE5-1FA6B7607685'],
   ['PostFX', 'Engine/PostFX/PostFX.vcxproj', 'BE5CB977-8776-4EFB-A718-766CF0026E4D'],
   ['Resource', 'Engine/Resource/Resource.vcxproj', '8E6B74E5-B0C4-4AC1-8E09-7C97747B3F2E'],
   ['Scene', 'Engine/Scene/Scene.vcxproj', '6FEBD175-576B-411D-8173-2833305AE882'],
   ['System', 'Engine/System/System.vcxproj', '2CA5C758-FA4D-431A-86EF-79065E243D3E'],
   ['ThirdParty', 'Engine/ThirdParty/ThirdParty.vcxproj', '491FE2EF-D449-4FDD-A3CE-9A1102AA2AF4']
  ]
end

def vs_project_dll_tuples
  [
       ['Oculus', 'Engine/Oculus/Oculus.vcxproj', 'BE5CB977-8776-4EFB-A718-766CF0026E4A', 'Engine/Oculus/Oculus.vcxproj.erb'],
  ]
end

def pbcs_extension_dll(config, n)
  core_csharp_protos_proj = "#{config.csharp_engine_root}/CorePBs.csproj"
  core_protos = "#{config.csharp_engine_root}/.coreproto.txt"

  # we don't need anything else, in theory...!
  options = {:template => 'Engine/CorePBs.csproj.erb', :rootdir => config.protocol_csharp_output_dir}
  GeneratorUtil.create_project_file(config, n, config.core_csharp_protos_proj, core_protos,
                                    options, [], [core_protos])

  [core_csharp_protos_proj]
end

def pb_dll(config, n)
  lib_protos = config.libs().map { |l| "#{config.code_working_dir}/Engine/#{l}/.proto.txt" }
  proto_list = FileList[lib_protos.flat_map{|p| File.readlines(p).map{|line| line.chomp}}]
  usagi_protos = "#{config.csharp_engine_root}/.proto.txt"
  write_file_list(proto_list, config.csharp_engine_root, "", '.proto.txt')

  GeneratorUtil.create_project_file(config,
                                    n,
                                    config.usagi_csharp_protos_proj,
                                    usagi_protos,
                                    {:template => 'Engine/UsagiPBs.csproj.erb', :rootdir => config.protocol_csharp_output_dir},
                                    [],
                                    config.protocol_csharp_classes()
                                    )

  [config.usagi_csharp_protos_proj]
end

def vs_project(config, n, order_only_deps)
  FileUtils.mkdir_p(config.vs_projects_dir)

  vs_project_tuples().map do |name, proj, guid|
    output = File.join(config.vs_projects_dir, proj)
    # TODO: fix these when adding the next projects
    rootdir = File.dirname(proj)
    sources = File.join(config.code_working_dir, rootdir, '.sources.txt')
    platform_source = File.join(config.code_working_dir, rootdir, 'platform.sources.txt')
    options = {:template => 'Engine/Project.vcxproj.erb', :rootdir => rootdir, :guid => guid}

    GeneratorUtil.create_project_file(config, n, output, sources, options,
                                      [], order_only_deps)

    output
  end
end

def vs_project_dll(config, n, order_only_deps)
    FileUtils.mkdir_p(config.vs_projects_dir)

    vs_project_dll_tuples().map do |name, proj, guid, template|
    output = File.join(config.vs_projects_dir, proj)
    # TODO: fix these when adding the next projects
    rootdir = File.dirname(proj)
    sources = File.join(config.code_working_dir, rootdir, '.sources.txt')
    platform_source = File.join(config.code_working_dir, rootdir, 'platform.sources.txt')
    options = {:template => template, :rootdir => rootdir, :guid => guid}

    GeneratorUtil.create_project_file(config, n, output, sources, options,
                                      [], order_only_deps)

    output
  end
end


def build_windows_files(config, n, generated_files, win_bp_cpp)
  projects = Set.new

  pb_project = pb_dll(config, n)
  projects.merge pb_project

  # we should make sure that when project generator runs, the .pb.cs files
  # are regenerated
  # projects.merge ['pbs_csharp']

  engine_projects = vs_project(config, n, generated_files + win_bp_cpp)
  engine_projects += vs_project_dll(config, n, generated_files + win_bp_cpp)
  projects.merge engine_projects
  n.alias('projects', projects)

  exe_files = config.protocol_python_classes + config.protocol_ruby_classes +
    generated_files + win_bp_cpp
  n.alias('exe_files', exe_files)
  n.defaults('exe_files')
end

def boilerplate(config, n, generated_code, win_bp_cpp)
  system_sources = grep(["namespace.Systems", "public.*System"], ['Engine'], ["*.h", "*.cpp"])

  lib_ymls = []

  registersystem_objects, lib_cpps = config.libs().map do |lib|
    source_root = Pathname.new("Engine/#{lib}")

    pch_source_file = config.pch_source_file(source_root.to_s)
    pch_file = nil

#   if config.target_platform == 'ctr' && File.exist?(pch_source_file)
#      pch_file = config.pch_file(source_root.to_s, lib)
#      generated_code << pch_file
#    end

    lib_systems = system_sources.reject{ |h| Pathname.new(h).relative_path_from(source_root).cleanpath.to_s =~ /^\.\./ }
                                .map{ |h| "#{(h + ".bp.cpp").gsub("\\", "/")}" }

    lib_system_ymls = lib_systems.map{ |s| "#{config.code_working_dir}/#{s.sub(/\.bp\.cpp$/, ".yml")}" }
    lib_yml = "#{config.code_working_dir}/#{source_root}/#{lib}Systems.yml"
    n.build('combine_yaml', {lib_yml => lib_system_ymls},
            {:implicit_deps => [config.yaml_combiner]})

    lib_registersystems_cpp_bp = "#{source_root}/Register#{lib}Systems.bp.cpp"
    lib_registersystems_cpp = "#{config.code_working_dir}/#{lib_registersystems_cpp_bp}"
    lib_registersystems_o = lib_registersystems_cpp.sub(/\.cpp$/, ".o")
    n.build('boilerplate_template',
            {lib_registersystems_cpp => [config.register_systems_template]},
            {:implicit_deps => [lib_yml],
            :variables => {'yml' => lib_yml}})

    GeneratorUtil.copy_boilerplate_to_vs_build_dir(config, n, lib_registersystems_cpp, win_bp_cpp)

#    if config.target_platform == 'ctr'
#      GeneratorUtil.compile_source(config, n,
#                                   {lib_registersystems_cpp => lib_registersystems_o},
#                                   generated_code, pch_file)
#    end

    lib_system_cpps, lib_system_hs = lib_systems.partition{ |s| s =~ /\.cpp\.bp\.cpp/ }
    write_file_list(lib_system_hs + [lib_registersystems_cpp_bp], config.code_working_dir, source_root, '.boilerplate.txt')
    lib_ymls << lib_yml

    [lib_registersystems_o, lib_system_cpps]
  end.transpose

  system_objects = GeneratorUtil.process_system_sources(config, n, system_sources, generated_code, win_bp_cpp)

  [registersystem_objects + system_objects, lib_cpps.flatten().map { |cpp| "#{config.code_working_dir}/#{cpp}"}]
end

#####################################################################
# Housekeeping                                                      #
#####################################################################
clean_protocol_ruby_classes($engine)

#####################################################################
# Variables                                                         #
#####################################################################
GeneratorUtil.define_variables($engine, $platform, n)
GeneratorUtil.define_ruby_variables($engine, $platform, n)
GeneratorUtil.define_protoc_includes_variable(n)

#####################################################################
# Rules                                                             #
#####################################################################
GeneratorUtil.define_base_rules($engine, $platform, n)

case $engine.target_platform
when 'win'
  GeneratorUtil.define_pc_rules($engine, $platform, n)
when 'nx', 'nxe'
  GeneratorUtil.define_switch_rules($engine, $platform, n)
end

#####################################################################
# Build Statements                                                  #
#####################################################################
GeneratorUtil.create_build_id($engine, n)
generated_headers = $engine.protocol_headers + [$engine.build_id_header]
includes = includes($engine, $platform, n)
generated_headers.push(*includes)
n.alias('includes', generated_headers)

win_bp_cpp = []
bp_objects, bp_generated_headers = boilerplate($engine, n, generated_headers, win_bp_cpp)

libs($engine, $platform, n, bp_objects, generated_headers + bp_generated_headers)
GeneratorUtil.nanopb_protocols($engine, n)
GeneratorUtil.usagi_protocols($engine, n)

# TODO: If we ever want a command-line NX build, we'll have to check
# to see VISUAL_STUDIO_BUILD is defined in the environment when the
# platform is 'nx'
SHOULD_BUILD_WINDOWS_FILES = $engine.target_platform == 'win' ||
  $engine.target_platform == 'nx' || $engine.target_platform == 'nxe'

if SHOULD_BUILD_WINDOWS_FILES
  build_windows_files($engine, n, generated_headers + bp_generated_headers, win_bp_cpp)
end

# hack to allow data builds for games
n.alias('data', 'pb')

n.save(NINJA_FILE)
