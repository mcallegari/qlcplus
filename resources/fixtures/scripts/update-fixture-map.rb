#!/usr/bin/env ruby

# Script to automatically update the fixtures map.
# Run it every time some fixtures are added to this folder.
# The packages 'ruby' and 'ruby-libxml' are required to run this

# When invoked with addtional parameter (filename), it will create
# HTML overview of the fixtures e.g. ./update-fixtures-map.rb index.html

# rubocop: disable Style/Documentation, Metrics/MethodLength
# rubocop: disable Metrics/ClassLength, Metrics/AbcSize
# rubocop: disable Metrics/CyclomaticComplexity, Metrics/PerceivedComplexity,
# Metrics/LineLength,

require 'libxml'

NS_URI = 'http://www.qlcplus.org/FixtureDefinition'
OLD_NS_URI = 'http://qlcplus.sourceforge.net/FixtureDefinition'

NS = ['xmlns:' + NS_URI]

class FixtureDef
  class Creator
    attr_accessor :name, :version, :author
    def initialize(node)
      return if node.empty?
      @name = node.find_first('xmlns:Name', NS).content
      @version = node.find_first('xmlns:Version', NS).content
      @author = node.find_first('xmlns:Author', NS).content
    end
  end

  class Capability
    attr_accessor :min, :max, :name, :res, :color, :color2
    def initialize(node)
      return if node.empty?
      @min = node.attributes['Min'].to_i
      @max = node.attributes['Max'].to_i
      @res = node.attributes['Res']
      @color = node.attributes['Color']
      @color2 = node.attributes['Color2']
      @name = node.content
    end
  end

  class Group
    attr_accessor :byte, :name

    MSB = 0
    LSB = 1

    def initialize(node)
      return if node.empty?
      @name = node.content
      @byte = node.attributes['Byte'].to_i
    end

    PAN = 'Pan'
    TILT = 'Tilt'
    COLOUR = 'Colour'
    EFFECT = 'Effect'
    GOBO = 'Gobo'
    SHUTTER = 'Shutter'
    SPEED = 'Speed'
    PRISM = 'Prism'
    MAINTENANCE = 'Maintenance'
    INTENSITY = 'Intensity'
    BEAM = 'Beam'

    def icon
      case @name
      when PAN
        'pan.png'
      when TILT
        'tilt.png'
      when COLOUR
        'colorwheel.png'
      when EFFECT
        'star.png'
      when GOBO
        'gobo.png'
      when SHUTTER
        'shutter.png'
      when SPEED
        'speed.png'
      when PRISM
        'prism.png'
      when MAINTENANCE
        'configure.png'
      when INTENSITY
        'intensity.png'
      when BEAM
        'beam.png'
      end
    end

    def byte_str
      case @group
      when MSB
        'MSB'
      when LSB
        'LSB'
      end
    end
  end

  class Channel
    attr_accessor :name, :group, :color, :capabilities
    def initialize(node)
      @capabilities = []
      return if node.empty?
      @name = node.attributes['Name']
      @group = Group.new(node.find_first('xmlns:Group', NS))
      n = node.find_first('xmlns:Colour', NS)
      @color = n.content unless n.nil?
      n = node.find('xmlns:Capability', NS)
      @capabilities = n.map { |c| Capability.new(c) } unless n.empty?
    end

    RED = 'Red'
    GREEN = 'Green'
    BLUE = 'Blue'
    CYAN = 'Cyan'
    MAGENTA = 'Magenta'
    YELLOW = 'Yellow'
    AMBER = 'Amber'
    WHITE = 'White'
    UV = 'UV'
    LIME = 'Lime'
    INDIGO = 'Indigo'

    def rgb_color
      case color
      when RED
        '#FF0000'
      when GREEN
        '#00FF00'
      when BLUE
        '#0000FF'
      when CYAN
        '#00FFFF'
      when MAGENTA
        '#FF00FF'
      when YELLOW
        '#FFFF00'
      when AMBER
        '#FF7E00'
      when WHITE
        '#FFFFFF'
      when UV
        '#9400D3'
      when LIME
        '#ADFF2F'
      when INDIGO
        '#4B0082'
      end
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
      @width = node.attributes['Width'].to_i
      @height = node.attributes['Height'].to_i
      @depth = node.attributes['Depth'].to_i
      @weight = node.attributes['Weight'].to_f
    end
  end

  class Lens
    attr_accessor :degrees_min, :degrees_max, :name
    def initialize(node)
      return if node.empty?
      @degrees_min = node.attributes['DegreesMin'].to_f
      @degrees_max = node.attributes['DegreesMax'].to_f
      @name = node.attributes['Name']
    end
  end

  class Focus
    attr_accessor :pan_max, :tilt_max, :type
    def initialize(node)
      return if node.empty?
      @pan_max = node.attributes['PanMax'].to_i
      @tilt_max = node.attributes['TiltMax'].to_i
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
      @bulb = Bulb.new(node.find_first('xmlns:Bulb', NS))
      @dimensions = Dimensions.new(node.find_first('xmlns:Dimensions', NS))
      @lens = Lens.new(node.find_first('xmlns:Lens', NS))
      @focus = Focus.new(node.find_first('xmlns:Focus', NS))
      @technical = Technical.new(node.find_first('xmlns:Technical', NS))
    end
  end

  class ChannelRef
    attr_accessor :number, :name
    def initialize(node)
      return if node.empty?
      @name = node.content
      @number = node.attributes['Number'].to_i
    end
  end

  class Head
    attr_accessor :channels, :index
    def initialize(node)
      @channels = []
      return if node.empty?
      @channels = node.find('xmlns:Channel', NS).map { |c| c.content.to_i }
    end
  end

  class Mode
    attr_accessor :name, :physical, :channels, :heads
    def initialize(node)
      @channels = []
      @heads = []
      return if node.empty?
      @name = node.attributes['Name']
      @physical = Physical.new(node.find_first('xmlns:Physical', NS))
      n = node.find('xmlns:Channel', NS)
      @channels = n.map { |c| ChannelRef.new(c) } unless n.empty?
      n = node.find('xmlns:Head', NS)
      @heads = n.map { |h| Head.new(h) } unless n.empty?
      @heads.each_with_index { |h, i| h.index = i + 1 }
    end
  end

  attr_accessor :path, :manufacturer, :model, :type, :creator, :channels, :modes

  def initialize(path)
    @channel = []
    @modes = []

    load(path)
  end

  COLOR_CHANGER = 'Color Changer'
  DIMMER = 'Dimmer'
  EFFECT = 'Effect'
  FAN = 'Fan'
  FLOWER = 'Flower'
  HAZER = 'Hazer'
  LASER = 'Laser'
  MOVING_HEAD = 'Moving Head'
  OTHER = 'Other'
  SCANNER = 'Scanner'
  SMOKE = 'Smoke'
  STROBE = 'Strobe'

  def type_icon
    case @type
    when COLOR_CHANGER
      'fixture.png'
    when DIMMER
      'dimmer.png'
    when EFFECT
      'effect.png'
    when FAN
      'fan.png'
    when FLOWER
      'flower.png'
    when HAZER
      'hazer.png'
    when LASER
      'laser.png'
    when MOVING_HEAD
      'movinghead.png'
    when OTHER
      'other.png'
    when SCANNER
      'scanner.png'
    when SMOKE
      'smoke.png'
    when STROBE
      'strobe.png'
    else
      'other.png'
    end
  end

  AUTHORS_TO_FIX = {
    'hjunnila' => 'Heikki Junnila',
    'jlgriffin' => 'JL Griffin',
    'griffinwebnet' => 'JL Griffin',
    ',,,' => ''
  }

  def load(path)
    qxf = File.read(path)
    if qxf.include?(OLD_NS_URI)
      qxf.gsub!(OLD_NS_URI, NS_URI)
      File.open(path, 'w') { |f| f.write(qxf) }
    end

    @doc = LibXML::XML::Document.string(qxf)
    @doc.root.find('//xmlns:Dimensions', NS).each do |node|
      node['Weight'] = node['Weight'].tr(',', '.')
    end

    @doc.root.find('//xmlns:Lens', NS).each do |node|
      node['DegreesMin'] = node['DegreesMin'].tr(',', '.')
    end

    @doc.root.find('//xmlns:Lens', NS).each do |node|
      node['DegreesMax'] = node['DegreesMax'].tr(',', '.')
    end

    @doc.root.find('//xmlns:Author', NS).each do |node|
      AUTHORS_TO_FIX.each do |old, new|
        node.content = node.content.gsub(old, new)
      end
    end

    if @doc.root.namespaces.default.nil?
      @doc.root.namespaces.namespace = LibXML::XML::Namespace.new(@doc.root, nil, NS_URI)
    end

    qxf2 = @doc.to_s(indent: true, encoding: LibXML::XML::Encoding::UTF_8)

    if qxf != qxf2
      File.open(path, 'w') { |f| f.write(qxf2) }
      @doc = LibXML::XML::Document.string(qxf2)
    end

    @path = path
    @manufacturer = @doc.find_first('/xmlns:FixtureDefinition/xmlns:Manufacturer', NS).content
    @model = @doc.find_first('/xmlns:FixtureDefinition/xmlns:Model', NS).content
    @type = @doc.find_first('/xmlns:FixtureDefinition/xmlns:Type', NS).content
    @creator = Creator.new(@doc.find_first('/xmlns:FixtureDefinition/xmlns:Creator', NS))
    @channels = @doc.find('/xmlns:FixtureDefinition/xmlns:Channel', NS).map { |c| Channel.new(c) }
    @modes = @doc.find('/xmlns:FixtureDefinition/xmlns:Mode', NS).map { |m| Mode.new(m) }
  end
