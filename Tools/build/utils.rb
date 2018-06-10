# Usagi Engine, Copyright (c) Vitei, Inc. 2013
require 'pathname'
require 'rake'
require 'rexml/document'
require 'yaml'



def get_platform_sourcelist(dir, target_platform, base_dir, underscore_dirs_whitelist, should_find_headers = false)

  platSrc = FileList[]

  base_path  = dir.sub("#{base_dir}", '')
  path = Pathname.new(dir)
  relative_path = path.relative_path_from(Pathname(base_dir))

  underscore_dirs_whitelist.each do |ext|

    type = ext.gsub('_', '')
    platformList = "Platform/#{type}/#{relative_path}"
    platSrc.include(get_sourcelist(platformList, target_platform, underscore_dirs_whitelist, should_find_headers))

  end

  platSrc
end

def get_sourcelist(dir, target_platform, 
                   underscore_dirs_whitelist = false, should_find_headers = false)
  src = FileList[]
  sourceList = "#{dir}/.sources.#{target_platform}"

  if File.exists?(sourceList)
    # If a .sources file for the target platform exists, only include
    # files listed in it.
    File.open(sourceList, 'r') do |file|
      file.each_line.select{|e| !e.empty? && e.split(//).first != "#"}.each do |line|
        # check for exclusions
        if line.start_with?('-:')
          src.exclude("#{dir}/#{line.sub!(/^-:/, '').strip}")
        elsif should_find_headers
          header = "#{dir}/#{line.strip.gsub(/(\.cpp|\.proto|\.c)$/, '.h')}"
          # prevent addition of headers that don't exist when a glob isn't used
          src.include(header) if line.match(/\*/) || File.exists?(header)
        else
          src.include("#{dir}/#{line.strip}")
        end
      end
    end
  else
    # If no .sources file exists for the target platform, default to
    # including standard files and then filtering out any underscore
    # dirs not in the whitelist for the platform.
    if should_find_headers
      src.include("#{dir}/**/*.h")
    else
      src.include("#{dir}/**/*.cpp", "#{dir}/**/*.c",
                  "#{dir}/**/*.proto", "#{dir}/**/*.sys")
    end

    # This regex will filter out any paths which contain an underscore
    # directory which doesn't appear in the platform whitelist. For
    # example, if the whitelist contains '_win', '_ovr', and
    # '_fragment', it will filter out paths which contain '_ctr'.

    # ...of course, sometimes you might want to turn this off - e.g. when
    # you're fetching protocol buffers. In that case, pass in anything that
    # doesn't respond to :map - conveniently, a boolean value will be okay.
    if underscore_dirs_whitelist.respond_to?(:map)
      dir_alternation = underscore_dirs_whitelist.map{|s| s.gsub(/^_/, '')}.join('|')
      src.exclude(Regexp.new(/\/_(?!#{dir_alternation})[^_\/]+\//))
    end
  end

  # Exclude usagipb.proto from the sourcelist - it contains edit-time
  # protcol buffer metadata.
  src.exclude("**/Engine/Core/usagipb.*")

  src
end

def gen_source_targets(source_list, config, platform)
  pairs = source_list.flat_map do |s|
    ext = File.extname(s)
    out_ext = '.o'
    c = s
    extra = []

    if ext == '.proto'
      c = "#{config.protocol_output_dir(false)}/" + s.sub(/\.proto$/, '.pb.cpp')
      out_ext = '.pb.o'

      c_lua = "#{config.protocol_output_dir(false)}/" + s.sub(/\.proto$/, '.lua.cpp')
      o_lua = "#{config.code_working_dir}/#{s.sub(/\.[^.]*$/, '.lua.o')}"
      extra = [c_lua, o_lua]
    end

    o = "#{config.code_working_dir}/#{s.sub(/\.[^.]*$/, out_ext)}"

    extra.empty? ? [[c,o]] : [[c, o], extra]
  end

  Hash[pairs]
end

def filter_deps_by_rsf(romfiles_dir, specfile, deps)
  # Re-implement the Include/Reject functionality in Nintendo's .rsf files
  # for our dependencies.  The way we'd like these to work is as follows:
  # first filter out anything that matched the 'reject' patterns from the
  # list of files in romfiles, then from that filtered list they select
  # only those files matching the 'include' patterns for inclusion.
  # Unfortunately, Nintendo's tool doesn't work this way, so we have to
  # simulate it.
  rejects, includes = File.open(specfile, 'r') do |f|
    rom = YAML.load(f)["Rom"] || {}
    [rom["Reject"] || [], rom["Include"] || []]
  end

  # Patterns begin with either '/' or '>' for shell-style globbing and
  # regular expression patterns respectively.  If they start with any
  # other character, the pattern is invalid.
  matches = lambda do |pattern, filename|
    is_valid = ['/', '>'].include?(pattern[0])
    abort "Invalid Rejects/Includes pattern in #{specfile}: '#{pattern}'" if !is_valid

    relative_path  = filename.sub("#{romfiles_dir}/", '')
    pattern_type   = pattern[0] == '>' ? :regex : :glob
    search_pattern = pattern[1..-1]

    case pattern_type
    when :glob
      File.fnmatch(search_pattern, relative_path)
    when :regex
      /#{search_pattern}/.match("/#{relative_path}") != nil
    end
  end

  deps.reject { |f| rejects.any?  { |p| matches.call(p, f) } }
      .select { |f| includes.any? { |p| matches.call(p, f) } }
end

# build an options hash that is used when creating build statements
# for .proto files
def protoc_options(build_config, output_dir, *deps)
  {
    :variables => {'outdir' => output_dir},
    :implicit_deps => deps.concat(build_config.nanopb_protocols)
  }
end

def find_source(objfile, ext)
  if objfile.include? CODE_WORKING_DIR then
    objfile.sub(/#{CODE_WORKING_DIR}\/(.*)\..*/,'\1' + ext)
  else
    objfile.sub(/\..*/, ext)
  end
end

def clean_protocol_ruby_classes(build_config, should_use_relative=true)
  dir = build_config.protocol_ruby_output_dir(should_use_relative)

  if !File.directory? dir
    return
  end

  old_files = FileList["#{dir}/**/*.pb.rb"].exclude(*build_config.protocol_ruby_classes(should_use_relative))
  # hack for FSID files
  old_files.exclude("**/*FSID*")
  old_files.map{|f| File.delete f }
end

# preprocess paths that go to ninja with this as the last step - don't do
# it before, or it'll interfere with filelist etc.
def normalise_windows_paths(path)
  path.gsub('\\', '/').gsub(':/', '$:/') if path
end

def to_windows_path(path)
  path.gsub(/\//,"\\")
end

def cyg_fix(exe)
  if RbConfig::CONFIG['host_os'] =~ /cygwin/
    `cygpath -m #{exe}`.strip
  else
    exe
  end
end

def should_build_lib(lib, target_platform, underscore_dirs_whitelist)
  src = get_sourcelist("Engine/#{lib}", target_platform, underscore_dirs_whitelist)
  return !src.empty? || lib =~ /Protocols/
end


# generates a depfile stanza for an output file created within ROMFILES_DIR
def generate_depfile_definition(output_file)
  depfile = Pathname.new(WORKING_DIR) + Pathname.new(output_file).relative_path_from(Pathname.new(ROMFILES_DIR))

  return "  ninja_depfile = #{depfile}.d"
end

def write_file_list(filelist, dir, subdir, filename='.sources.txt')
  srclist = Pathname.new(dir).join(subdir, filename)
  FileUtils.mkdir_p(File.dirname srclist)

  if File.exists? srclist
    existing_list = File.readlines(srclist).map{|line| line.chomp}

    # Rewriting the output file will trigger the project generation
    # rule, so don't do it if its contents match the array we're going
    # to write
    if filelist == existing_list
      return
    end
  end

  File.open(srclist, 'w') do |f|
    filelist.map{|path| f.puts path}
  end
end

# Platform-specific grep
def grep(strings, directories, file_filters)
  if Gem.win_platform?
    matches = `findstr /D:#{directories.join(";")} /S /M #{strings.map{ |s| "\"#{s}\""}.join(" ")} #{file_filters.join(" ")}`
    grouped_matches = matches.each_line.inject([]) { |acc, l| l =~ /  ([^:]+):/ ? acc.push([$1, []]) : acc[0...-1] << [acc.last[0], acc.last[1] << l] }
    grouped_matches.map { |group, matches| matches.map{ |f| "#{group}\\#{f.strip}" } }.flatten
  else
    `grep -Rl #{file_filters.map{|f| "--include \"#{f}\""}.join(" ")} #{strings.map{|s| "-e \"#{s}\""}.join(" ")} #{directories.join(" ")}`.lines.map{|l|l.strip}
  end
end

# Utility to extract certain information from a file.
# By default it returns the first match variable, but you can pass
# a block to do more with it if you wish.  Return nil to indicate
# no match.
def extract_matches(file, regex)
  File.open(file, "r") do |f|
    f.each_line.flat_map{ |l| l.encode('UTF-8', 'binary', invalid: :replace, undef: :replace, replace: '')
        .match(regex) { |m| block_given? ? yield(m) : m[1] } || [] }
  end
end
