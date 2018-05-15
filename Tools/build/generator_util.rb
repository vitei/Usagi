# Usagi Engine, Copyright (c) Vitei, Inc. 2013

# Description: Utility functions for writing Ninja file generators.
require_relative 'build_config.rb'
require_relative 'platform.rb'
require_relative 'utils.rb'

require 'ninja'
require 'nokogiri'

module GeneratorUtil
#####################################################################
# Basic Functions                                                   #
#####################################################################

  def GeneratorUtil.define_variables(config, platform, n)
    n.variable('builddir', config.working_dir)
    n.variable('project', config.project)

    n.variable('msbuild', config.msbuild)

    n.variable('cc', platform.cc)
    n.variable('cxx', platform.cxx)
    n.variable('cm', platform.cm)
    n.variable('cmm', platform.cmm)
    n.variable('luac', config.luac)
    n.variable('ar', platform.ar)
    n.variable('ld', platform.ld)

    n.variable('cppflags', ENV['CPPFLAGS'] || platform.cppflags.join(' '))
    n.variable('cmflags',  ENV['CMFLAGS']  || platform.cmflags.join(' '))
    n.variable('cflags',   ENV['CFLAGS']   || platform.cflags.join(' '))
    n.variable('cxxflags', ENV['CXXFLAGS'] || platform.cxxflags.join(' '))
    n.variable('arflags',  ENV['ARFLAGS']  || platform.arflags.join(' '))
    n.variable('ldflags',  ENV['LDFLAGS']  || platform.ldflags.join(' '))
    n.variable('pchflags', platform.pchflags.join(' '))

    n.variable('protoc', config.protoc)
    n.variable('protoc_gen_c', config.protoc_gen_c)
    n.variable('protoc_gen_cs', config.protoc_gen_cs)
    n.variable('protoc_gen_lua', config.protoc_gen_lua)
    n.variable('protoc_gen_ruby', config.protoc_gen_ruby)
    n.variable('nanopb_proto_dir', config.nanopb_proto_dir)
  end

  def GeneratorUtil.define_ruby_variables(config, platform, n, *dirs)
    all_dirs = [config.protocol_ruby_output_dir] + dirs
    includes = all_dirs.map{|d| "-I#{d}"}.join(' ')
    n.variable('ruby', 'ruby ' + includes)
    n.variable('require_all_pb_opt', includes.gsub('-I', '-R'))
  end

  def GeneratorUtil.define_protoc_includes_variable(n, *dirs)
    protoc_includes = '-I.'
    protoc_includes += dirs.map{|d| " -I#{d}"}.join('')
    n.variable('protoc_includes', protoc_includes)
    n.variable('protoc_includes_cs', protoc_includes.gsub(/-I/, '-r:'))
  end

  def GeneratorUtil.define_base_rules(config, platform, n)
    n.rule('cc',
           "$cc -c $cflags $cppflags #{platform.c_cpp_dep_flags} $in -o $out $use_pch",
           {:dependencies => :gcc, :description => 'CC $out'})

    n.rule('cxx',
           "$cxx -c $cxxflags $cppflags #{platform.c_cpp_dep_flags} $in -o $out $use_pch",
           {:dependencies => :gcc, :description => 'CXX $out'})

    n.rule('pch',
           "$cxx -c $cxxflags $cppflags $pchflags #{platform.c_cpp_dep_flags} $in -o $out.dummy #{platform.create_pch_flag}=$out",
           {:dependencies => :gcc, :description => 'PCH $out'})

    n.rule('cm',
           "$cm -c $cflags $cmflags $cppflags #{platform.c_cpp_dep_flags} $in -o $out",
           {:dependencies => :gcc, :description => 'CM $out'})

    n.rule('cmm',
           "$cmm -c $cmflags $cxxflags $cppflags #{platform.c_cpp_dep_flags} $in -o $out",
           {:dependencies => :gcc, :description => 'CMM $out'})

    n.rule('luac', '$luac -o $out $in', {:description => 'LUA $out'})
    n.rule('ar', '$ar $arflags $out $in', {:description => 'AR $out'})

    all_ld_flags = "$ldflags"
    all_ld_flags += " #{platform.additional_ld_flags}" if config.target_platform == 'ctr'
    n.rule('ld', "$ld #{all_ld_flags} -o $out --via $out.rsp",
           {:description => 'LINK $out', :rspfile => '$out.rsp',
             :rspfile_content => '$in $syslibs'})

    n.rule('protoc_cpp',
           '$protoc --nanopb_out=$outdir $protoc_includes -I$nanopb_proto_dir --plugin=protoc-gen-nanopb="$protoc_gen_c" $in',
           {:description => 'PROTOCOL_CPP $out'})

    n.rule('protoc_cs',
           "$protoc_gen_cs -q -ns:NoNamespace $protoc_includes_cs -r:$nanopb_proto_dir -i:$inproto -o:$out -dll:$dll",
           {:description => 'PROTOCOL_CS $out'})

    n.rule('protoc_rb',
           '$protoc --ruby_out=$outdir $protoc_includes -I$nanopb_proto_dir --plugin=protoc-gen-ruby="$protoc_gen_ruby" $in',
           {:description => 'PROTOCOL_RB $out'})

    n.rule('protoc_py',
           '$protoc --python_out=$outdir $protoc_includes -I$nanopb_proto_dir $in',
           {:description => 'PROTOCOL_PY $out'})

    n.rule('protoc_lua',
           '$protoc --nanopblua_out=$outdir $protoc_includes -I$nanopb_proto_dir --plugin=protoc-gen-nanopblua="$protoc_gen_lua" $in',
           {:description => 'PROTOCOL_LUA $out'})

    n.rule('cp',
           "$ruby -e \"FileUtils.cp_r('$in', '$out')\" -r'fileutils'",
           {:description => 'CP $out'})

    n.rule('cat',
           "$ruby -e \"File.open('$out', 'wb') { |f| f.puts ARGF.read }\" $in",
           {:description => 'CAT $out'})

    n.rule('build_id',
           "$ruby #{config.build_id_generator} $prefix_option $out -r #{config.region}",
           :description => 'BUILD_ID $out')

    n.rule('combine_yaml', "$ruby #{config.yaml_combiner} -o $out $in",
           {:description => 'YAML $out'})

    n.rule('boilerplate_template', "$ruby #{config.boilerplate_script} -y $yml -o $out $in",
           {:description => 'ERB $out'})

    n.rule('boilerplate', "$ruby #{config.boilerplate_scan} -d $out.d -o $out $in #{config.boilerplate_scan_options}",
           {:description => 'BOILERPLATE $out', :dependencies => :gcc})

    n.rule('project',
           "$ruby #{config.vs_project_generator} #{config.script_dep_opt} $guid -o $out $in $fail_opt $template $rootdir #{config.project}",
           {:description => 'VS PROJECT $out', :dependencies => :gcc})

  end

  def GeneratorUtil.define_system_rules(config, platform, n)
    n.rule('audiotool',
           # Note: We must use double quotes to quote the tool
           # parameters, as single quotes will cause errors.
           config.vitei_audio_tool + ' --proto -i="$in" -o="$out" -e="$enumCountName" -g="$ifndefName"',
           {:description => 'FSID_BUILD $out'})
  end

  def GeneratorUtil.define_data_rules(config, platform, n)
    n.rule('collision',
           "#{config.cmdl_converter} -o$out --collision #{config.cmdl_converter_dep_opts} -lh $in ",
           {:dependencies => :gcc, :description => 'COLLISION $out'})

    n.rule('name_data_hash_list',
           "$ruby #{config.name_data_hash_list_tool} -o $out -d #{config.romfiles_dir} $require_all_pb_opt $in",
           {:description => 'NAME DATA HASH LIST $out'})

    n.rule('data_list',
           "$ruby -e \"filters = ['#{config.name_data_hash_list}', 'resources.pak']; f=File.open('$out', 'w'); Dir.glob( '$in/**/*.*' ) {|name| f.puts name['$in'.length+1..-1] if filters.none?{|f| name.include?(f)}}; f.close;\"",
           {:description => 'DATA LIST $out'})

    n.rule('hierarchy',
           "$ruby #{config.entity_converter} $require_all_pb_opt #{config.entity_opts} -o $out $in",
           {:description => 'HIERARCHY $out', :dependencies => :gcc})

    n.rule('expandhierarchy',
           "$ruby #{config.entity_expander} $require_all_pb_opt -o $out $in",
           {:description => 'EXPAND $out', :dependencies => :gcc})

    n.rule('position',
           "$ruby #{config.maya_data_converter} #{config.script_dep_opt} -o $out $in",
           {:description => 'POSITION $out', :dependencies => :gcc})

    n.rule('level',
           "python #{config.level_converter} --dist 30.0 $in $out",
           {:description => 'LEVEL $out'})


    n.rule('custom_effect', "#{config.cmdl_converter} -o$out #{config.cmdl_converter_dep_opts} $in",
           {:dependencies => :gcc, :description => 'CUSTOM_EFFECT $out'})

    n.rule('skeletal_animation', "#{config.cmdl_converter} -o$out -lh #{config.cmdl_converter_dep_opts} $in",
           {:dependencies => :gcc, :description => 'SKELETAL_ANIMATION $out'})

    n.rule('behavior_tree',
           "$ruby #{config.behavior_tree_converter} #{config.script_dep_opt} -o $out $in",
           {:description => 'BEHAVIOR_TREE $out', :dependencies => :gcc})

    n.rule('xml_behavior_tree',
           "$ruby #{config.xml_behavior_tree_converter} #{config.script_dep_opt} -o $out $in",
           {:description => 'XML_BEHAVIOR_TREE $out', :dependencies => :gcc})

    n.rule('vpb',
           "$ruby #{config.vpb_converter} #{config.vpb_converter_opts} #{config.script_dep_opt} $require_all_pb_opt -o $out $in",
           {:description => 'VPB $out', :dependencies => :gcc})

  end

  def GeneratorUtil.define_pc_rules(config, platform, n)
    n.rule('msbuild',
           "#{config.msbuild} $csproj /p:Configuration=$configuration,Platform=AnyCPU,OutputPath=$outpath /v:quiet /nologo",
           {:description => 'VS BUILD $out'})

    n.rule('cmdl',
           "#{config.cmdl_converter} -a16 -o$out -h$skel -sk$skanimdir -lh #{config.cmdl_converter_dep_opts} -h$skel $in",
           {:dependencies => :gcc, :description => 'CMDL $out $shdir $fxdir $vshbase $pshbase'})


    n.rule('font', "#{config.font_converter} $in $out_stub",
           {:description => 'FONT $out_stub'})

    n.rule('vulkanshader', "glslc $in -o $out -std=430 -DPLATFORM_PC -DUSE_VULKAN",
           {:description => 'SHADER $out_stub'})    

  end

  #####################################################################
  # Functions for Extracting Items from Files                         #
  #####################################################################

  def GeneratorUtil.tag_files_for_animation_file(input, output)
    extract_matches(input, /<animTag.*name="([^"]+)"/).map do |tag|
      ext         = File.extname(output)
      basename    = File.basename(output, ext)
      dirname     = File.dirname(output)

      File.join(dirname, "#{basename}_#{tag}#{ext}")
    end
  end

  #####################################################################
  # Functions for Creating Build Statements                           #
  #####################################################################

  def GeneratorUtil.nanopb_protocols(config, n)
    config.nanopb_protocols.each do |py|
      proto = py.sub('_pb2.py', '.proto')
      n.build('protoc_py', {py => [proto]}, {:variables => {'outdir' => config.usagi_dir}})
    end
  end

  def GeneratorUtil.usagi_protocols(config, n)
    config.usagi_protocols.each do |py|
      proto = py.sub('_pb2.py', '.proto')
      n.build('protoc_py', {py => [proto]}, {:variables => {'outdir' => config.usagi_dir}})
    end
  end

  def GeneratorUtil.lua_protobuf(config, n, proto_files)
    lua_ext = proto_files.map{|p| p.sub(/\.proto$/, '.lua')}
    lua_cpp = lua_ext.map{|p| File.join(config.protocol_output_dir(false), p + '.cpp')}
    lua_h = lua_ext.map{|p| File.join(config.protocol_output_dir(false), p + '.h')}
    lua_o = lua_ext.map{|p| p + '.o'}
    include_lua_h = lua_ext.map{|p| File.join(config.includes_output_dir(false), p + '.h')}

    proto_files.zip(lua_h, lua_cpp, lua_o, include_lua_h).each do |proto, h, cpp, o, include_h|
      FileUtils.mkdir_p(File.dirname(cpp))
      n.build('protoc_lua', {cpp => [proto]}, protoc_options(config, config.protocol_output_dir(false), config.nanopb_lua_generator))
      n.alias(h, cpp)

      FileUtils.mkdir_p(File.dirname(include_h))
      n.build('cp', {include_h => [h]})
    end

    lua_h
  end

  def GeneratorUtil.ruby_protobuf(config, n, proto_files)
    pb_rb = proto_files.map{|p| File.join(config.protocol_ruby_output_dir(false), p.sub(/\.proto$/, '.pb.rb'))}

    proto_files.zip(pb_rb).each do |proto, rb|
      FileUtils.mkdir_p(File.dirname rb)
      n.build('protoc_rb', {rb =>[proto]}, protoc_options(config, config.protocol_ruby_output_dir(false)))
    end

    pb_rb
  end

  def GeneratorUtil.csharp_protobuf(config, n, proto_files, dll = nil)
    pb_cs = proto_files.map{|p| File.join(config.protocol_csharp_output_dir(false), p.sub(/\.proto$/, '.pb.cs'))}

    proto_files.zip(pb_cs).each do |proto, cs|
      FileUtils.mkdir_p(File.dirname cs)

      inputs = [proto]
      opts = protoc_options(config, config.protocol_csharp_output_dir(false))
      vars = opts[:variables]
      vars[:inproto] = proto
      if (dll != nil)
        vars[:dll] = to_windows_path(dll)
        inputs.push(dll)
      end
      opts[:variables] = vars

      n.build('protoc_cs', {cs => inputs}, opts)
    end

    pb_cs
  end

  def GeneratorUtil.python_protobuf(config, n, proto_files)
    pb_py = proto_files.map{|p| File.join(config.tools_python_dir(false), p.sub(/\.proto$/, '_pb2.py'))} 

    proto_files.zip(pb_py).each do |proto, py|
      pydir = File.dirname(py)
      FileUtils.mkdir_p(pydir)

      while pydir != config.tools_python_dir(false)
        File.open(pydir + "/__init__.py", "w").close()
        pydir = File.dirname( pydir )
      end

      n.build('protoc_py', {py =>[proto]}, protoc_options(config, config.tools_python_dir(false)))
    end

    pb_py
  end

  def GeneratorUtil.cpp_protobuf(config, n, proto_files)
    pb_ext = proto_files.map{|p| p.sub(/\.proto$/, '.pb')}
    pb_cpp = pb_ext.map{|p| File.join(config.protocol_output_dir(false), p + '.cpp')}
    pb_h = pb_ext.map{|p| File.join(config.protocol_output_dir(false), p + '.h')}
    pb_o = pb_ext.map{|p| p + '.o'}
    include_pb_h = pb_ext.map{|p| File.join(config.includes_output_dir(false), p + '.h')}

    proto_files.zip(pb_h, pb_cpp, pb_o, include_pb_h).each do |proto, h, cpp, o, include_h|
      FileUtils.mkdir_p(File.dirname(cpp))
      n.build('protoc_cpp', {cpp => [proto]}, protoc_options(config, config.protocol_output_dir(false), config.nanopb_generator))
      n.alias(h, cpp)

      FileUtils.mkdir_p(File.dirname(include_h))
      n.build('cp', {include_h => [h]})
    end

    pb_h
  end

  def GeneratorUtil.compile_cpp_protobuf(config, n, targets, order_only_deps)
    targets.each do |input, output|
      FileUtils.mkdir_p(File.dirname(output))
      n.build('cxx', {output => [input]}, {:order_only_deps => order_only_deps})
    end
  end

  def GeneratorUtil.compile_source(config, n, targets, order_only_deps, pch_file=nil)
    parameters = {:order_only_deps => order_only_deps}

    if !pch_file.nil?
      parameters[:variables] = {'use_pch' => "$pchflags --use_pch=#{pch_file}"}
      parameters[:implicit_deps] = [pch_file]
    end

    targets.each do |input, output|
      FileUtils.mkdir_p(File.dirname(output))
      n.build(config.compiler(input), {output => [input]}, parameters)
    end
  end

  def GeneratorUtil.copy_files(n, targets)
    targets.each do |input, output|
      FileUtils.mkdir_p(File.dirname(output))
      n.build('cp', {output => [input]})
    end
  end

  def GeneratorUtil.create_build_id(config, n, output = config.build_id_header, prefix = nil)
    git_head_file = '.git/HEAD'
    dependencies = [config.build_id_generator]
    dependencies << git_head_file if File.exists? git_head_file
    parameters = {:implicit_deps => dependencies}
    parameters[:variables] = {:prefix_option => "-p #{prefix}"} if !prefix.nil?

    n.build('build_id', {output => []}, parameters)
    n.alias('id', output)
  end

  def GeneratorUtil.create_data_list(n, targets, dependencies)
    targets.each do |input, output|
      FileUtils.mkdir_p(File.dirname(output))
      n.build('data_list', {output => [input]}, {:implicit_deps => dependencies})
    end
  end

  def GeneratorUtil.create_hash_list(config, n)
    FileUtils.mkdir_p(File.dirname(config.name_data_hash_list))
    n.build('name_data_hash_list',
            {config.name_data_hash_list => [config.data_list_file]},
            {:implicit_deps => [config.name_data_hash_list_tool]})
    n.alias('data', config.name_data_hash_list)
  end

  def GeneratorUtil.process_entities(config, n, targets, order_only_deps)
    targets.each do |input, output|
      FileUtils.mkdir_p(File.dirname(output))
      n.build('hierarchy', {output => [input]},
              {:order_only_deps => order_only_deps,
                :implicit_deps => [config.entity_converter]})
    end
  end

  def GeneratorUtil.expand_entities(config, n, targets, order_only_deps)
    targets.each do |input, output|
      FileUtils.mkdir_p(File.dirname(output))
      n.build('expandhierarchy', {output => [input]},
              {:order_only_deps => order_only_deps,
                :implicit_deps => [config.entity_converter]})
    end
  end

  def GeneratorUtil.process_positions(config, n, targets, order_only_deps)
    targets.each do |input, output|
      FileUtils.mkdir_p(File.dirname(output))
      n.build('position', {output => [input]},
              {:order_only_deps => order_only_deps,
                :implicit_deps => [config.maya_data_converter]})
    end
  end

  def GeneratorUtil.create_intermediate_level_files(config, n, targets)
    # note: outputs is actually an array
    targets.each do |input, outputs|
      outputs.each {|o| FileUtils.mkdir_p(File.dirname(o))}
      # join outputs as a hack for ninja-gen
      n.build('level', {outputs.join(' ') => [input]},
              {:implicit_deps => [config.level_converter]})
    end
  end

  def GeneratorUtil.process_model_data(config, n, targets, rule)
    rules = ['skeletal_animation', 'collision'] 
    raise "Error! Unknown rule: #{rule}" if ! rules.include? rule

    targets.each do |input, output|
      FileUtils.mkdir_p(File.dirname(output))
      n.build(rule, {output => [input]},
              {:implicit_deps => [config.cmdl_converter]})
    end
  end

  def GeneratorUtil.process_behavior_tree_data(config, n, targets, rule, order_only_deps)
    rules = ['behavior_tree', 'xml_behavior_tree']
    raise "Error! Unknown rule: #{rule}" if ! rules.include? rule

    tool = config.behavior_tree_converter

    if rule == 'xml_behavior_tree'
      tool = config.xml_behavior_tree_converter
    end

    targets.each do |input, output|
      FileUtils.mkdir_p(File.dirname(output))
      n.build(rule, {output => [input]},
              {:order_only_deps => order_only_deps, :implicit_deps => [tool]})
    end
  end


  def GeneratorUtil.pak_data(config, n, targets)
    targets.each do |input, output, implicit_deps|
      if implicit_deps.any?
        FileUtils.mkdir_p(File.dirname(output))
        n.build('resource_packer', {output => [input]}, {:implicit_deps => [config.resource_packer] + implicit_deps})
      end
    end
  end

  def GeneratorUtil.get_msbuild_targetpath(project_file, configuration, assembly_name)
    outpath_base = project_file.match("(.*\/)")[0].chomp('/')
    outfile = "#{outpath_base}/bin/#{configuration}/#{assembly_name}.dll"

    outfile
  end

  def GeneratorUtil.invoke_msbuild(config, n, project_file, additional_inputs, configuration, assembly_name)
    inputs = additional_inputs
    inputs.push(project_file)

    outfile = get_msbuild_targetpath(project_file, configuration, assembly_name)

    n.build('msbuild', { outfile => [inputs] }, {:variables => {
      'configuration' => configuration,
      'csproj' => project_file,
      'outpath' => to_windows_path("bin/#{configuration}")
    }})

    outfile
  end

  def GeneratorUtil.create_project_file(config, n, project_file, sources, options,
                                        implicit_deps = [], order_only_deps = [],
                                        should_fail_on_update = false, platform_sources = FileList[], project_id = "USAGI")
    template = options[:template]
    rootdir = options[:rootdir]
    guid = options.fetch(:guid, nil)
    projectname = project_id
    # this 'failure' is here so visual studio prompts the user to reload
    # the solution and any changed projects.    
    failure = ''

    if should_fail_on_update || ENV.has_key?('VISUAL_STUDIO_BUILD')
      failure = '-f'
    end

    implicit = [config.vs_project_generator, template] + implicit_deps
    order_only = [] + order_only_deps

    rule_options = {
      :variables => {'rootdir' => rootdir, 'template' => template, 'fail_opt' => failure},
      :implicit_deps => implicit
    }

    if options.has_key?(:guid)
      rule_options[:variables].store('guid', "-g '#{options[:guid]}'")
    end

    rule_options[:order_only_deps] = order_only if ! order_only.empty?
    FileUtils.mkdir_p(File.dirname project_file)
    n.build('project', {project_file => [sources]}, rule_options)
  end

  def GeneratorUtil.copy_boilerplate_to_vs_build_dir(config, n, source_file, source_list)
    # TODO: Update this if we ever get a command-line NX build
    is_nx_emulator_build = config.target_platform == 'nxe'
    is_nx_build = config.target_platform == 'nx'
    is_windows_build = config.target_platform == 'win'

    if is_nx_build
      vs_copy = source_file.sub("\/#{config.target_platform}\/", "/NX64/")
      source_list << vs_copy
      n.build('cp', {vs_copy => [source_file]})
    elsif is_windows_build || is_nx_emulator_build
      # 'Win32', no longer supported
      ['x64'].each do |platform|
        vs_copy = source_file.sub("\/#{config.target_platform}\/", "/#{platform}/")
     #   vs_copy = vs_copy.sub("\/debug\/", '/NXEmu_Debug/') if config.target_platform == 'nxe'
        source_list << vs_copy
        n.build('cp', {vs_copy => [source_file]})
      end
    end
  end

  def GeneratorUtil.process_system_sources(config, n, system_sources, order_only_deps,
                                           win_bp_cpp, existing_pch_file=nil)
    path_regex = Regexp.new(/Engine\/([^\/]+)\//)

    system_objects = system_sources.map do |src|
      src = src.gsub(/\\/, "/")
      yml = "#{config.code_working_dir}/#{src + ".yml"}"
      cpp = "#{config.code_working_dir}/#{src + ".bp.cpp"}"
      o = cpp.sub(/\.cpp$/, ".o")

      FileUtils.mkdir_p(File.dirname yml)
      n.build('boilerplate', {yml => [src]},
              {:implicit_deps => [config.boilerplate_scan, config.boilerplate_tool],
              :order_only_deps => order_only_deps})
      FileUtils.mkdir_p(File.dirname cpp)
      n.build('boilerplate_template', {cpp => [config.system_boilerplate_template]},
              {:implicit_deps => [yml], :variables => {'yml' => yml}})

      GeneratorUtil.copy_boilerplate_to_vs_build_dir(config, n, cpp, win_bp_cpp)

      if src =~ /\.h/
#        if config.target_platform == 'ctr'
#          pch_file = nil

#          if existing_pch_file != nil
#            pch_file = existing_pch_file
#          else
#            match = path_regex.match(cpp)

#            if match != nil
#              source_root = "Engine/#{match[1]}"
#              candidate = config.pch_file(source_root, match[1])

#              if File.exist?(config.pch_source_file(source_root))
#                pch_file = candidate
#              end
#            end
#          end

#          GeneratorUtil.compile_source(config, n, {cpp => o}, order_only_deps, pch_file)
#        end

        o
      end
    end.select{ |o| o != nil }
  end
end # closes GeneratorUtil module
