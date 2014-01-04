#!/usr/bin/env ruby

# Script to automatically update the fixtures map.
# Run it every time some fixtures are added to this folder.
# The packages 'ruby' and 'ruby-libxml' are required to run this

# When invoked with addtional parameter (filename), it will create HTML overview of the fixtures
# e.g. ./update-fixtures-map.rb index.html

require 'libxml'

class FixtureDef
  class Creator 
    attr_accessor :name, :version, :author
    def initialize(node)
      return if node.empty?
      @name = node.find_first('Name').content 
      @version = node.find_first('Version').content 
      @author = node.find_first('Author').content 
    end
  end

  class Capability
    attr_accessor :min, :max, :name, :color, :colour2
    def initialize(node)
      return if node.empty?
      @min = node.attributes['Min'] 
      @max = node.attributes['Max'] 
      @color = node.attributes['Color'] 
      @color2 = node.attributes['Color2'] 
      @name = node.content 
    end
  end

  class Group
    attr_accessor :byte, :name
    def initialize(node)
      return if node.empty?
      @name = node.content 
      @byte = node.attributes['Byte'] 
    end
  end

  class Channel
    attr_accessor :name, :group, :color, :capabilities
    def initialize(node)
      return if node.empty?
      @name = node.attributes['Name'] 
      @group = Group.new(node.find_first('Group'))
      n = node.find_first('Colour')
      @color = n.content unless n.nil?
      n = node.find('Capability')
      @capabilities = n.nil? ? [] : n.map {|c| Capability.new(c) }
    end
  end

  class Bulb
    attr_accessor :lumens, :type, :color_temp
    def initialize(node)
      return if node.empty?
      @lumens = node.attributes['Lumens']
      @type = node.attributes['Type']
      @color_temp = node.attributes['ColourTemperature']
    end
  end

  class Dimensions
    attr_accessor :width, :height, :depth, :weight
    def initialize(node)
      return if node.empty?
      @width = node.attributes['Width']
      @height = node.attributes['Height']
      @depth = node.attributes['Depth']
      @weight = node.attributes['Weight']
    end
  end

  class Lens
    attr_accessor :degrees_min, :degrees_max, :name
    def initialize(node)
      return if node.empty?
      @degrees_min = node.attributes['DegreesMin']
      @degrees_max = node.attributes['DegreesMax']
      @name = node.attributes['Name']
    end
  end

  class Focus
    attr_accessor :pan_max, :tilt_max, :type
    def initialize(node)
      return if node.empty?
      @pan_max = node.attributes['PanMax']
      @tilt_max = node.attributes['TiltMax']
      @type = node.attributes['Type']
    end
  end

  class Technical
    attr_accessor :power_consumption, :dmx_connector
    def initialize(node)
      return if node.nil?
      return if node.empty?
      @power_consumption = node.attributes['PowerConsumption']
      @dmx_connector = node.attributes['DmxConnector'] 
    end
  end

  class Physical
    attr_accessor :bulb, :dimensions, :lens, :focus, :technical
    def initialize(node)
      return if node.empty?
      @bulb = Bulb.new(node.find_first('Bulb'))
      @dimensions = Dimensions.new(node.find_first('Dimensions'))
      @lens = Lens.new(node.find_first('Lens'))
      @focus = Focus.new(node.find_first('Focus'))
      @technical = Technical.new(node.find_first('Technical'))
    end
  end

  class ChannelRef
    attr_accessor :number, :name
    def initialize(node)
      return if node.empty?
      @name = node.content 
      @number = node.attributes['Number'] 
    end
  end
  
  class Head
    attr_accessor :channels
    def initialize(node)
      return if node.empty?
      @channels = node.find("Channel").map {|c| c.content.to_i }
    end
  end

  class Mode
    attr_accessor :name, :physical, :channels, :heads
    def initialize(node)
      @channels = []
      @heads = []
      return if node.empty?
      @name = node.attributes['Name']
      @physical = Physical.new(node.find_first('Physical'))
      n = node.find('Channel')
      @channels = n.empty? ? [] : n.map {|c| ChannelRef.new(c) }
      n = node.find('Head')
      @heads = n.empty? ? [] : n.map {|h| Head.new(h) }
    end
  end

  attr_accessor :path, :manufacturer, :model, :type, :creator, :channels, :modes
  
  def initialize(path)
    load(path)
  end

  def load(path)
    doc = LibXML::XML::Document.file(path)
    @path = path
    @manufacturer = doc.find_first('/FixtureDefinition/Manufacturer').content
    @model = doc.find_first('/FixtureDefinition/Model').content
    @type = doc.find_first('/FixtureDefinition/Type').content
    @creator = Creator.new(doc.find_first('/FixtureDefinition/Creator'))
    @channels = doc.find('/FixtureDefinition/Channel').map {|c| Channel.new(c) }
    @modes = doc.find('/FixtureDefinition/Mode').map {|m| Mode.new(m) }
  end
