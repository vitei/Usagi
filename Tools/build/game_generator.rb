# encoding: UTF-8
require 'set'
require 'pathname'
require 'yaml'


#####################################################################
# Utility Functions                                                 #
#####################################################################
def build_pc_data(config, n, platform)
  data_deps = Set.new

  audio = build_audio(config, n)
  data_deps.merge audio

  shaders = build_pc_shaders(config, n, platform)
  data_deps.merge shaders

  effects = build_pc_effects(config, n)
  data_deps.merge effects

  textures = build_pc_textures(config, n)
  data_deps.merge textures

  models = build_pc_models(config, n)
  data_deps.merge models

  fonts = build_fonts(config, n)
  data_deps.merge fonts

  custom_effects = build_custom_effects(config, n)
  data_deps.merge custom_effects


  data_deps
end

# All data processing functions should return an array of the output
# files they produce, and should be called from this function so we
# can build the name data hash list and the phony 'data' rule.
def process_data(config, platform, n)
  data_deps = Set.new


  # Do this first to ensure we have the hierarchy information
  # Convert models
  if config.target_platform == 'win'
    pc_data = build_pc_data(config, n, platform)
    data_deps.merge pc_data
  end

  # find once, and pass around the array into build rule functions
  protocol_ruby_classes = find_protocol_ruby_classes(config)

  collision = build_collision_models(config, platform, n)
  data_deps.merge collision

  entities = build_engine_entities(config, n, protocol_ruby_classes)
  data_deps.merge entities

  entities = build_entities(config, n, protocol_ruby_classes)
  data_deps.merge entities

  levels = build_levels(config, n, protocol_ruby_classes)
  data_deps.merge levels

  skeletal_animations = build_skeletal_animations(config, n)
  data_deps.merge skeletal_animations

  behavior_trees = build_behavior_data(config, n, protocol_ruby_classes)
  data_deps.merge behavior_trees

  lua_scripts = build_lua_scripts(config, n)
  data_deps.merge lua_scripts

  emitters = build_emitters(config, n)
  data_deps.merge emitters

  vpb_files = build_vpb_files(config, n, protocol_ruby_classes)
  data_deps.merge vpb_files


  # FIXME: Letter list

  # FIXME: Convert fonts

  build_hash_list(config, n, data_deps)

  data_deps
end


def projects(config, n, order_only_deps, win_bp_cpp, data_deps)
  all_projects = Set.new

  projects = vs_project(config, n, order_only_deps + win_bp_cpp)
  all_projects.merge projects

  pb_project = pb_dll(config, n)
  all_projects.merge pb_project

  n.alias('projects', all_projects.to_a)

  exe_files = ['includes'] + order_only_deps + win_bp_cpp +
    data_deps.to_a + [config.name_data_hash_list]
  n.alias('exe_files', exe_files)
  n.defaults('exe_files')
end


def build_model_paks(config, n, models_textures, models_resources, vfx_ext, aux_materials)
  pak_resources  = models_resources.select { |resource| resource.include?(vfx_ext) } # only want VFX resources
  pak_resources |= aux_materials.select { |resource| resource.include?(vfx_ext) } if aux_materials.nil? == false
  pak_resources |= models_textures
  folders = pak_resources.map { |f| File.dirname(f) }.uniq

  targets = folders.map do |src_path|
    output_path  = "#{src_path}".sub!("#{config.working_dir}/Data", "#{config.romfiles_dir}")
    dst_path     = "#{output_path}/resources.pak"
    dependencies = pak_resources.select { |f| f.include?(src_path) }

    [src_path, dst_path, dependencies] if dependencies.any?
  end.compact

  GeneratorUtil.pak_data(config, n, targets)

  targets.map {|i, o, d| o}
end

