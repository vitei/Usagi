
BUILDS          = (ENV["build"]    || "debug").split(",")
TARGET_PLATFORM = ENV["platform"]  || DEFAULT_PLATFORM
REGION          = ENV["region"]    || "JP"
VERBOSITY       = ENV["verbosity"] || "normal"
CORES           = ENV["cores"]

PYTHON = "python3"
RUBY = "ruby"

SKIP_USAGI_BUILD = ["cia"]

raise "USAGI_DIR environment variable not defined!" if ENV['USAGI_DIR'].nil?


if VERBOSITY == "verbose"
  verbose(true)
  VERBOSITY_FLAG="-v"
else
  verbose(false)
  VERBOSITY_FLAG=""
end

def build_dir(build_name)
  File.join("_build", TARGET_PLATFORM, build_name)
end

def ninja_file(build_name)
  File.join(build_dir(build_name), "build.ninja")
end

def protobuf_depfile(use_engine_dir=false)
  depfile = "_build/proto/deps.txt"
  depfile = "#{SUBMODULE_DIR}/#{depfile}" if use_engine_dir
  dir = File.dirname depfile
  FileUtils.mkdir_p dir if ! File.directory? dir

  depfile
end

$usagi_ninja_command_succeeded = false

def ninja(build_name, failed_job_limit=10)
  "#{NINJA_PREFIX}ninja #{VERBOSITY_FLAG} -w dupbuild=err -f #{ninja_file(build_name)} -k #{failed_job_limit} #{CORES ? "-j #{CORES}" : ""}"
end

def build_audio()
  files = []
  mono = ''
  mono = 'mono' if ! Gem.win_platform?
  FileUtils.mkdir_p("#{PROJECT}/audio_gen")
  FileList["Data/VPB/Audio/*.yml"].select{|f| !File.directory? f}.map do |input|
    output = "#{PROJECT}/#{input.sub(/^Data\/VPB\/Audio\//, "audio_gen\/")}"
    output = output.sub(".yml", ".proto")
    name = File.basename(input).sub(".yml", "") 
    name_upper = name.upcase
    vitei_audio_tool = "#{mono} #{ENV['USAGI_DIR']}/Tools/AudioTool/FSIDBuilder.exe"
    cmdline = vitei_audio_tool + ' --proto -i="' + input + '" -o=' + output + ' -e=' + name + 'Audio -g=_CLR_' + name_upper + '_FSID_'
    puts cmdline
    sh cmdline
    files.push(output)
  end
  puts files
  files
end

task :default => :build
task :build => :ninja do |t|
  BUILDS.each{ |b| sh ninja(b) }
end


task :data => :ninja do
  BUILDS.each{ |b| sh "#{ninja b} data" }
end

task :libs => :ninja do
  BUILDS.each{ |b| sh "#{ninja b} libs" }
end

task :id => :ninja do
  BUILDS.each{ |b| sh "#{ninja b} id" }
end

task :includes => :ninja do
  BUILDS.each{ |b| sh "#{ninja b} includes" }
end

task :pb => :ninja do
  BUILDS.each{ |b| sh "#{ninja b} pb" }
end

task :projects => :ninja do
  # the second parameter to the ninja function should be
  # greater than the total number of project files we generate
  BUILDS.each do |b|
    game_ninja_command_succeeded = false

    sh "#{ninja(b, 30)} projects" do |finished, status|
      game_ninja_command_succeeded = finished
    end

    if ENV.has_key?("VISUAL_STUDIO_BUILD") && !($usagi_ninja_command_succeeded && game_ninja_command_succeeded)
      raise "Project files updated!"
    end
  end
end

task :docs do
  event_docs = `find #{PROJECT} Usagi/Engine -name '*.proto' | grep -v ThirdParty/nanopb | xargs python3 Usagi/Tools/python/lua_docs_generator.py`
  File.open('event_docs.txt', 'w') { |f| f.write event_docs }
end

task :clean do |t|
  BUILDS.each{ |b| sh "#{ninja b} -t clean" }
end

task :clean_bp do |t|
  BUILDS.map{|b| build_dir b}.each do |dir|
    bp_files = FileList["#{dir}/**/*.bp.cpp"]

    if TARGET_PLATFORM == 'win'
       bp_files.include "#{dir.to_s.sub(Regexp.quote('/win/'), ENV.fetch("VISUAL_STUDIO_PLATFORM","x64") )}/**/*.bp.cpp"
    end

   FileUtils.rm bp_files
  end
end

task :clean_code do |t|
  BUILDS.map{|b| build_dir b}.each do |dir|
    code_files = FileList["#{dir}/**/*.o"]
    code_files.exclude("#{dir}/Data/**/*", "#{dir}/Effects/**/*")

    FileUtils.rm code_files
  end
end

task :clean_data do |t|
  FileUtils.rm_rf FileList["_romfiles/#{TARGET_PLATFORM}/**"]
end

task :clean_includes do |t|
  FileUtils.rm_rf FileList["_includes/**"]
end

task :clean_projects do
  FileUtils.rm_rf FileList["_build/projects/**"]
end