end

class Fixtures
  attr_reader :fixtures

  def initialize
    @fixtures = []
  end

  def load_fixtures(path)
    Dir.chdir(path) do
      Dir.glob('*.qxf').sort.each do |f|
        @fixtures << FixtureDef.new(f)
      end
    end
  end

  def heads_have_common_channels(mode)
    all_head_channels = mode.heads.inject([]) { |a, e| a + e.channels }.sort
    unique_head_channels = all_head_channels.uniq.sort

    all_head_channels != unique_head_channels
  end

  def update_fixtures_map(filename = 'FixturesMap.xml')
    orig_data = File.read(filename)
    doc = LibXML::XML::Document.string(orig_data)
    doc.root.children.map(&:remove!)
    total_wrong = 0
    @fixtures.sort_by { |f| f.path.downcase }.each do |f|
      node = LibXML::XML::Node.new('fixture')
      LibXML::XML::Attr.new(node, 'path', f.path)
      LibXML::XML::Attr.new(node, 'mf', f.manufacturer)
      LibXML::XML::Attr.new(node, 'md', f.model)

      wrong_pan = f.modes.find { |m| m.physical.focus.pan_max == 0 }
      wrong_tilt = f.modes.find { |m| m.physical.focus.tilt_max == 0 }
      has_pan = f.channels.find { |c| c.group.name == FixtureDef::Group::PAN }
      has_tilt = f.channels.find { |c| c.group.name == FixtureDef::Group::TILT }
      wrong_width = f.modes.find { |m| m.physical.dimensions.width == 0 }
      wrong_height = f.modes.find { |m| m.physical.dimensions.height == 0 }
      wrong_depth = f.modes.find { |m| m.physical.dimensions.depth == 0 }
      wrong_weight = f.modes.find { |m| m.physical.dimensions.weight == 0 }
      wrong_heads = f.modes.find { |m| heads_have_common_channels(m) }
      wrong_msb = f.channels.select do |c|
        c.group.byte == FixtureDef::Group::MSB &&
        (c.name.downcase.include?('fine') || c.name.downcase.include?('least')) &&
        (f.model != 'Event Bar LED' || c.name != 'Movement (Fine)')
      end
      wrong_lsb = f.channels.select { |c| c.group.byte == FixtureDef::Group::LSB && c.name.downcase.include?('coarse') }

      problems = []
      problems << 'PAN' if wrong_pan && has_pan
      problems << 'TILT' if wrong_tilt && has_tilt
      problems << 'WIDTH' if wrong_width
      problems << 'HEIGHT' if wrong_height
      problems << 'DEPTH' if wrong_depth
      problems << 'WEIGHT' if wrong_weight
      problems << 'HEADS' if wrong_heads
      problems << "MSB: #{wrong_msb.map(&:name).join(',')}" unless wrong_msb.empty?
      problems << "LSB: #{wrong_msb.map(&:name).join(',')}" unless wrong_lsb.empty?

      unless problems.empty?
        puts "#{f.path}: #{problems.join ' '}"
        total_wrong += 1
      end

      doc.root << node
    end

    puts "wrong: #{total_wrong}"

    if doc.root.namespaces.default.nil?
      doc.root.namespaces.namespace = LibXML::XML::Namespace.new(doc.root, nil, 'http://www.qlcplus.org/FixturesMap')
    end
    new_data = doc.to_s(indent: true, encoding: LibXML::XML::Encoding::UTF_8).gsub('http://qlcplus.sourceforge.net/FixturesMap', 'http://www.qlcplus.org/FixturesMap')
    return if orig_data == new_data

    File.open(filename, 'w') { |f| f.write(new_data) }
  end

  def make_overview(filename = 'index.html')
    File.open(filename, 'w') do |f|
      f << <<-EOF