end

class Fixtures
  attr_reader :fixtures

  def initialize()
    @fixtures = []
  end

  def load_fixtures(path)
    Dir.chdir(path) do
      Dir.glob('*.qxf') do |f|
        @fixtures << FixtureDef.new(f)
      end
    end
  end

  def update_fixtures_map(filename = 'FixturesMap.xml')
    doc = LibXML::XML::Document.file(filename)
    doc.root.children.map(&:remove!)
    @fixtures.sort_by {|f| f.path.downcase }.each do |f|
      node = LibXML::XML::Node.new('fixture')
      LibXML::XML::Attr.new(node, 'path', f.path)
      LibXML::XML::Attr.new(node, 'mf', f.manufacturer)
      LibXML::XML::Attr.new(node, 'md', f.model)
      doc.root << node
    end
    doc.save(filename, :indent => true, :encoding => LibXML::XML::Encoding::UTF_8)
  end

  def make_overview(filename = 'index.html')
    File.open(filename, 'w') do |f|
      f << <<-EOF
<html>
<head>
</head>
<body>
  <table border=1>
    <tr>
      <th rowspan=3>Manuf</th>
      <th rowspan=3>Model</th>
      <th rowspan=3>Version</th>
      <th rowspan=3>Author</th>
      <th rowspan=3>Mode</th>
      <th rowspan=3>Heads</th>
      <th colspan=15>Physical</th>
    </tr>
    <tr>
      <th colspan=3>Bulb</th>
      <th colspan=4>Dim</th>
      <th colspan=3>Lens</th>
      <th colspan=3>Focus</th>
      <th colspan=2>Tech</th>
    </tr>
    <tr>
      <th>Lm</th>
      <th>Type</th>
      <th>ColTemp</th>
      <th>Wid</th>
      <th>Hei</th>
      <th>Dep</th>
      <th>Wei</th>
      <th>Min</th>
      <th>Max</th>
      <th>Name</th>
      <th>Pan</th>
      <th>Tilt</th>
      <th>Type</th>
      <th>Power</th>
      <th>Conn</th>
    </tr>
EOF
      @fixtures.sort_by {|f| f.path.downcase }.each do |fix|
        f << <<-EOF
    <tr>
      <td rowspan=#{fix.modes.size}>#{fix.manufacturer}</td>
      <td rowspan=#{fix.modes.size}>#{fix.model}</td>
      <td rowspan=#{fix.modes.size}>#{fix.creator.version}</td>
      <td rowspan=#{fix.modes.size}>#{fix.creator.author}</td>

EOF
        fix.modes.each_with_index do |m, i|
        if i > 0
          f << "</tr></tr>"
        end
          f << <<-EOF
      <td>#{m.name}</td>
      <td>#{m.heads.size}</td>
      <td>#{m.physical.bulb.lumens}</td>
      <td>#{m.physical.bulb.type}</td>
      <td>#{m.physical.bulb.color_temp}</td>
      <td>#{m.physical.dimensions.width}</td>
      <td>#{m.physical.dimensions.height}</td>
      <td>#{m.physical.dimensions.depth}</td>
      <td>#{m.physical.dimensions.weight}</td>
      <td>#{m.physical.lens.degrees_min}</td>
      <td>#{m.physical.lens.degrees_max}</td>
      <td>#{m.physical.lens.name}</td>
      <td>#{m.physical.focus.pan_max}</td>
      <td>#{m.physical.focus.tilt_max}</td>
      <td>#{m.physical.focus.type}</td>
      <td>#{m.physical.technical.power_consumption}</td>
      <td>#{m.physical.technical.dmx_connector}</td>
    </tr>

EOF
        end
      end

      f << <<-EOF
</table>
</body>
</html>
EOF
    end
  end
end

fm = Fixtures.new
fm.load_fixtures('.')
fm.update_fixtures_map

if ARGV.size > 0
  fm.make_overview(ARGV[0])
end
