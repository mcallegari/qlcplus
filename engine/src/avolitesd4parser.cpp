/*
  Q Light Controller
  avolitesd4parser.cpp

  Copyright (C) Rui Barreiros
                Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QDomDocument>
#include <QDomElement>
#include <QStringList>
#include <QDebug>

#include "avolitesd4parser.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcphysical.h"
#include "qlcchannel.h"
#include "qlcfile.h"

// Channel groups
#define KD4GroupSpecial   "S"
#define KD4GroupIntensity "I"
#define KD4GroupPanTilt   "P"
#define KD4GroupColour    "C"
#define KD4GroupGobo      "G"
#define KD4GroupBeam      "B"
#define KD4GroupEffect    "E"

// Channels
#define KD4TagFixture   "Fixture"
#define KD4TagName      "Name"
#define KD4TagShortName "Shortname"
#define KD4TagCompany   "Company"
#define KD4TagControl   "Control"
#define KD4TagID        "ID"
#define KD4TagGroup     "Group"
#define KD4TagSpeed     "Speed"
#define KD4TagMacro     "Macro"
#define KD4TagReserved  "Reserved"
#define KD4TagShutter   "Shutter"
#define KD4TagPan       "Pan"
#define KD4TagTilt      "Tilt"
#define KD4TagCyan      "Cyan"
#define KD4TagMagenta   "Magenta"
#define KD4TagYellow    "Yellow"
#define KD4TagRed       "Red"
#define KD4TagBlue      "Blue"
#define KD4TagGreen     "Green"
#define KD4TagPrism     "Prism"
#define KD4TagEffect    "Effect"
#define KD4TagAttribute "Attribute"
#define KD4TagUpdate    "Update"

// Capabilities
#define KD4TagFunction                  "Function"
#define KD4TagFunctionName              "Name"
#define KD4TagFunctionDmx               "Dmx"
#define KD4TagFunctionDmxValueSeparator '~'

// Mode section
#define KD4TagMode                 "Mode"
#define KD4TagModeName             "Name"
#define KD4TagModeInclude          "Include"
#define KD4TagModeAttribute        "Attribute"
#define KD4TagModeChannelOffset    "ChannelOffset"
#define KD4TagModeID               "ID"
#define KD4TagModeChannelSeparator ','

// Physical section
#define KD4TagPhysical                     "Physical"
#define KD4TagPhysicalBulb                 "Bulb"
#define KD4TagPhysicalBulbType             "Type"
#define KD4TagPhysicalBulbLumens           "Lumens"
#define KD4TagPhysicalBulbColourTemp       "ColourTemp"
#define KD4TagPhysicalLens                 "Lens"
#define KD4TagPhysicalLensName             "Name"
#define KD4TagPhysicalLensDegrees          "Degrees"
#define KD4TagPhysicalLensDegreesSeparator '~'
#define KD4TagPhysicalWeight               "Wight"
#define KD4TagPhysicalWeightKg             "Kg"
#define KD4TagPhysicalSize                 "Size"
#define KD4TagPhysicalSizeHeight           "Height"
#define KD4TagPhysicalSizeWidth            "Width"
#define KD4TagPhysicalSizeDepth            "Depth"
#define KD4TagPhysicalFocus                "Focus"
#define KD4TagPhysicalFocusType            "Type"
#define KD4TagPhysicalFocusPanMax          "PanMax"
#define KD4TagPhysicalFocusTiltMax         "TiltMax"

// Static attibute map shared between instances of the parser, initialized only
// once per application.
AvolitesD4Parser::StringToEnumMap AvolitesD4Parser::s_attributesMap;

AvolitesD4Parser::AvolitesD4Parser()
{
    if (s_attributesMap.isEmpty() == true)
    {
        // Setup our attribute mapping map helper
        s_attributesMap.insert(KD4GroupSpecial, AvolitesD4Parser::SPECIAL);
        s_attributesMap.insert(KD4GroupIntensity, AvolitesD4Parser::INTENSITY);
        s_attributesMap.insert(KD4GroupPanTilt, AvolitesD4Parser::PANTILT);
        s_attributesMap.insert(KD4GroupColour, AvolitesD4Parser::COLOUR);
        s_attributesMap.insert(KD4GroupGobo, AvolitesD4Parser::GOBO);
        s_attributesMap.insert(KD4GroupBeam, AvolitesD4Parser::BEAM);
        s_attributesMap.insert(KD4GroupEffect, AvolitesD4Parser::EFFECT);
    }
}

AvolitesD4Parser::~AvolitesD4Parser()
{
}

bool AvolitesD4Parser::loadXML(const QString& path)
{
    m_lastError = QString();
    m_documentRoot = QDomDocument();
    m_channels.clear();

    if (path.isEmpty())
    {
        m_lastError = "filename not specified";
        return false;
    }

    m_documentRoot = QLCFile::readXML(path);
    if (m_documentRoot.isNull() == true)
    {
        m_lastError = "unable to read document";
        return false;
    }

    // check if the document has <Fixture></Fixture> if not then it's not a valid file
    QDomElement el = m_documentRoot.namedItem(KD4TagFixture).toElement();
    if (el.isNull() && (!el.hasAttribute(KD4TagName) ||
        !el.hasAttribute(KD4TagShortName) || !el.hasAttribute(KD4TagCompany)))
    {
        m_lastError = "wrong document format";
        return false;
    }

    return true;
}

bool AvolitesD4Parser::fillFixtureDef(QLCFixtureDef* fixtureDef)
{
    if (m_documentRoot.isNull())
    {
        m_lastError = "no XML loaded to process";
        return false;
    }

    fixtureDef->setManufacturer(fixtureCompany());
    fixtureDef->setModel(fixtureName());
    fixtureDef->setAuthor(copyright());

    // Parse all channels
    if (!parseChannels(m_documentRoot.namedItem(KD4TagFixture).toElement().namedItem(KD4TagControl).toElement(), fixtureDef))
        return false;

    // Parse all modes
    if (!parseModes(m_documentRoot.namedItem(KD4TagFixture).toElement(), fixtureDef))
        return false;

    fixtureDef->setType(guessType(fixtureDef));

    // TODO TODO TODO
    // Maybe also import preset palettes and macros ?!?!?!?!
    /**
        Can't be done for now, as qxf files don't have any information on preset palettes or macros
        for fixtures, they are automatically generated on the main application maybe in future... **/

    return true;
}