def build_particle_paks(config, n)
  particle_src = [["#{config.particle_dir}/Effects", '.pfx'], ["#{config.particle_dir}/Emitters", '.pem']]

  output_dir = "#{config.particle_working_dir}"

  dependencies = []
  particle_src.each do |src_info|
    src_folder = src_info[0]
    cvt_ext = src_info[1]
    FileList["#{src_folder}/*.vpb"].each do |file|
      dependencies << file.sub(src_folder, output_dir).sub!('.vpb', cvt_ext)
    end
  end

  targets  = []
  if dependencies.any?
    dst_path = "#{config.particle_out_dir}/resources.pak"
    targets << [config.particle_working_dir, dst_path, dependencies]
    GeneratorUtil.pak_data(config, n, targets)
  end

  targets.map {|i, o, d| o}
end


#####################################################################
# PC-Specific Functions                                             #
#####################################################################

def build_audio(config, n)
  pairs = FileList["Data/Audio/*.wav"].select{|f| !File.directory? f}.map do |input|
    [input, "#{config.romfiles_dir}/Audio/#{input.sub(/^Data\/Audio\//, "")}"]
  end

  targets = Hash[pairs]

  GeneratorUtil.copy_files(n, targets)

  targets.values
end

def build_pc_shaders(config, n, platform)
  if platform.underscore_dirs_whitelist.include?("_vulkan")
    game_shaders = build_vulkan_shaders_for_dir(config, n, config.pc_shader_dir(false))
    engine_shaders = build_vulkan_shaders_for_dir(config, n, config.pc_shader_dir)
  else
    game_shaders = build_pc_shaders_for_dir(config, n, config.pc_shader_dir(false))
    engine_shaders = build_pc_shaders_for_dir(config, n, config.pc_shader_dir)
  end

  game_shaders + engine_shaders
end

