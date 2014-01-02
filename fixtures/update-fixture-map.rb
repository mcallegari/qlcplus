#!/usr/bin/env ruby

# Script to automatically update the fixtures map.
# Run it every time some fixtures are added to this folder.
# The packages 'ruby' and 'ruby-libxml' are required to run this

require 'libxml'

class FixtureDef
  attr_accessor :path, :manufacturer, :model
  
  def initialize(path)
    load(path)
  end

  def load(path)
    doc = LibXML::XML::Document.file(path)
    @path = path
    @manufacturer = doc.find_first('/FixtureDefinition/Manufacturer').content
    @model = doc.find_first('/FixtureDefinition/Model').content
  end
end

class FixturesMap
  def initialize()
    @fixtures = []
  end

  def add_fixtures(path)
    Dir.chdir(path) do
      Dir.glob('*.qxf') do |f|
        @fixtures << FixtureDef.new(f)
      end
    end
  end

  def store(filename)
    doc = LibXML::XML::Document.file(filename)
    doc.root.children.map(&:remove!)
#    doc.root = LibXML::XML::Node.new('FixtureMap')
    @fixtures.sort_by {|f| f.path.downcase }.each do |f|
      node = LibXML::XML::Node.new('fixture')
      LibXML::XML::Attr.new(node, 'path', f.path)
      LibXML::XML::Attr.new(node, 'mf', f.manufacturer)
      LibXML::XML::Attr.new(node, 'md', f.model)
      doc.root << node
    end
    doc.save(filename, :indent => true, :encoding => LibXML::XML::Encoding::UTF_8)
  end
end

fm = FixturesMap.new
fm.add_fixtures('.')
fm.store('FixturesMap.xml')