<html>
<head>
    <meta charset='utf-8'>
</head>
<body>
  <table border=1>
    <tr>
      <th rowspan=3>Manuf</th>
      <th rowspan=3>Model</th>
      <th rowspan=3>Version</th>
      <th rowspan=3>Author</th>
      <th rowspan=3>Type</th>
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
      @fixtures.sort_by { |fixture| fixture.path.downcase }.each do |fixture|
        f << <<-EOF
    <tr>
      <td rowspan=#{fixture.modes.size}>#{fixture.manufacturer}</td>
      <td rowspan=#{fixture.modes.size}>#{fixture.model}</td>
      <td rowspan=#{fixture.modes.size}>#{fixture.creator.version}</td>
      <td rowspan=#{fixture.modes.size}>#{fixture.creator.author}</td>
      <td rowspan=#{fixture.modes.size}><img src="gfx/#{fixture.type_icon}" /> #{fixture.type}</td>

EOF
        fixture.modes.each_with_index do |mode, index|
          f << "    <tr>\n" if index > 0
          f << <<-EOF
      <td>#{mode.name}</td>
      <td>#{mode.heads.size}</td>
      <td>#{mode.physical.bulb.lumens}</td>
      <td>#{mode.physical.bulb.type}</td>
      <td>#{mode.physical.bulb.color_temp}</td>
      <td>#{mode.physical.dimensions.width}</td>
      <td>#{mode.physical.dimensions.height}</td>
      <td>#{mode.physical.dimensions.depth}</td>
      <td>#{mode.physical.dimensions.weight}</td>
      <td>#{mode.physical.lens.degrees_min}</td>
      <td>#{mode.physical.lens.degrees_max}</td>
      <td>#{mode.physical.lens.name}</td>
      <td>#{mode.physical.focus.pan_max}</td>
      <td>#{mode.physical.focus.tilt_max}</td>
      <td>#{mode.physical.focus.type}</td>
      <td>#{mode.physical.technical.power_consumption}</td>
      <td>#{mode.physical.technical.dmx_connector}</td>
    </tr>