def build_pc_shaders_for_dir(config, n, shader_dir)
  targets = FileList["#{shader_dir}/**/*"].exclude{|f| File.directory?(f)}.map do |input|
    output = "#{config.shader_out_dir}/" + input.sub(/#{shader_dir}\//, '')

    [input, output]
  end

  GeneratorUtil.copy_files(n, targets)

  targets.map{|i, o| o}
end


def build_vulkan_shaders_for_dir(config, n, shader_dir)
  targets = FileList["#{shader_dir}/**/*.vert", "#{shader_dir}/**/*.frag", "#{shader_dir}/**/*.geom"].exclude{|f| File.directory?(f)}.map do |input|
    output = "#{config.shader_out_dir}/" + input.sub(/#{shader_dir}\//, '') + ".spv"
    defines = ""
    n.build('vulkanshader', {output => [input]},
        :variables => {'out' => to_windows_path(output),
        'in' => input})

    output
  end
end

def build_pc_effects(config, n)
  game_effects = build_pc_effects_for_dir(config, n, config.effects_dir(false))
  engine_effects = build_pc_effects_for_dir(config, n, config.effects_dir)

  game_effects + engine_effects
end

def build_pc_effects_for_dir(config, n, effects_dir)
  effects = FileList["#{effects_dir}/**/*"].exclude { |f| File.directory?(f) }

  targets = effects.map do |input|
    output = "#{config.effects_out_dir}/" + input.sub(/#{effects_dir}\//, '')

    [input, output]
  end

  GeneratorUtil.copy_files(n, targets)

  targets.map{|i, o| o}
end

def build_pc_textures(config, n)
  game_textures = build_pc_textures_for_dir(config, n, config.textures_dir(false))
  engine_textures = build_pc_textures_for_dir(config, n, config.textures_dir)

  model_textures = build_pc_model_textures_for_dir(config, n, config.model_dir)
  game_textures + engine_textures + model_textures
end

def build_pc_model_textures_for_dir(config, n, textures_dir)
  targets = FileList["#{textures_dir}/**/*.dds"].map do |input|
    tex, ext = input.match(/\/([^\/]*)\.([^.\/]*)$/).captures
    path = Pathname.new(input)
    tex = path.relative_path_from(Pathname(textures_dir)).sub_ext('')
    output = "#{config.model_out_dir}/#{tex}.#{ext}"

    [input, output]
  end

  GeneratorUtil.copy_files(n, targets)

  targets.map{|i, o| o}
end

def build_pc_textures_for_dir(config, n, textures_dir)
  targets = FileList["#{textures_dir}/**/*.tga", "#{textures_dir}/**/*.dds"].map do |input|
    tex, ext = input.match(/\/([^\/]*)\.([^.\/]*)$/).captures
    path = Pathname.new(input)
    tex = path.relative_path_from(Pathname(textures_dir)).sub_ext('')
    output = "#{config.textures_out_dir}/#{tex}.#{ext}"

    [input, output]
  end

  GeneratorUtil.copy_files(n, targets)

  targets.map{|i, o| o}
end

def build_fonts(config, n)

  font_targets = Array.new
  FileUtils.mkdir_p(config.font_out_dir)

  fonts = FileList["#{config.font_dir}/*.yml"].map do |input|
    out_stub = input.sub("#{config.font_dir}/", "#{config.font_out_dir}/").sub(/.yml/, '')
    output = "#{out_stub}.vpb"

    n.build('font', {output => [input]},
        :variables => {'out_stub' => to_windows_path(out_stub),
        'in' => input})

    output
  end
end

def build_pc_models(config, n)
  FileUtils.mkdir_p(config.shader_out_dir)
  FileUtils.mkdir_p(config.skeleton_out_dir)
  FileUtils.mkdir_p(config.effects_out_dir)

  models = FileList["#{config.model_dir}/**/*.{fbx}"].exclude do |f|
    File.directory?(f) || f.include?("/collision")
  end

  variables_hash = {
    'shdir' => config.shader_out_dir,
    'skel' => config.skeleton_out_dir,
  }

  models.map do |input|
    model_sub = input
      .sub(/#{config.model_working_dir}\//, "")
      .sub(/#{config.model_dir}\//, "")
      .sub(/.fbx/,  '.vmdf')
    output = "#{config.model_out_dir}/#{model_sub}"
    model_out_dir = File.dirname(output)

    FileUtils.mkdir_p(File.dirname output)
    model_variables_hash = variables_hash.clone
    model_variables_hash['fxdir'] = model_out_dir
    model_variables_hash['skel'] = "#{config.skeleton_out_dir}/#{model_sub}.xml"
    model_variables_hash['skanimdir'] = "#{model_out_dir}/"
    FileUtils.mkdir_p(File.dirname(model_variables_hash['skel']))
    n.build('cmdl', {output => [input]},
              {:implicit_deps => [config.cmdl_converter],
              :variables => model_variables_hash})

    output
  end
end

def build_custom_effects(config, n)
  FileUtils.mkdir_p(config.shader_out_dir)
  FileUtils.mkdir_p(config.effects_out_dir)

  custom_effects = FileList["#{config.custom_effect_dir}/*.yml", "#{config.custom_effect_dir(false)}/*.yml"].exclude{|f| File.directory?(f)}

  custom_effects.map do |input|
    effect_sub = input.sub(/(#{config.custom_effect_dir(false)}|#{config.custom_effect_dir})\/(.*)\.yml$/, '\2.cfx')
    output = "#{config.custom_effect_out_dir}/#{effect_sub}"
    effect_out_dir = File.dirname(output)

    FileUtils.mkdir_p(File.dirname output)
    n.build('custom_effect', {output => [input]} )

    output
  end
end


#####################################################################
# Game-Specific Functions                                           #
#####################################################################

def find_protocol_ruby_classes(config)
  config.protocol_ruby_classes(false)
end

def find_csharp_pb_classes(config)
  config.protocol_csharp_classes(false)
end

def behavior_tree_targets(config)
  pairs = FileList["Data/BehaviorTrees/**/*.btyml"].map do |input|
    output = "#{config.romfiles_dir}/#{input.sub(/^Data\//, "")}".sub(/btyml$/, "vbt")

    [input, output]
  end

  Hash[pairs]
end

def xml_behavior_tree_targets(config)
  pairs = FileList["Data/BehaviorTrees/**/*.btxml"].map do |input|
    output = "#{config.romfiles_dir}/#{input.sub(/^Data\//, "")}".sub(/btxml$/, "vbt")

    [input, output]
  end

  Hash[pairs]
end

def build_behavior_data(config, n, deps)
  bt_targets = behavior_tree_targets(config)
  GeneratorUtil.process_behavior_tree_data(config, n, bt_targets, 'behavior_tree', deps)

  xml_bt_targets = xml_behavior_tree_targets(config)
  GeneratorUtil.process_behavior_tree_data(config, n, xml_bt_targets, 'xml_behavior_tree', deps)

  bt_targets.values + xml_bt_targets.values
end

def collision_model_targets(config, n)
  pairs = FileList["Data/Models/collision/*.fbx"].map do |cm|
    cm_out = "#{config.romfiles_dir}/#{cm.sub(/^Data\//, "")}".sub(/.(fbx)/, '.coll')

    [cm, cm_out]
  end

  Hash[pairs]
end

def build_collision_models(config, platform, n)
  targets = collision_model_targets(config, n)
  GeneratorUtil.process_model_data(config, n, targets, 'collision')

  targets.values
end


def skeletal_animation_targets(config)
  pairs = FileList["Data/Models/**/*.cskla"].map do |input|
    ext = File::extname input
    output = "#{config.romfiles_dir}/#{input.sub(/^Data\//,"")}".sub(/#{ext}$/, ".vskla")

    [input, output]
  end

  Hash[pairs]
end

def build_skeletal_animations(config, n)
  targets = skeletal_animation_targets(config)
  GeneratorUtil.process_model_data(config, n, targets, 'skeletal_animation')

  targets.values
end

def build_entities(config, n, deps)
  inputs = FileList["Data/Entities/**/*.yml"]

  pairs = inputs.map do |i|
    [i, "#{config.romfiles_dir}/#{i.sub(/^Data\//, "")}".sub(/yml$/, 'vent')]
  end

  targets = Hash[pairs]
  GeneratorUtil.process_entities(config, n, targets, deps)

  targets.values
end

def build_engine_entities(config, n, deps)
  inputs = FileList["Usagi/Data/Entities/**/*.yml"]

  pairs = inputs.map do |i|
    [i, "#{config.romfiles_dir}/#{i.sub(/^Usagi\/Data\//, "")}".sub(/yml$/, 'vent')]
  end

  targets = Hash[pairs]
  GeneratorUtil.process_entities(config, n, targets, deps)

  targets.values
end

# This function returns an arry of tuples, where
# the tuple format is as follows:
# [input_file, yaml_file, position_yaml, instance_yaml]
def level_tuples(config)
  inputs = FileList["Data/Levels/*.lvl"]

  tuples = Array.new

  inputs.each do |i|
    manifest = "#{config.working_dir}/#{i}".sub(/\.lvl$/, '_manifest.txt')
    manifestlines = IO.readlines(manifest)
    manifestlines.each do |line|
      files = line.chomp.split(/\t/).map do |f|
        f = "#{config.working_dir}/Data/Levels/#{f}"
      end
      tuples.push ([i, files[0], files[1], files[2]])
    end
  end

  tuples
end

def build_levels(config, n, deps)
  tuples = level_tuples(config)
  pairs = tuples.map {|t| [t[0], t[1..3]]}
  GeneratorUtil.create_intermediate_level_files(config, n, pairs)

  # 1. hierarchy
  hierarchy_pairs = tuples.map do |level, yaml, pos, instance|
    hierarchy = "#{yaml.sub(config.working_dir, config.romfiles_dir)}".sub(/yml$/, 'vhir').sub(/Data\//, "")
    [yaml, hierarchy]
  end

  hierarchy_targets = Hash[hierarchy_pairs]
  GeneratorUtil.process_entities(config, n, hierarchy_targets, deps)

  # 2. positions
  pos_pairs = tuples.map do |level, yaml, pos, instance|
    pos_out = "#{pos.sub(config.working_dir, config.romfiles_dir)}".sub(/_pos\.yml$/, '.vpos').sub(/Data\//, "")

    [pos, pos_out]
  end

  pos_targets = Hash[pos_pairs]
  GeneratorUtil.process_positions(config, n, pos_targets, deps)

  # 3. instances
  instance_yaml = tuples.map {|level, yaml, pos, instance| instance}

  instance_pairs = instance_yaml.zip(pos_targets.values).map do |y, o|
    [y, o.sub(/.vpos/, '.vins')]
  end

  instance_targets = Hash[instance_pairs]
  GeneratorUtil.process_positions(config, n, instance_targets, deps)

  hierarchy_targets.values + pos_targets.values + instance_targets.values
end

def project_code(config, platform, n, order_only_deps)
  # Assume that engine headers have been created,
  # either by running the engine generator or downloading
  pbs = []
  source_root = config.project
  pch_file = nil
  pch_source_file = config.pch_source_file(source_root)

  sources = get_sourcelist(source_root, config.target_platform,
                           platform.underscore_dirs_whitelist)
  targets = gen_source_targets(sources, config, platform)

  standard_targets = targets.reject{|c, o| c.end_with?('.pb.cpp') || c.end_with?('.lua.cpp')}
  write_file_list(standard_targets.keys, config.code_working_dir, source_root)

  proto_files = sources.select{|s| s.end_with?('.proto')}
  write_file_list(proto_files, config.code_working_dir, source_root, '.proto.txt')


  pb_cpp = GeneratorUtil.cpp_protobuf(config, n, proto_files)
  pbs.concat pb_cpp
  pb_lua = GeneratorUtil.lua_protobuf(config, n, proto_files)
  pbs.concat pb_lua
  pb_rb = GeneratorUtil.ruby_protobuf(config, n, proto_files)
  pbs.concat pb_rb
  pb_csharp = GeneratorUtil.csharp_protobuf(config, n, proto_files)
  pbs.concat pb_csharp
  # These are only needed by the level processing script, which we
  # don't use in this project
  # pb_py = GeneratorUtil.python_protobuf(config, n, proto_files)
  # pbs.concat pb_py

  n.alias('pb', pbs)

  targets.values
end

def project_exe(config, platform, n, project_obj)
  # Currently building through VS
end

def project_includes(config, platform, n)
  include_root = config.project
  headers = get_sourcelist(include_root, config.target_platform,
                           platform.underscore_dirs_whitelist, true)
  write_file_list(headers, config.code_working_dir, include_root, '.headers.txt')
  pairs = headers.map {|h| [h, "#{config.includes_output_dir(false)}/#{h}"] }
  targets = Hash[pairs]
  GeneratorUtil.copy_files(n, targets)
  n.alias('includes', targets.values)

  targets.values
end

def build_hash_list(config, n, data_set)
  list_targets = {config.romfiles_dir => config.data_list_file}
  GeneratorUtil.create_data_list(n, list_targets, data_set.to_a)
  GeneratorUtil.create_hash_list(config, n)
end

def build_lua_scripts(config, n)
  pairs = FileList["Data/Scripts/**/*.lua"].map do |input|
    output = "#{config.romfiles_dir}/#{input.sub(/^Data\//,"")}"

    [input, output]
  end

  targets = Hash[pairs]

  targets.each do |input, output|
    FileUtils.mkdir_p(File.dirname(output))
    n.build('luac', {output => [input]}, {:implicit_deps => [config.luac]})
  end

  targets.values
end

def build_emitters(config, n)
  hash = { '/Emitters' => 'pem', '/Effects' => 'pfx' }

  output_dir = config.particle_working_dir

  if config.target_platform == 'win'
    output_dir = config.particle_out_dir
  end

  pairs = FileList["Data/Particle/**/*.vpb"].map do |input|
    emitter = File.basename(input)
    ext = hash.map { |k, v| v if File.dirname(input).include?(k) }.compact.first
    emitter.sub!('vpb', ext) unless ext.nil? || ext.empty?
    output = "#{output_dir}/#{emitter}"

    [input, output]
  end

  targets = Hash[pairs]

  GeneratorUtil.copy_files(n, targets)

  targets.values
end

def vpb_targets(config)
  pairs = FileList["Data/VPB/**/*.yml"].map do |input|
    [input, "#{config.romfiles_dir}/#{input.sub(/^Data\//, "")}".sub(/yml$/, "vpb")]
  end

  Hash[pairs]
end

def build_vpb_files(config, n, deps)
  targets = vpb_targets(config)

  targets.each do |input, output|
    FileUtils.mkdir_p(File.dirname(output))
    n.build('vpb', {output => [input]},
            {:order_only_deps => deps, :implicit_deps => [config.vpb_converter]})
  end

  targets.values
end
 

def pb_dll(config, n)
  project_sources = "#{config.code_working_dir}/#{config.project}/.sources.txt"
  options = {:template => "#{config.project}/#{config.project}PBs.csproj.erb", :rootdir => config.protocol_csharp_output_dir(false)}
  GeneratorUtil.create_project_file(config, n, config.project_csharp_protos_proj,
                                    project_sources, options,
                                    [project_sources], find_csharp_pb_classes(config), config.project)

  [config.project_csharp_protos_proj]
end

def vs_project(config, n, order_only_deps)
  FileUtils.mkdir_p(config.vs_projects_dir(false))

  outputs = []
  vs_projects = ["#{config.project}/#{config.project}.vcxproj"]

  vs_projects.each do |proj|
    output = File.join(config.vs_projects_dir(false), proj)
    template = proj + '.erb'
    # TODO: fix these when adding the next projects
    rootdir = File.dirname(proj)
    sources = File.join(config.code_working_dir, rootdir, '.sources.txt')

    GeneratorUtil.create_project_file(config, n, output, sources,
                                      {:template => template, :rootdir => rootdir},
                                      [], order_only_deps, config.project)
    outputs << output

    settings_input = "#{proj}.user"
    settings_output = File.join(config.vs_projects_dir(false), settings_input)
    n.build('cp', {settings_output => [settings_input]})
    outputs << settings_output
  end

  outputs
end


def boilerplate(config, n, generated_code, win_bp_cpp)
  system_sources = grep(["namespace.Systems", "public.*System"], [config.project], ["*.h", "*.cpp"])
  project_systems = system_sources.map{ |h| "#{(h + ".bp.cpp").gsub("\\", "/")}" }

  project_system_ymls = project_systems.map{ |s| "#{config.code_working_dir}/#{s.sub(/\.bp\.cpp$/, ".yml")}" }
  project_yml = "#{config.code_working_dir}/#{config.project}Systems.yml"
  n.build('combine_yaml', {project_yml => project_system_ymls},
          {:implicit_deps => [config.yaml_combiner]})

  pch_source_file = config.pch_source_file(config.project)
  pch_file = nil

  project_registersystems_cpp_bp = "#{config.project}/Register#{config.project}Systems.bp.cpp"
  project_registersystems_cpp = "#{config.code_working_dir}/#{project_registersystems_cpp_bp}"
  project_registersystems_o = project_registersystems_cpp.sub(/\.cpp$/, ".o")
  n.build('boilerplate_template',
            {project_registersystems_cpp => [config.register_systems_template]},
            {:implicit_deps => [project_yml],
            :variables => {'yml' => project_yml}})

  GeneratorUtil.copy_boilerplate_to_vs_build_dir(config, n, project_registersystems_cpp, win_bp_cpp)


  project_system_cpps, project_system_hs = project_systems.partition{ |s| s =~ /\.cpp\.bp\.cpp/ }
  write_file_list(project_system_hs + [project_registersystems_cpp_bp], config.code_working_dir, config.project, '.boilerplate.txt')
  registersystem_objects = [project_registersystems_o]
  system_objects = GeneratorUtil.process_system_sources(config, n, system_sources, generated_code, win_bp_cpp, pch_file)

  [system_objects + registersystem_objects, project_system_cpps.map { |cpp| "#{config.code_working_dir}/#{cpp}"}]
end
