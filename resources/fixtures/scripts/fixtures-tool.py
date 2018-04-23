#!/usr/bin/env python

import sys
import os
import argparse
import lxml.etree as etree

singleCapCount = 0

namespace = "http://www.qlcplus.org/FixtureDefinition"

def getPresetsArray():
    return [ 
        "Custom", 
        "IntensityMasterDimmer", "IntensityMasterDimmerFine", "IntensityDimmer", "IntensityDimmerFine",
        "IntensityRed", "IntensityRedFine", "IntensityGreen", "IntensityGreenFine", "IntensityBlue", "IntensityBlueFine",
        "IntensityCyan", "IntensityCyanFine", "IntensityMagenta", "IntensityMagentaFine", "IntensityYellow", "IntensityYellowFine",
        "IntensityAmber", "IntensityAmberFine", "IntensityWhite", "IntensityWhiteFine", "IntensityUV", "IntensityUVFine",
        "IntensityIndigo", "IntensityIndigoFine", "IntensityLime", "IntensityLimeFine", "IntensityHue", "IntensityHueFine",
        "IntensitySaturation", "IntensitySaturationFine", "IntensityLightness", "IntensityLightnessFine", 
        "IntensityValue", "IntensityValueFine",
        "PositionPan", "PositionPanFine", "PositionTilt", "PositionTiltFine", "PositionXAxis", "PositionYAxis",
        "SpeedPanSlowFast", "SpeedPanFastSlow", "SpeedTiltSlowFast", "SpeedTiltFastSlow", "SpeedPanTiltSlowFast", "SpeedPanTiltFastSlow",
        "ColorMacro", "ColorWheel", "ColorWheelFine", "ColorRGBMixer", "ColorCTOMixer", "ColorCTBMixer", 
        "GoboWheel", "GoboWheelFine", "GoboIndex", "GoboIndexFine",
        "ShutterStrobeSlowFast", "ShutterStrobeFastSlow",
        "BeamFocusNearFar", "BeamFocusFarNear", "BeamIris", "BeamIrisFine", "BeamZoomSmallBig", "BeamZoomBigSmall",
        "PrismRotationSlowFast", "PrismRotationFastSlow",
        "NoFunction" ]

def printPresets(group):
    presets = getPresetsArray()
    min = 1
    max = presets.index("NoFunction")

    if group == "Intensity":
        min = presets.index("IntensityMasterDimmer")
        max = presets.index("IntensityValueFine")
    elif group == "Pan":
        min = presets.index("PositionPan")
        max = presets.index("PositionPanFine")
    elif group == "Tilt":
        min = presets.index("PositionTilt")
        max = presets.index("PositionTiltFine")
    elif group == "Speed":
        min = presets.index("SpeedPanSlowFast")
        max = presets.index("SpeedPanTiltFastSlow")
    elif group == "Colour":
        min = presets.index("ColorMacro")
        max = presets.index("ColorCTBMixer")
    elif group == "Gobo":
        min = presets.index("GoboWheel")
        max = presets.index("GoboIndexFine")
    elif group == "Shutter":
        min = presets.index("ShutterStrobeSlowFast")
        max = presets.index("ShutterStrobeFastSlow")
    elif group == "Beam":
        min = presets.index("BeamFocusNearFar")
        max = presets.index("BeamZoomBigSmall")
    elif group == "Prism":
        min = presets.index("PrismRotationSlowFast")
        max = presets.index("PrismRotationFastSlow")

    for i in range(min, max + 1):
        sys.stdout.write("[" + str(i) + "] " + presets[i] + " ")
        sys.stdout.flush()
    print ""

###########################################################################################
# update_fixture
#
# Convert an 'old' syntax definition to the 'new' syntax, which includes:
# - single capability channels 
# - global physical dimension
#
# path: the source path with the fixtures to convert
# filename: the relative file name
# destpath: the destination folder where to save the converted fixture
###########################################################################################