EOF
        end

        f << <<-EOF
    <tr>
      <td colspan=22>
        <table border=1>
          <tr>
            <th>&nbsp;</th>
            <th colspan="2">group</th>
            <th>byte</th>
            <th>color</th>
EOF
        fixture.modes.each do |mode|
          f << "            <th colspan=2> #{mode.name}</th>\n"
        end

        f << "          </tr>\n"

        fixture.channels.each do |channel|
          f << "          <tr>\n"
          f << "            <td>#{channel.name}</td>\n"
          icon = channel.group.icon
          if icon.nil?
            f << "            <td></td>\n"
          else
            f << "            <td><img src=\"gfx/#{icon}\" /></td>\n"
          end
          f << "            <td>#{channel.group.name}</td>\n"
          f << "            <td>#{channel.group.byte_str}</td>\n"
          rgb_color = channel.rgb_color
          if rgb_color.nil?
            f << "            <td>#{channel.color}</td>\n"
          else
            f << "            <td style=\"background-color: #{rgb_color}\">#{channel.color}</td>\n"
          end

          fixture.modes.each do |mode|
            mch = mode.channels.find { |mc| mc.name == channel.name }
            if mch.nil?
              f << "            <td></td>\n"
              f << "            <td></td>\n"
            else
              heads = mode.heads.map { |head| head.index if head.channels.include? mch.number }.compact.join(',')
              f << "            <td>#{mch.number}</td>\n"
              f << "            <td>#{heads.empty? ? '&nbsp;' : heads}</td>\n"
            end
          end

          f << "          </tr>\n"
          if false
            channel.capabilities.each do |capability|
              f << "          <tr>\n"
              f << "            <td colspan=\"#{5 + fixture.modes.size * 2}\"></td>\n"
              f << "            <td>#{capability.min}</td>\n"
              f << "            <td>#{capability.max}</td>\n"
              f << "            <td>#{capability.name}</td>\n"
              f << "            <td>#{capability.res}</td>\n"
              if capability.color.nil?
                f << "            <td>&nbsp;</td>\n"
              else
                f << "            <td style=\"background-color: #{capability.color}\">#{capability.color}</td>\n"
              end
              if capability.color2.nil?
                f << "            <td>&nbsp;</td>\n"
              else
                f << "            <td style=\"background-color: #{capability.color2}\">#{capability.color2}</td>\n"
              end
              f << "          </tr>\n"
            end
          end
        end

        f << <<-EOF
          </tr>
        </table>
      </td>
