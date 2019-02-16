#!/usr/bin/env python

# requires Python LXML (sudo apt-get install python-lxml)

import sys
import os
import re
import argparse
import lxml.etree as etree

singleCapCount = 0

namespace = "http://www.qlcplus.org/FixtureDefinition"

# please see https://github.com/mcallegari/qlcplus/wiki/Fixture-definition-presets when changing this list
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
        "ColorMacro", "ColorWheel", "ColorWheelFine", "ColorRGBMixer", "ColorCTOMixer", "ColorCTCMixer", "ColorCTBMixer",
        "GoboWheel", "GoboWheelFine", "GoboIndex", "GoboIndexFine",
        "ShutterStrobeSlowFast", "ShutterStrobeFastSlow", "ShutterIrisMinToMax", "ShutterIrisMaxToMin", "ShutterIrisFine"
        "BeamFocusNearFar", "BeamFocusFarNear", "BeamFocusFine", "BeamZoomSmallBig", "BeamZoomBigSmall", "BeamZoomFine",
        "PrismRotationSlowFast", "PrismRotationFastSlow",
        "NoFunction" ]

def printPresets(group):
    presets = getPresetsArray()
    pMin = 1
    pMax = presets.index("NoFunction")

    if group == "Intensity":
        pMin = presets.index("IntensityMasterDimmer")
        pMax = presets.index("IntensityValueFine")
    elif group == "Pan":
        pMin = presets.index("PositionPan")
        pMax = presets.index("PositionPanFine")
    elif group == "Tilt":
        pMin = presets.index("PositionTilt")
        pMax = presets.index("PositionTiltFine")
    elif group == "Speed":
        pMin = presets.index("SpeedPanSlowFast")
        pMax = presets.index("SpeedPanTiltFastSlow")
    elif group == "Colour":
        pMin = presets.index("ColorMacro")
        pMax = presets.index("ColorCTBMixer")
    elif group == "Gobo":
        pMin = presets.index("GoboWheel")
        pMax = presets.index("GoboIndexFine")
    elif group == "Shutter":
        pMin = presets.index("ShutterStrobeSlowFast")
        pMax = presets.index("ShutterIrisFine")
    elif group == "Beam":
        pMin = presets.index("BeamFocusNearFar")
        pMax = presets.index("BeamZoomFine")
    elif group == "Prism":
        pMin = presets.index("PrismRotationSlowFast")
        pMax = presets.index("PrismRotationFastSlow")

    for i in range(pMin, pMax + 1):
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
                        print "Select an option!"
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

def check_physical(absname, node, hasPan, hasTilt):
    errNum = 0
    phy_tag = node.find('{' + namespace + '}Physical')

    if phy_tag is not None:

        dim_tag = phy_tag.find('{' + namespace + '}Dimensions')
        focus_tag = phy_tag.find('{' + namespace + '}Focus')
        tech_tag = phy_tag.find('{' + namespace + '}Technical')

        width = int(dim_tag.attrib.get('Width', 0))
        height = int(dim_tag.attrib.get('Height', 0))
        depth = int(dim_tag.attrib.get('Depth', 0))
        panDeg = int(focus_tag.attrib.get('PanMax', 0))
        tiltDeg = int(focus_tag.attrib.get('TiltMax', 0))

        if width == 0 or height == 0 or depth == 0:
            print absname + ": Invalid physical dimensions detected"
            errNum += 1

        if hasPan and panDeg == 0:
            print absname + ": Invalid PAN degrees"
            errNum += 1

        if hasTilt and tiltDeg == 0:
            print absname + ": Invalid TILT degrees"
            errNum += 1

        if tech_tag is not None:
            power = int(tech_tag.attrib.get('PowerConsumption', 0))
            if power == 0:
                print absname + ": Invalid power consumption"
                errNum += 1

    return errNum

###########################################################################################
# validate_fixture
#
# Check the syntax of a definition and reports errors if found
#
# path: the source path with the fixtures to validate
# filename: the relative file name
###########################################################################################