def update_fixture(path, filename, destpath):
    absname = os.path.join(path, filename)
    parser = etree.XMLParser(ns_clean=True, recover=True)
    xmlObj = etree.parse(absname, parser=parser)
    root = xmlObj.getroot()
    fxSingleCapCount = 0
    
    global namespace
    
    ################################## PHYSICAL PROCESSING ################################

    global_phy = {}
    gphy_tag = etree.Element("Physical")
    
    for mode in root.findall('{' + namespace + '}Mode'):
        phy_dict = {}
        phy_tag = mode.find('{' + namespace + '}Physical')
        
        if not phy_tag:
            # Mode already processed. Skip
            continue
        
        bulb_tag = phy_tag.find('{' + namespace + '}Bulb')
        dim_tag = phy_tag.find('{' + namespace + '}Dimensions')
        lens_tag = phy_tag.find('{' + namespace + '}Lens')
        focus_tag = phy_tag.find('{' + namespace + '}Focus')
        tech_tag = phy_tag.find('{' + namespace + '}Technical')
        
        phy_dict.update(phy_tag.attrib)
        phy_dict.update(bulb_tag.attrib)
        phy_dict.update(dim_tag.attrib)
        phy_dict.update(lens_tag.attrib)
        phy_dict.update(focus_tag.attrib)
        if tech_tag:
            phy_dict.update(tech_tag.attrib)

        if not global_phy:
            global_phy = phy_dict
            gphy_tag = phy_tag
            mode.remove(phy_tag)
            print "Moving mode " + mode.attrib['Name'] + " to global"
        elif phy_dict == global_phy:
            mode.remove(phy_tag)
            print "Mode " + mode.attrib['Name'] + " is identical to global"     

    root.append(gphy_tag)

    ##################################### CHANNELS PROCESSING #################################
    
    for channel in root.findall('{' + namespace + '}Channel'):
        locCapCount = 0

        if 'Preset' in channel.attrib:
            # Channel already converted. Skip
            continue

        for cap in channel.findall('{' + namespace + '}Capability'):
            locCapCount += 1

        if locCapCount < 2:
            print "Single capability found in " + filename
            fxSingleCapCount += 1
            preset = ""
            name = ""
            group = ""
            color = ""
            controlByte = 0
            fineWord = ""

            # Modes have a <Channel> tag too, but they don't have a name
            if not 'Name' in channel.attrib:
                continue

            name = channel.attrib['Name']

            grpNode = channel.find('{' + namespace + '}Group')
            if grpNode is not None:
                group = grpNode.text
                controlByte = int(grpNode.attrib['Byte'])

            if controlByte == 1:
                fineWord = "Fine"

            if group == "Intensity":
                colNode = channel.find('{' + namespace + '}Colour')
                if colNode is not None:
                    color = colNode.text

                if color == "Red":
                    preset = "IntensityRed" + fineWord
                elif color == "Green":
                    preset = "IntensityGreen" + fineWord
                elif color == "Blue":
                    preset = "IntensityBlue" + fineWord
                elif color == "Cyan":
                    preset = "IntensityCyan" + fineWord
                elif color == "Magenta":
                    preset = "IntensityMagenta" + fineWord
                elif color == "Yellow":
                    preset = "IntensityYellow" + fineWord
                elif color == "Amber":
                    preset = "IntensityAmber" + fineWord
                elif color == "White":
                    preset = "IntensityWhite" + fineWord
                elif color == "UV":
                    preset = "IntensityUV" + fineWord
                elif color == "Lime":
                    preset = "IntensityLime" + fineWord
                elif color == "Indigo":
                    preset = "IntensityIndigo" + fineWord
                elif color == "Hue":
                    preset = "IntensityHue" + fineWord
                elif color == "Saturation":
                    preset = "IntensitySaturation" + fineWord
                elif color == "Lightness":
                    preset = "IntensityLightness" + fineWord
                elif color == "Value":
                    preset = "IntensityValue" + fineWord
                    
            elif group == "Pan":
                preset = "PositionPan" + fineWord
            elif group == "Tilt":
                preset = "PositionTilt" + fineWord

            #print "Found group " + group + ", control byte: " + str(controlByte)
            print chr(27) + "[2J" # clear screen
            print "File: " + filename
            print etree.tostring(channel)

            if not preset:
                printPresets(group)

            select = ""

            # wait for user input until a preset can be resolved
            while 1:
                select = raw_input("Replacement preset code (0 = keep) (enter = " + preset + "): ")
                if select == "":
                    if preset == "":
                        print "Select an option !"
                    else:
                        break
                else:
                    presets = getPresetsArray()
                    preset = presets[int(select)]
                    break

            # perform the XML Channel node replacement
            if preset != "Custom":
                channel.clear()
                channel.set("Name", name)
                channel.set("Preset", preset)
                channel.tail = "\n "

    newfile = os.path.join(destpath, filename)
    xmlFile = open(newfile, "w")
    xmlFile.write(etree.tostring(root, pretty_print=True, xml_declaration=True, encoding="UTF-8", doctype="<!DOCTYPE FixtureDefinition>"))
    xmlFile.close()

    return fxSingleCapCount

###########################################################################################
# createFixtureMap
#
# Creates the Fixture definition map read by QLC+ at startup
#
###########################################################################################
def createFixtureMap():
    global namespace

    count = 0
    xmlFile = open("FixturesMap.xml", "w")
    root = etree.Element("FixturesMap")
    root.set('xmlns', 'http://www.qlcplus.org/FixturesMap')

    for filename in sorted(os.listdir('.'), key=lambda s: s.lower()):
        if not filename.endswith('.qxf'): continue

        parser = etree.XMLParser(ns_clean=True, recover=True)
        xmlObj = etree.parse(filename, parser=parser)
        fxRoot = xmlObj.getroot()
        manufacturer = fxRoot.find('{' + namespace + '}Manufacturer')
        model = fxRoot.find('{' + namespace + '}Model')
        fxTag = etree.SubElement(root, "F")
        fxTag.set('n', os.path.splitext(filename)[0])
        fxTag.set('m', manufacturer.text)
        fxTag.set('d', model.text)
        #print manufacturer.text + ", " + model.text
        count += 1

    xmlFile.write(etree.tostring(root, pretty_print=True, xml_declaration=True, encoding="UTF-8", doctype="<!DOCTYPE FixturesMap>"))
    xmlFile.close()
    print "Fixtures in map: " + str(count)

###########################################################################################
#
#                                       MAIN
#
###########################################################################################
    
parser = argparse.ArgumentParser(description='Unified Fixture tool.')
parser.add_argument('--map', help='Create the Fixture map', action='store_true')
parser.add_argument('--convert [source] [destination]', help='Convert an "old" syntax Fixture definition', 
                    nargs='*', dest='convert')

args = parser.parse_args()

print args

if args.map:
    createFixtureMap()
elif args.convert:
    if len(sys.argv) < 3:
        print "Usage " + sys.argv[0] + "--convert [source folder] [destination folder]"
        sys.exit()

    path = sys.argv[2]
    if len(sys.argv) > 3:
        destpath = sys.argv[3]
    else:
        destpath = ""
    print "Converting fixtures in " + path + "..."

    for filename in os.listdir(path):
        if not filename.endswith('.qxf'): continue
        print "Processing file " + filename

        singleCapCount += update_fixture(path, filename, destpath)

    print "Scan done. Single cap found: " + str(singleCapCount)