EOF
      end

      f << <<-EOF
</table>
</body>
</html>
EOF
    end
  end

  def shutter(filename = 'index.html')
    File.open(filename, 'w') do |f|
      @fixtures.sort_by { |fixture| fixture.path.downcase }.each do |fixture|
        shutter_channels = fixture.channels.select { |channel| channel.group.name == FixtureDef::Group::Shutter && channel.group.byte == FixtureDef::Group::MSB }
        next if shutter_channels.empty?

        f << <<-EOF
=== #{fixture.manufacturer} #{fixture.model}
#{fixture.type}

EOF
        shutter_channels.each do |channel|
          f << "-- #{channel.name}\n"
          channel.capabilities.each do |capability|
            shutter = 'open'
            if capability.name =~ /close|blackout|off/i
              shutter = 'closed'
            elsif capability.name =~ /strob|pulse/i
              shutter = 'strobe'
            end

            f << "#{capability.min} - #{capability.max} #{shutter} #{capability.name}\n"
          end
        end
      end
    end
  end
end

# LibXML::XML::Error.set_handler(&LibXML::XML::Error::VERBOSE_HANDLER)
LibXML::XML::Error.set_handler do |error|
  puts error.to_s
end

fm = Fixtures.new
fm.load_fixtures('.')
fm.update_fixtures_map

puts "Total fixtures: #{fm.fixtures.size}"

if ARGV.size > 0
  fm.make_overview(ARGV[0])
  # fm.shutter(ARGV[0])
end
