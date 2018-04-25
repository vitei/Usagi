# Usagi Engine, Copyright © Vitei, Inc. 2013
#!/usr/bin/env ruby
# Script to generate an RSF file for use with ctr_makerom

require 'erb'
require 'optparse'

##################
# option parsing #
##################

options = {:is_dlp_child => false, :is_final_build => false,
  :media_size => 512, :stack_size => 32768}

option_parser = OptionParser.new do |opts|
  opts.banner = "Usage: generate_rsf.rb -p code -u ID template_file outputs_file"
  opts.on('-p', '--product-code CODE', 'Product code') do |code|
    options[:product_code] = code
  end

  opts.on('-u', '--unique-id ID', 'Unique ID') do |id|
    options[:unique_id] = id
  end

  opts.on('-m', '--media-size SIZE', OptionParser::DecimalInteger) do |size|
    options[:media_size] = size
  end

  opts.on('-s', '--stack-size SIZE', OptionParser::DecimalInteger) do |size|
    options[:stack_size] = size
  end

  opts.on('-d', '--dlp-child', 'Build RSF file for DLP child') do
    options[:is_dlp_child] = true
  end

  opts.on('-f', '--final', 'Build RSF file for final build') do
    options[:is_final_build] = true
  end
end

option_parser.parse!

if ARGV.length < 2
  STDERR.puts "ERROR! The template file and output file must be specified."
  exit
end

[:product_code, :unique_id].each do |opt|
  next if options.has_key?(opt)

  STDERR.puts "ERROR! Missing option. Please specify all options!"
  STDERR.puts option_parser
  exit
end

##################
# main body      #
##################
TEMPLATE = ARGV[0]
OUTPUT_FILE = ARGV[1]

PRODUCT_CODE = options[:product_code]
UNIQUE_ID = options[:unique_id]
CATEGORY = options[:is_dlp_child] ? 'DlpChild' : 'Application'
MEDIA_SIZE = "#{options[:media_size]}MB"
STACK_SIZE = options[:stack_size]

IS_DLP_CHILD = options[:is_dlp_child]
IS_FINAL_BUILD = options[:is_final_build]

output = ERB.new(File.read(TEMPLATE), nil, '-').result(binding())
outfile = File.open(OUTPUT_FILE, 'w')
outfile << output
outfile.close
