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
GIT_COMMIT = `git rev-parse --short HEAD`.strip

if !options[:destination].empty?
  FileUtils.mkdir_p options[:destination]
  FileUtils.mkdir_p File.join(options[:destination], 'icons')
  FileUtils.cp_r 'images', options[:destination]
  FileUtils.cp_r Dir.glob('../icons/png/*.png'), File.join(options[:destination], 'icons')

  Dir.glob("html_*/") do |lang|
    FileUtils.mkdir_p File.join(options[:destination], lang)
    FileUtils.cp_r Dir.glob([lang, '*.css'].join('/')), [options[:destination], lang].join('/')

    Dir.glob([lang, "*.html"].join('/')) do |filename|
        text = File.read(filename)

        # replace QT resouce path with web ones
        text.gsub!(%r{qrc:/}, "../icons/")

        # replace PNG files with SVG when available
        text.gsub!(%r{(src=")(images/[^.]*)\.png(")}) {|m| File.file?("#{$2}.svg") ? "#{$1}#{$2}.svg#{$3}" : m }

        # remove js that removes qrc links (since it's done here)
        text.gsub!(%r{ onLoad="replaceqrc\(\)"}, "")
        text.gsub!(%r{<SCRIPT SRC="utility.js" TYPE="text/javascript"></SCRIPT>\r?\n}i, "")

        # add nice header
        if !filename.include? "index.html"
        text.gsub!(%r{<BODY>}i, "<BODY>\n<H1 style=\"background-color: lightgreen;padding:3pt\"><img src=\"../icons/qlcplus.png\" width=32 align=\"absmiddle\" /> Q Light Controller Plus - User Documentation</H1>\n<a href=\"index.html\">Index page</a>")
        else
        text.gsub!(%r{<H1>}i, "<H1 style=\"background-color: lightgreen;padding:3pt\"><img src=\"../icons/qlcplus.png\" width=32 align=\"absmiddle\" /> ")
        end

        # add nice footer
        text.gsub!(%r{</BODY>}i, "<HR />\nVersion: #{VERSION} (#{GIT_COMMIT}) Last update: #{Time.now}\r\n</BODY>")

        File.open(File.join(options[:destination], filename), 'w') do |f|
        f << text
        end
    end
  end
end