def validate_fixture(path, filename):
    absname = os.path.join(path, filename)
    parser = etree.XMLParser(ns_clean=True, recover=True)
    xmlObj = etree.parse(absname, parser=parser)
    root = xmlObj.getroot()

    global namespace
    errNum = 0
    hasPan = False
    hasTilt = False
    needSave = False

    ##################################### CHECK CREATOR #################################

    creator_tag = root.find('{' + namespace + '}Creator')

    if creator_tag is None:
        print "Creator tag not found"
    else:
        author_tag = creator_tag.find('{' + namespace + '}Author')
        name_tag = creator_tag.find('{' + namespace + '}Name')
        version_tag = creator_tag.find('{' + namespace + '}Version')

        numversion_tok = re.findall('\d+', version_tag.text)
        #print "Definition version: " + version_tag.text

        # extract a unified number from the QLC version string
        if len(numversion_tok) == 3:
            qlc_version = (int(numversion_tok[0]) * 10000) + (int(numversion_tok[1]) * 100) + int(numversion_tok[2])
        else:
            qlc_version = (int(numversion_tok[0]) * 10000) + (int(numversion_tok[1]) * 100)

        if author_tag is None:
            print absname + ": Author tag not found"
            errNum += 1
        else:
            # pre QLC+ definition didn't have the Author tag. Let's do
            # the following check only for newer defs
            if name_tag.text == "Q Light Controller Plus":
                if not author_tag.text:
                    print absname + ": Empty author name detected"
                    errNum += 1
                else:
                    authName = author_tag.text
                    if "@" in authName or "://" in authName or "www" in authName:
                        print absname + ": URLs or emails not allowed in author tag"
                        errNum += 1

    ################################ CHECK FIXTURE GENERALS ##############################

    manuf_tag = root.find('{' + namespace + '}Manufacturer')
    model_tag = root.find('{' + namespace + '}Model')
    type_tag = root.find('{' + namespace + '}Type')

    if manuf_tag is None or not manuf_tag.text:
        print absname + ": Invalid manufacturer detected"
        errNum += 1
    if model_tag is None or not model_tag.text:
        print absname + ": Invalid model detected"
        errNum += 1
    if type_tag is None or not type_tag.text:
        print absname + ": Invalid type detected"
        errNum += 1

    ##################################### CHECK CHANNELS #################################

    chCount = 0
    channelNames = []

    for channel in root.findall('{' + namespace + '}Channel'):
        chName = ""
        chPreset = ""
        if not 'Name' in channel.attrib:
            print absname + ": Invalid channel. No name specified"
            errNum += 1
        else:
            chName = channel.attrib['Name']
            channelNames.append(chName)

        if 'Preset' in channel.attrib:
            chPreset = channel.attrib['Preset']

        childrenCount = len(channel.getchildren())
        group_tag = channel.find('{' + namespace + '}Group')
        groupByte = -1

        if not chPreset and childrenCount == 0:
            print absname + "/" + chName + ": Invalid channel. Not a preset and no capabilities found"
            errNum += 1

        if not chPreset and group_tag is None:
            print absname + "/" + chName + ": Invalid channel. Not a preset and no group tag found"
            errNum += 1

        if group_tag is not None:
            if not group_tag.text:
                print absname + "/" + chName + ": Invalid channel. Empty group tag detected"
                errNum += 1
            else:
                if group_tag.text == 'Pan':
                    hasPan = True
                if group_tag.text == 'Tilt':
                    hasTilt = True

            if not 'Byte' in group_tag.attrib:
                print absname + "/" + chName + ": Invalid channel. Group byte attribute not found"
                errNum += 1
            else:
                groupByte = group_tag.attrib['Byte']

        if chPreset:
            if chPreset == "PositionPan" or chPreset == "PositionPanFine" or chPreset == "PositionXAxis":
                hasPan = True
            if chPreset == "PositionTilt" or chPreset == "PositionTiltFine" or chPreset == "PositionYAxis":
                hasTilt = True
            # no need to go further if this is a preset
            chCount += 1
            continue

        # check the word 'fine' against control byte
        if groupByte == 0 and 'fine' in chName:
            print absname + "/" + chName + ": control byte should be set to Fine (LSB)"
            errNum += 1

        ################################# CHECK CAPABILITIES ##############################

        rangeMin = 255
        rangeMax = 0
        lastMax = -1
        capCount = 0

        for capability in channel.findall('{' + namespace + '}Capability'):

            newResSyntax = False
            capName = capability.text
            if not capName:
                print absname + "/" + chName + ": Capability with no description detected"
                errNum += 1

            # check capabilities overlapping
            currMin = int(capability.attrib['Min'])
            currMax = int(capability.attrib['Max'])

            #print "Min: " + str(currMin) + ", max: " + str(currMax)

            if currMin <= lastMax:
                print absname + "/" + chName + "/" + capName + ": Overlapping values detected " + str(currMin) + "/" + str(lastMax)
                errNum += 1

            # disabled for now. 710 errors with this !
            #if currMin != lastMax + 1:
            #    print absname + "/" + chName + "/" + capName + ": Non contiguous range detected " + str(currMin) + "/" + str(lastMax)
            #    errNum += 1

            lastMax = currMax

            resource = capability.attrib.get('Res', "")

            # try and see if new sytax is on
            if not resource:
                resource = capability.attrib.get('Res1', "")
                newResSyntax = True

            if resource.startswith('/'):
                print absname + "/" + chName + "/" + capName + ": Absolute paths not allowed in resources"
                errNum += 1

            # check the actual existence of a gobo. If possible, migrate to SVG
            if resource and '/' in resource:
                goboPath = os.getcwd() + "/../gobos/" + resource
                #print "GOBO path: " + goboPath

                if not os.path.isfile(goboPath):
                    # check if a SVG version of the gobo exists
                    resource = resource.replace('png', 'svg')
                    goboPath = os.getcwd() + "/../gobos/" + resource

                    if not os.path.isfile(goboPath):
                        print absname + "/" + chName + "/" + capName + ": Non existing gobo file detected (" + resource + ")"
                        errNum += 1
                    else:
                        needSave = True
                        if newResSyntax:
                            capability.set('Res1', resource)
                        else:
                            capability.set('Res', resource)

            capCount += 1

        if capCount == 0:
            print absname + "/" + chName + ": Channel has no capabilities"
            errNum += 1

        chCount += 1

    if chCount == 0:
        print absname + ": Invalid fixture. No channels found!"
        errNum += 1

    ###################################### CHECK MODES ###################################

    modeCount = 0
    global_phy_tag = root.find('{' + namespace + '}Physical')

    for mode in root.findall('{' + namespace + '}Mode'):

        modeName = ""

        if not 'Name' in mode.attrib:
            print absname + ": mode name attribute not found"
            errNum += 1
        else:
            modeName = mode.attrib['Name']

        if not modeName:
            print absname + ": Empty mode name detected"
            errNum += 1

        # better to skip this for now. Still too many errors
        #if qlc_version >= 41100 and 'mode' in modeName.lower():
        #    print absname + "/" + modeName + ": word 'mode' found in mode name"
        #    errNum += 1

        modeChanCount = 0
        modeHeadsCount = 0

        for mchan in mode.findall('{' + namespace + '}Channel'):

            if mchan.text is None:
                print absname + "/" + modeName + ": Empty channel name found. This definition won't work."
                errNum += 1
            else:
                if not mchan.text in channelNames:
                    print absname + "/" + modeName + ": Channel " + mchan.text + " doesn't exist. This definition won't work."
                    errNum += 1

            modeChanCount += 1

        if modeChanCount == 0:
            print absname + "/" + modeName + ": No channel found in mode"
            errNum += 1

        for mchan in mode.findall('{' + namespace + '}Head'):
            modeHeadsCount += 1

        if modeHeadsCount == 1:
            print absname + "/" + modeName + ": Single head found. Not allowed"
            errNum += 1

