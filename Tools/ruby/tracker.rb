# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
# tracker.rb
# This library lets you track dependencies within our Tools/ruby
# directory and write them as files that can be used in Ninja build
# rules. We use it to make sure that ruby modules generated from
# protocol buffer files and ruby libraries we've written are included
# as build dependencies for data files output from ruby scripts.

require 'optparse'
require 'pathname'
require 'set'
require 'singleton'

module Tracker
  RUBY_TOOLS_PATH = 'Tools/ruby'
  RUBY_PROTOBUF_INCLUDE_DIR = '_build/ruby'
  RUBY_PROTOBUF_SUFFIX = '.pb'
  PROTOBUF_SUFFIX = '.proto'

  DEPFILE_OPTION_KEY = :depfile

  raise "ERROR! 'USAGI_DIR' not defined!" if ! ENV.has_key?('USAGI_DIR')
  USAGI_PREFIX, USAGI_DIR_PREFIX = Pathname.new(ENV['USAGI_DIR']).split

  class DepSet
    include Singleton

    # set of filenames
    attr_accessor :dependencies
    # hash that maps a protobuf message / enum to the .proto file
    # where it is defined
    attr_accessor :message_lookup

    def initialize()
      @dependencies = Set.new()
      @message_lookup = {}

      File.open('_build/proto/deps.txt', 'r').each do |line|
        object, file = line.chomp!.split('|')
        @message_lookup[object] = file
      end
    end

    def addDep(item, shouldLookupProtoFile = false)
      if ! shouldLookupProtoFile
        @dependencies.add(item)
        return
      end

      if ! @message_lookup.has_key? item
        return
      end

      @dependencies.add(@message_lookup[item])
    end

    def createDepfile(dep_file, output_file)
      ruby_tools_path = Pathname.new(ENV['USAGI_DIR']) + Pathname.new(RUBY_TOOLS_PATH)
      ruby_protobuf_include_path = Pathname.new(RUBY_PROTOBUF_INCLUDE_DIR)

      dep_file_path = Pathname.new(dep_file)
      dep_file_path.parent.mkpath
      out = dep_file_path.open('w')
      out.puts "#{output_file}: \\\n"

      @dependencies.each do |dep|
        isProtobufLibrary = Tracker.isProtobufFile?(dep)
        dep_path = Pathname.new(dep)

        if dep.end_with?(PROTOBUF_SUFFIX) || dep.end_with?(RUBY_PROTOBUF_SUFFIX)
          if dep.start_with?(USAGI_DIR_PREFIX.to_path)
            dep_path = USAGI_DIR_PREFIX + ruby_protobuf_include_path + dep_path.relative_path_from(USAGI_DIR_PREFIX)
          else
            dep_path = ruby_protobuf_include_path + dep_path
          end

          dep_path = dep_path.sub_ext("#{RUBY_PROTOBUF_SUFFIX}.rb")
        elsif File.exists?(ruby_tools_path + dep_path.sub_ext('.rb'))
          dep_path = ruby_tools_path + dep_path.sub_ext('.rb')
        end

        next if ! dep_path.exist?

        out.puts "  #{dep_path} \\\n"
      end

      out.puts "  "
      out.close
    end
  end # DepSet class

  def Tracker.isProtobufFile?(path)
     return path.end_with?(RUBY_PROTOBUF_SUFFIX)
  end

  def Tracker.addDepfileOption(options, optionParser)
    optionParser.on('--MF file', 'Write dependencies file for Ninja') do |f|
      options[DEPFILE_OPTION_KEY] = f
    end
  end

  def Tracker.writeDependenciesFile(options, scriptOutputFile)
    return if ! options.has_key?(DEPFILE_OPTION_KEY)

    DepSet.instance.createDepfile(options[DEPFILE_OPTION_KEY], scriptOutputFile)
  end
end # Tracker module

module Kernel
  # define aliases to the original versions of require_relative and require
  alias_method :original_require_relative, :require_relative
  alias_method :original_require, :require

  # then, rewrite these two methods
  def require_relative(relative_feature)
    Tracker::DepSet.instance.addDep(relative_feature)

    return original_require_relative(relative_feature)
  end

  def require(relative_feature)
    normalizedRelativeFeature = relative_feature.sub(/.rb$/, '')
    isProtobufFile = Tracker.isProtobufFile?(normalizedRelativeFeature)
    isNotNanopbFile = normalizedRelativeFeature != 'nanopb.pb'

    if isProtobufFile && isNotNanopbFile
      Tracker::DepSet.instance.addDep(normalizedRelativeFeature)
    end

    return original_require(relative_feature)
  end
end