QString AvolitesD4Parser::lastError() const
{
    return m_lastError;
}

QString AvolitesD4Parser::fixtureName() const
{
    if (m_documentRoot.isNull())
        return QString();
    else
        return m_documentRoot.namedItem(KD4TagFixture).toElement().attribute(KD4TagName);
}

QString AvolitesD4Parser::fixtureShortName() const
{
    if (m_documentRoot.isNull())
        return QString();
    else
        return m_documentRoot.namedItem(KD4TagFixture).toElement().attribute(KD4TagShortName);
}

QString AvolitesD4Parser::fixtureCompany() const
{
    if (m_documentRoot.isNull())
        return QString();
    else
        return m_documentRoot.namedItem(KD4TagFixture).toElement().attribute(KD4TagCompany);
}

QString AvolitesD4Parser::copyright() const
{
    return QString("Avolites");
}

QLCChannel::Group AvolitesD4Parser::getGroupFromXML(const QDomElement& elem)
{
    if (elem.isNull())
        return QLCChannel::NoGroup;

    switch (stringToAttributeEnum(elem.attribute(KD4TagGroup)))
    {
    case AvolitesD4Parser::SPECIAL:
        if (elem.attribute(KD4TagID).contains(KD4TagSpeed, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagSpeed, Qt::CaseInsensitive))
            return QLCChannel::Speed;
        else if (elem.attribute(KD4TagID).contains(KD4TagMacro, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagMacro, Qt::CaseInsensitive))
            return QLCChannel::Effect;
        else if (elem.attribute(KD4TagID).contains(KD4TagReserved, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagReserved, Qt::CaseInsensitive))
            return QLCChannel::NoGroup;
        else
            return QLCChannel::Maintenance;
        break;

    default:
    case AvolitesD4Parser::INTENSITY:
        if (elem.attribute(KD4TagID).contains(KD4TagShutter, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagShutter, Qt::CaseInsensitive))
            return QLCChannel::Shutter;
        else
            return QLCChannel::Intensity;
        break;

    case AvolitesD4Parser::PANTILT:
        if (elem.attribute(KD4TagID).contains(KD4TagPan, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagPan, Qt::CaseInsensitive))
            return QLCChannel::Pan;
        else if (elem.attribute(KD4TagID).contains(KD4TagTilt, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagTilt, Qt::CaseInsensitive))
            return QLCChannel::Tilt;
        else
            return QLCChannel::NoGroup;
        break;

    case AvolitesD4Parser::COLOUR:
        if (elem.attribute(KD4TagID).contains(KD4TagCyan, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagCyan, Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute(KD4TagID).contains(KD4TagMagenta, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagMagenta, Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute(KD4TagID).contains(KD4TagYellow, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagYellow, Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute(KD4TagID).contains(KD4TagRed, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagRed, Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute(KD4TagID).contains(KD4TagGreen, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagGreen, Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute(KD4TagID).contains(KD4TagBlue, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagBlue, Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else
            return QLCChannel::Colour;
        break;

    case AvolitesD4Parser::GOBO:
        return QLCChannel::Gobo;
        break;

    case AvolitesD4Parser::BEAM:
        return QLCChannel::Beam;
        break;

    case AvolitesD4Parser::EFFECT:
        if (elem.attribute(KD4TagID).contains(KD4TagPrism, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagPrism, Qt::CaseInsensitive))
            return QLCChannel::Prism;
        else if (elem.attribute(KD4TagID).contains(KD4TagEffect, Qt::CaseInsensitive)
                || elem.attribute(KD4TagName).contains(KD4TagEffect, Qt::CaseInsensitive))
            return QLCChannel::Effect;
        else
            return QLCChannel::NoGroup;
        break;
    }

    return QLCChannel::NoGroup;
}

QLCChannel::PrimaryColour AvolitesD4Parser::getColourFromXML(const QDomElement& elem)
{
    if (elem.attribute(KD4TagGroup).compare(KD4GroupColour, Qt::CaseInsensitive) != 0)
        return QLCChannel::NoColour;

    if (elem.attribute(KD4TagID).contains(KD4TagCyan, Qt::CaseInsensitive)
            || elem.attribute(KD4TagName).contains(KD4TagCyan, Qt::CaseInsensitive))
        return QLCChannel::Cyan;
    else if (elem.attribute(KD4TagID).contains(KD4TagMagenta, Qt::CaseInsensitive)
            || elem.attribute(KD4TagName).contains(KD4TagMagenta, Qt::CaseInsensitive))
        return QLCChannel::Magenta;
    else if (elem.attribute(KD4TagID).contains(KD4TagYellow, Qt::CaseInsensitive)
            || elem.attribute(KD4TagName).contains(KD4TagYellow, Qt::CaseInsensitive))
        return QLCChannel::Yellow;
    else if (elem.attribute(KD4TagID).contains(KD4TagRed, Qt::CaseInsensitive)
            || elem.attribute(KD4TagName).contains(KD4TagRed, Qt::CaseInsensitive))
        return QLCChannel::Red;
    else if (elem.attribute(KD4TagID).contains(KD4TagGreen, Qt::CaseInsensitive)
            || elem.attribute(KD4TagName).contains(KD4TagGreen, Qt::CaseInsensitive))
        return QLCChannel::Green;
    else if (elem.attribute(KD4TagID).contains(KD4TagBlue, Qt::CaseInsensitive)
            || elem.attribute(KD4TagName).contains(KD4TagBlue, Qt::CaseInsensitive))
        return QLCChannel::Blue;
    else
        return QLCChannel::NoColour;
}

bool AvolitesD4Parser::isFunction(const QDomElement& elem) const
{
    QDomElement el = elem.firstChildElement(KD4TagFunction);
    for (; !el.isNull(); el = el.nextSiblingElement(KD4TagFunction))
    {
        if (!el.attribute(KD4TagUpdate).isEmpty())
            return true;
    }

    return false;
}

bool AvolitesD4Parser::is16Bit(const QDomElement& elem) const
{
    QDomElement el = elem.firstChildElement(KD4TagFunction);
    for (; !el.isNull(); el = el.nextSiblingElement(KD4TagFunction))
    {
        QString dmx = el.attribute(KD4TagFunctionDmx);
        QStringList dmxValues = dmx.split(KD4TagFunctionDmxValueSeparator);

        if (dmxValues.isEmpty())
            return false;

        // I remember avolites sometimes switches the sides on dmx values, sometimes, the left of the ~
        // is not always the lowest, better check both sides

        if (dmxValues.value(0).toInt() > 256)
            return true;

        // Is there aright side ? (there should always be something in the right side of the ~,
        // or avolites desks won't parse the file, anyway, there should be a check, ir some smart
        // dude will complain this crashes with his D4 file)

        if (dmxValues.size() > 1)
        {
            if (dmxValues.value(1).toInt() > 256)
                return true;
        }
    }

    return false;
}

bool AvolitesD4Parser::parseChannels(const QDomElement& elem, QLCFixtureDef* fixtureDef)
{
    QDomElement el = elem.firstChildElement(KD4TagAttribute);
    for (; !el.isNull(); el = el.nextSiblingElement(KD4TagAttribute))
    {
        // Small integrity check
        if (el.attribute(KD4TagID).isEmpty())
            continue;

        // If this attribute is a function (i.e. an attribute used as a control variable for other attributes)
        // then we just ignore it and continue. We can check it by checking if attribute Update on a <Function/> exists
        if (isFunction(el))
            continue;

        QLCChannel* chan = new QLCChannel();
        chan->setName(el.attribute(KD4TagName));
        chan->setGroup(getGroupFromXML(el));
        chan->setColour(getColourFromXML(el));
        chan->setControlByte(QLCChannel::MSB);

        // add channel to fixture definition
        fixtureDef->addChannel(chan);
        m_channels.insert(el.attribute(KD4TagID), chan);

        // if this channel is a NoGroup then we don't need to continue
        // no capabilities nor 16 bit channel
        if (chan->group() == QLCChannel::NoGroup)
            continue;

        // parse capabilities
        if (!parseCapabilities(el, chan))
        {
            m_channels.remove(el.attribute(KD4TagID));
            delete chan;
            return false;
        }

        // If we have a DMX attribute higher than 255 means we have an attribute with a 16bit precision
        // so, we add another channel, with 'Fine' appended to it's name and set the LSB controlbyte

        // NOTE: this can be changed in the future, pending the revamp over adding 16bit capabilities to any channel
        // not only pan/tiltm, therefore I didn't add a constant for Fine and kept it as it.
        if (is16Bit(el))
        {
            QLCChannel* fchan = new QLCChannel();
            fchan->setName(el.attribute(KD4TagName) + " Fine");
            fchan->setGroup(getGroupFromXML(el));
            fchan->setColour(getColourFromXML(el));
            fchan->setControlByte(QLCChannel::LSB);

            // parse capabilities
            if (!parseCapabilities(el, fchan, true))
            {
                delete fchan;
                return false;
            }

            // Finally add channel to fixture definition
            fixtureDef->addChannel(fchan);
            m_channels.insert(el.attribute(KD4TagID) + " Fine", fchan);
        }
    }

    return true;
}

bool AvolitesD4Parser::parseCapabilities(const QDomElement& elem, QLCChannel* chan, bool isFine)
{
    QDomElement el = elem.firstChildElement(KD4TagFunction);
    for (; !el.isNull(); el = el.nextSiblingElement(KD4TagFunction))
    {
        // Small integrity check
        if (el.attribute(KD4TagFunctionName).isEmpty())
            continue;

        QString dmx = el.attribute(KD4TagFunctionDmx);
        QStringList dmxValues = dmx.split(KD4TagFunctionDmxValueSeparator);

        // Here, instead of checking all the time for both dmxValues, it's more efficient to
        // set a default value if it's missing
        if (dmxValues.size() == 0)
            dmxValues << QString("0") << QString("0");
        else if (dmxValues.size() == 1)
            dmxValues << QString("0");

        // if were trying to get capabilities from a 16bit channel, we need to change them to 8 bit
        int minValue = 0, maxValue = 0;

        if (dmxValues.value(0).toInt() > 256)
            minValue = 0xFF & (dmxValues.value(0).toInt() >> 8);
        else
            minValue = dmxValues.value(0).toInt();

        if (dmxValues.value(1).toInt() > 256)
            maxValue = 0xFF & (dmxValues.value(1).toInt() >> 8);
        else
            maxValue = dmxValues.value(1).toInt();

        // Guess what, I seen this happen, it seems min value is not always on the left of the ~
        // sometimes they're switched!
        if (minValue > maxValue)
        {
            int tmp = maxValue;
            maxValue = minValue;
            minValue = tmp;
        }

        QString name = el.attribute(KD4TagFunctionName);
        if (isFine)
            name += " Fine";

        QLCCapability* cap = new QLCCapability(minValue, maxValue, name);

        // We just ignore capability adding errors, because avolites often repeats attributes due to conditionals
        // so we just add the first one we get, the repeating ones are ignored naturally and
        // obviously further human verification is needed on the fixture definition to fix this issues
        chan->addCapability(cap);
    }

    return true;
}

bool AvolitesD4Parser::parseModes(const QDomElement& elem, QLCFixtureDef* fixtureDef)
{
    QDomElement el = elem.firstChildElement(KD4TagMode);
    for (; !el.isNull(); el = el.nextSiblingElement(KD4TagMode))
    {
        if (el.attribute(KD4TagModeName).isEmpty())
            continue;

        QLCFixtureMode* mode = new QLCFixtureMode(fixtureDef);
        mode->setName(el.attribute(KD4TagModeName));

        // Parse physical
        parsePhysical(el.namedItem(KD4TagPhysical).toElement(), mode);

        QMap <int,QLCChannel*> channelList;
        QDomElement e = el.namedItem(KD4TagModeInclude).toElement().firstChildElement(KD4TagModeAttribute);
        for (; !e.isNull(); e = e.nextSiblingElement(KD4TagModeAttribute))
        {
            // Some channels are conditionals not real channels
            if (e.attribute(KD4TagModeChannelOffset).isEmpty())
                continue;

            if (m_channels.contains(e.attribute(KD4TagModeID)))
            {
                // might be a 16 bit channel, so we have 2 DMX addresses
                QString dmx = e.attribute(KD4TagModeChannelOffset);
                if (dmx.contains(KD4TagModeChannelSeparator, Qt::CaseInsensitive))
                {
                    // 16 bit address, we need to add 2 channels, this one, and we need the fine one
                    QStringList dmxValues = dmx.split(KD4TagModeChannelSeparator);
                    // if there's more than 2 addresses, or less than 2, bail out, don't know how to handle this, shouldn't happen ever.
                    if (dmxValues.size() > 2 || dmxValues.size() < 2)
                        continue;

                    // Add this one
                    channelList.insert(dmxValues.value(0).toInt(), m_channels.value(e.attribute(KD4TagID)));
                    QString name = m_channels.value(e.attribute(KD4TagModeID))->name();

                    // Search for the fine one
                    QMapIterator <QString,QLCChannel*> it(m_channels);
                    while (it.hasNext() == true)
                    {
                        it.next();
                        QLCChannel* ch(it.value());
                        Q_ASSERT(ch != NULL);

                        if (ch->name() == QString(name + " Fine"))
                            channelList.insert(dmxValues.value(1).toInt(), ch);
                    }
                }
                else
                {
                    channelList.insert(dmx.toInt(), m_channels.value(e.attribute(KD4TagModeID)));
                }
            }
        }

        QMapIterator <int,QLCChannel*> it(channelList);
        while (it.hasNext() == true)
        {
            it.next();
            Q_ASSERT(mode != NULL);
            mode->insertChannel(it.value(), it.key());
        }

        // Add the mode
        fixtureDef->addMode(mode);
    }

    return true;
}

void AvolitesD4Parser::parsePhysical(const QDomElement& el, QLCFixtureMode* mode)
{
    QLCPhysical phys;
    phys.setBulbType(el.namedItem(KD4TagPhysicalBulb).toElement().attribute(KD4TagPhysicalBulbType));
    phys.setBulbLumens(el.namedItem(KD4TagPhysicalBulb).toElement().attribute(KD4TagPhysicalBulbLumens).toInt());
    phys.setBulbColourTemperature(el.namedItem(KD4TagPhysicalBulb).toElement().attribute(KD4TagPhysicalBulbColourTemp).toInt());
    phys.setLensName(el.namedItem(KD4TagPhysicalLens).toElement().attribute(KD4TagName));

    QString degrees = el.namedItem(KD4TagPhysicalLens).toElement().attribute(KD4TagPhysicalLensDegrees);
    if (degrees.contains(KD4TagPhysicalLensDegreesSeparator))
    {
        QStringList deg = degrees.split(KD4TagPhysicalLensDegreesSeparator);
        if (deg.size() == 2)
        {
            if (deg.value(0).toInt() > deg.value(1).toInt())
            {
                phys.setLensDegreesMin(deg.value(1).toInt());
                phys.setLensDegreesMax(deg.value(0).toInt());
            }
            else
            {
                phys.setLensDegreesMin(deg.value(0).toInt());
                phys.setLensDegreesMax(deg.value(1).toInt());
            }
        } else if (deg.size() == 1)
        {
            phys.setLensDegreesMax(deg.value(0).toInt());
            phys.setLensDegreesMin(deg.value(0).toInt());
        }
    }
    else if (!degrees.isEmpty())
    {
        phys.setLensDegreesMax(degrees.toInt());
        phys.setLensDegreesMin(degrees.toInt());
    }

    phys.setWeight(el.namedItem(KD4TagPhysicalWeight).toElement().attribute(KD4TagPhysicalWeightKg).toDouble());

    phys.setHeight((int)(el.namedItem(KD4TagPhysicalSize).toElement().attribute(KD4TagPhysicalSizeHeight).toDouble() * 1000));
    phys.setWidth((int)(el.namedItem(KD4TagPhysicalSize).toElement().attribute(KD4TagPhysicalSizeWidth).toDouble() * 1000));
    phys.setDepth((int)(el.namedItem(KD4TagPhysicalSize).toElement().attribute(KD4TagPhysicalSizeDepth).toDouble() * 1000));

    phys.setFocusType(el.namedItem(KD4TagPhysicalFocus).toElement().attribute(KD4TagPhysicalFocusType));
    phys.setFocusPanMax(el.namedItem(KD4TagPhysicalFocus).toElement().attribute(KD4TagPhysicalFocusPanMax).toInt());
    phys.setFocusTiltMax(el.namedItem(KD4TagPhysicalFocus).toElement().attribute(KD4TagPhysicalFocusTiltMax).toInt());

    mode->setPhysical(phys);
}

AvolitesD4Parser::Attributes AvolitesD4Parser::stringToAttributeEnum(const QString& attr)
{
    // If there is none, empty or whatever always return something, default is SPECIAL
    if (attr.isEmpty())
        return AvolitesD4Parser::SPECIAL;

    if (s_attributesMap.value(attr.toUpper()))
        return s_attributesMap.value(attr.toUpper());
    else
        return AvolitesD4Parser::SPECIAL;
}

QString AvolitesD4Parser::guessType(QLCFixtureDef* def) const
{
    Q_ASSERT(def != NULL);

    int pan = 0, tilt = 0;
    int r = 0, g = 0, b = 0, c = 0, m = 0, y = 0, nocol = 0;
    int gobo = 0, colour = 0;
    int haze = 0, smoke = 0;
    int strobe = 0;

    QListIterator <QLCChannel*> it(def->channels());
    while (it.hasNext() == true)
    {
        const QLCChannel* ch(it.next());
        if (ch->group() == QLCChannel::Pan)
        {
            pan++;
        }
        else if (ch->group() == QLCChannel::Tilt)
        {
            tilt++;
        }
        else if (ch->group() == QLCChannel::Intensity)
        {
            if (ch->colour() == QLCChannel::Red)
                r++;
            else if (ch->colour() == QLCChannel::Green)
                g++;
            else if (ch->colour() == QLCChannel::Blue)
                b++;
            else if (ch->colour() == QLCChannel::Cyan)
                c++;
            else if (ch->colour() == QLCChannel::Magenta)
                m++;
            else if (ch->colour() == QLCChannel::Yellow)
                y++;
            else
                nocol++;
        }
        else if (ch->group() == QLCChannel::Shutter)
        {
            if (ch->searchCapability(/*S/s*/"trobe", false) != NULL)
                strobe++;
        }
        else if (ch->group() == QLCChannel::Gobo)
        {
            gobo++;
        }
        else if (ch->group() == QLCChannel::Colour)
        {
            colour++;
        }
        else if (ch->name().contains("strobe", Qt::CaseInsensitive) == true)
        {
            strobe++;
        }
        else if (ch->name().contains("haze", Qt::CaseInsensitive) == true)
        {
            haze++;
        }
        else if (ch->name().contains("smoke", Qt::CaseInsensitive) == true)
        {
            smoke++;
        }
    }

    if (pan >= 2 && tilt >= 2)
        return QString("Moving Head"); // Quite probable, few scanners with 16bit addressing
    else if (pan == 1 && tilt == 1)
        return QString("Scanner"); // Quite probable, though some moving heads are only 8bit
    else if (gobo > 0)
        return QString("Flower"); // No pan/tilt, but gobo, fairly certain
    else if (colour > 0 || (r > 0 && g > 0 && b > 0) || (c > 0 && m > 0 && y > 0))
        return QString("Color Changer"); // No pan/tilt/gobos, but RGB/CMY mixing or dichro
    else if (strobe > 0)
        return QString("Strobe"); // Duh.
    else if (smoke > 0)
        return QString("Smoke"); // Duh.
    else if (nocol > 0)
        return QString("Dimmer"); // Kinda..mmmmh..
    else
        return QString("Other"); // Give up
}