#        if modeHeadsCount > 3:
#            print absname + "/" + modeName + ": Heads found: " + str(modeHeadsCount)

        phy_tag = mode.find('{' + namespace + '}Physical')

        if phy_tag is None and global_phy_tag is None:
            print absname + "/" + modeName + ": No physical data found"
            errNum += 1

        errNum += check_physical(absname, mode, hasPan, hasTilt)

        modeCount += 1

    if modeCount == 0:
        print absname + ": Invalid fixture. No modes found!"
        errNum += 1

    ################################ CHECK GLOBAL PHYSICAL ################################

    errNum += check_physical(absname, root, hasPan, hasTilt)

    if needSave:
        print "Saving back " + filename + "..."
        xmlFile = open(absname, "w")
        xmlFile.write(etree.tostring(root, pretty_print=True, xml_declaration=True, encoding="UTF-8", doctype="<!DOCTYPE FixtureDefinition>"))
        xmlFile.close()

    return errNum

###########################################################################################
# createFixtureMap
#
# Creates the Fixture definition map read by QLC+ at startup
#
###########################################################################################
def createFixtureMap():
    global namespace

    count = 0
    manufacturer = ""
    xmlFile = open("FixturesMap.xml", "w")
    root = etree.Element("FixturesMap")
    root.set('xmlns', 'http://www.qlcplus.org/FixturesMap')

    for dirname in sorted(os.listdir('.'), key=lambda s: s.lower()):

        if not os.path.isdir(dirname): continue

        if dirname != "scripts" and dirname != manufacturer:
            mfTag = etree.SubElement(root, "M")
            mfTag.set('n', dirname)
            manufacturer = dirname

        for filename in sorted(os.listdir(dirname), key=lambda s: s.lower()):
            if not filename.endswith('.qxf'): continue

            parser = etree.XMLParser(ns_clean=True, recover=True)
            xmlObj = etree.parse(os.path.join(dirname, filename), parser=parser)
            fxRoot = xmlObj.getroot()
            manufacturer = fxRoot.find('{' + namespace + '}Manufacturer')
            model = fxRoot.find('{' + namespace + '}Model')
            fxTag = etree.SubElement(mfTag, "F")
            fxTag.set('n', os.path.splitext(filename)[0])
            #fxTag.set('m', manufacturer.text)
            fxTag.set('m', model.text)
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
parser.add_argument('--validate [path]', help='Validate fixtures in the specified path',
                    nargs='*', dest='validate')
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
elif args.validate:
    if len(sys.argv) < 2:
        print "Usage " + sys.argv[0] + "--validate [path]"
        sys.exit()

    path = sys.argv[2]

    fileCount = 0
    errorCount = 0

    for dirname in sorted(os.listdir(path), key=lambda s: s.lower()):

        if not os.path.isdir(dirname): continue

        for filename in sorted(os.listdir(dirname), key=lambda s: s.lower()):
            if not filename.endswith('.qxf'): continue

            #print "Processing file " + filename
            errorCount += validate_fixture(dirname, filename)
            fileCount += 1

    print str(fileCount) + " definitions processed. " + str(errorCount) + " errors detected"

