#!/usr/bin/env ruby

require 'optparse'
require 'fileutils'

options = {}

OptionParser.new do |opts|
  opts.banner = "Usage: make-docs.rb [options]"

  opts.on("-d", "--destination DIR", "Destination directory") do |d|
    options[:destination] = d
  end

  opts.on_tail("-h", "--help", "Show this message") do
    puts opts
    exit
  end
end.parse!

VERSION = (File.read('../../variables.pri') =~ /APPVERSION = (.*?)$/) ? $1 : "Unknown"

if !options[:destination].empty?
  FileUtils.mkdir_p options[:destination]
  FileUtils.mkdir_p File.join(options[:destination], 'gfx')
  FileUtils.cp_r Dir.glob('*.css'), options[:destination]
  FileUtils.cp_r 'images', options[:destination]
  FileUtils.cp_r Dir.glob('../icons/png/*.png'), File.join(options[:destination], 'gfx')

  Dir.glob("*.html") do |filename|
    text = File.read(filename)
    text.gsub!(%r{file:}, "")
    text.gsub!(%r{qrc:/}, "gfx/")
    text.gsub!(%r{ onLoad="replaceqrc\(\)"}, "")
    text.gsub!(%r{<SCRIPT SRC="utility.js" TYPE="text/javascript"></SCRIPT>\r?\n}, "")
    if !filename.include? "index.html"
      text.gsub!(%r{<BODY>}, "<BODY>\n<H1 style=\"background-color: lightgreen;padding:3pt\"><img src=\"gfx/qlcplus.png\" width=32 align=\"absmiddle\" /> Q Light Controller Plus - User Documentation</H1>\n<a href=\"index.html\">Index page</a>")
    end
    text.gsub!(%r{</BODY>}, "<HR />\nVersion: #{VERSION} Last update: #{Time.now}\r\n</BODY>")
    File.open(File.join(options[:destination], filename), 'w') do |f|
      f << text
    end
  end
end
