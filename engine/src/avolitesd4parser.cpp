/*
  Q Light Controller Plus
  avolitesd4parser.cpp

  Copyright (C) Rui Barreiros
                Heikki Junnila
                Massimo Callegari

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

#include <QXmlStreamReader>
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
#define KD4TagFixture   QString("Fixture")
#define KD4TagName      QString("Name")
#define KD4TagShortName QString("ShortName")
#define KD4TagCompany   QString("Company")
#define KD4TagControl   QString("Control")
#define KD4TagID        QString("ID")
#define KD4TagGroup     QString("Group")
#define KD4TagSpeed     QString("Speed")
#define KD4TagMacro     QString("Macro")
#define KD4TagReserved  QString("Reserved")
#define KD4TagShutter   QString("Shutter")
#define KD4TagPan       QString("Pan")
#define KD4TagTilt      QString("Tilt")
#define KD4TagCyan      QString("Cyan")
#define KD4TagMagenta   QString("Magenta")
#define KD4TagYellow    QString("Yellow")
#define KD4TagRed       QString("Red")
#define KD4TagBlue      QString("Blue")
#define KD4TagGreen     QString("Green")
#define KD4TagPrism     QString("Prism")
#define KD4TagEffect    QString("Effect")
#define KD4TagAttribute QString("Attribute")
#define KD4TagUpdate    QString("Update")

// Capabilities
#define KD4TagFunction                  QString("Function")
#define KD4TagFunctionName              QString("Name")
#define KD4TagFunctionDmx               QString("Dmx")
#define KD4TagFunctionDmxValueSeparator '~'

// Mode section
#define KD4TagMode                 QString("Mode")
#define KD4TagModeName             QString("Name")
#define KD4TagModeInclude          QString("Include")
#define KD4TagModeAttribute        QString("Attribute")
#define KD4TagModeChannelOffset    QString("ChannelOffset")
#define KD4TagModeID               QString("ID")
#define KD4TagModeChannelSeparator ','

// Palettes section
#define KD4TagPalettes          QString("Palettes")
#define KD4TagPalette           QString("Palette")

// Physical section
#define KD4TagPhysical                     QString("Physical")
#define KD4TagPhysicalBulb                 QString("Bulb")
#define KD4TagPhysicalBulbType             QString("Type")
#define KD4TagPhysicalBulbLumens           QString("Lumens")
#define KD4TagPhysicalBulbColourTemp       QString("ColourTemp")
#define KD4TagPhysicalLens                 QString("Lens")
#define KD4TagPhysicalLensName             QString("Name")
#define KD4TagPhysicalLensDegrees          QString("Degrees")
#define KD4TagPhysicalLensDegreesSeparator QString('~')
#define KD4TagPhysicalWeight               QString("Weight")
#define KD4TagPhysicalWeightKg             QString("Kg")
#define KD4TagPhysicalSize                 QString("Size")
#define KD4TagPhysicalSizeHeight           QString("Height")
#define KD4TagPhysicalSizeWidth            QString("Width")
#define KD4TagPhysicalSizeDepth            QString("Depth")
#define KD4TagPhysicalFocus                QString("Focus")
#define KD4TagPhysicalFocusType            QString("Type")
#define KD4TagPhysicalFocusPanMax          QString("PanMax")
#define KD4TagPhysicalFocusTiltMax         QString("TiltMax")

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

bool AvolitesD4Parser::loadXML(const QString& path, QLCFixtureDef *fixtureDef)
{
    m_lastError = QString();
    m_channels.clear();

    if (path.isEmpty())
    {
        m_lastError = "filename not specified";
        return false;
    }

    QXmlStreamReader *doc = QLCFile::getXMLReader(path);
    if (doc == NULL || doc->device() == NULL || doc->hasError())
    {
        m_lastError = QString("Unable to read from %1").arg(path);
        return false;
    }

    // check if the document has <Fixture></Fixture> if not then it's not a valid file
    if (doc->readNextStartElement() == false || doc->name() != KD4TagFixture)
    {
        m_lastError = "wrong document format";
        return false;
    }

    QXmlStreamAttributes attrs = doc->attributes();
    if ((!attrs.hasAttribute(KD4TagName)) || (!attrs.hasAttribute(KD4TagCompany)))
    {
        m_lastError = "the document doesn't have the required attributes";
        return false;
    }

    fixtureDef->setManufacturer(doc->attributes().value(KD4TagCompany).toString());
    fixtureDef->setModel(doc->attributes().value(KD4TagName).toString());
    fixtureDef->setAuthor("Avolites");

    while (doc->readNextStartElement())
    {
        if (doc->name() == KD4TagControl)
        {
            // Parse a channel
            if (parseChannel(doc, fixtureDef) == false)
                return false;
        }
        else if (doc->name() == KD4TagMode)
        {
            // Parse mode tag
            parseMode(doc, fixtureDef);
        }
        else if (doc->name() == KD4TagPalettes)
        {
            // TODO TODO TODO
            // Maybe also import preset palettes and macros ?!?!?!?!
            /**
                Can't be done for now, as qxf files don't have any information on preset palettes or macros
                for fixtures, they are automatically generated on the main application maybe in future... **/
            doc->skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown D4 tag:" << doc->name().toString();
            doc->skipCurrentElement();
        }
    }

    fixtureDef->setType(guessType(fixtureDef));

    QLCFile::releaseXMLReader(doc);

    return true;
}

QString AvolitesD4Parser::lastError() const
{
    return m_lastError;
}

QLCChannel::Group AvolitesD4Parser::getGroup(QString ID, QString name, QString group)
{
    if (name.isEmpty() && group.isEmpty())
        return QLCChannel::NoGroup;

    switch (stringToAttributeEnum(group))
    {
        case AvolitesD4Parser::SPECIAL:
            if (ID.contains(KD4TagSpeed, Qt::CaseInsensitive) ||
                name.contains(KD4TagSpeed, Qt::CaseInsensitive))
                    return QLCChannel::Speed;
            else if (ID.contains(KD4TagMacro, Qt::CaseInsensitive) ||
                     name.contains(KD4TagMacro, Qt::CaseInsensitive))
                        return QLCChannel::Effect;
            else if (ID.contains(KD4TagReserved, Qt::CaseInsensitive) ||
                     name.contains(KD4TagReserved, Qt::CaseInsensitive))
                        return QLCChannel::NoGroup;
            else
                return QLCChannel::Maintenance;
        break;

        default:
        case AvolitesD4Parser::INTENSITY:
            if (ID.contains(KD4TagShutter, Qt::CaseInsensitive) ||
                name.contains(KD4TagShutter, Qt::CaseInsensitive))
                    return QLCChannel::Shutter;
            else
                return QLCChannel::Intensity;
        break;

        case AvolitesD4Parser::PANTILT:
            if (ID.contains(KD4TagPan, Qt::CaseInsensitive) ||
                name.contains(KD4TagPan, Qt::CaseInsensitive))
                    return QLCChannel::Pan;
            else if (ID.contains(KD4TagTilt, Qt::CaseInsensitive) ||
                    name.contains(KD4TagTilt, Qt::CaseInsensitive))
                        return QLCChannel::Tilt;
            else
                return QLCChannel::NoGroup;
        break;

        case AvolitesD4Parser::COLOUR:
            if (ID.contains(KD4TagCyan, Qt::CaseInsensitive) ||
                name.contains(KD4TagCyan, Qt::CaseInsensitive))
                    return QLCChannel::Intensity;
            else if (ID.contains(KD4TagMagenta, Qt::CaseInsensitive) ||
                    name.contains(KD4TagMagenta, Qt::CaseInsensitive))
                        return QLCChannel::Intensity;
            else if (ID.contains(KD4TagYellow, Qt::CaseInsensitive) ||
                    name.contains(KD4TagYellow, Qt::CaseInsensitive))
                        return QLCChannel::Intensity;
            else if (ID.contains(KD4TagRed, Qt::CaseInsensitive) ||
                    name.contains(KD4TagRed, Qt::CaseInsensitive))
                        return QLCChannel::Intensity;
            else if (ID.contains(KD4TagGreen, Qt::CaseInsensitive) ||
                    name.contains(KD4TagGreen, Qt::CaseInsensitive))
                        return QLCChannel::Intensity;
            else if (ID.contains(KD4TagBlue, Qt::CaseInsensitive) ||
                    name.contains(KD4TagBlue, Qt::CaseInsensitive))
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
            if (ID.contains(KD4TagPrism, Qt::CaseInsensitive) ||
                name.contains(KD4TagPrism, Qt::CaseInsensitive))
                    return QLCChannel::Prism;
            else if (ID.contains(KD4TagEffect, Qt::CaseInsensitive) ||
                     name.contains(KD4TagEffect, Qt::CaseInsensitive))
                        return QLCChannel::Effect;
            else if (ID.contains(KD4TagMacro, Qt::CaseInsensitive) ||
                     name.contains(KD4TagMacro, Qt::CaseInsensitive))
                        return QLCChannel::Effect;
            else
                return QLCChannel::NoGroup;
        break;
    }

    return QLCChannel::NoGroup;
}

QLCChannel::PrimaryColour AvolitesD4Parser::getColour(QString ID, QString name, QString group)
{
    if (group.compare(KD4GroupColour, Qt::CaseInsensitive) != 0)
        return QLCChannel::NoColour;

    if (ID.contains(KD4TagCyan, Qt::CaseInsensitive) ||
        name.contains(KD4TagCyan, Qt::CaseInsensitive))
            return QLCChannel::Cyan;
    else if (ID.contains(KD4TagMagenta, Qt::CaseInsensitive) ||
            name.contains(KD4TagMagenta, Qt::CaseInsensitive))
                return QLCChannel::Magenta;
    else if (ID.contains(KD4TagYellow, Qt::CaseInsensitive) ||
             name.contains(KD4TagYellow, Qt::CaseInsensitive))
                return QLCChannel::Yellow;
    else if (ID.contains(KD4TagRed, Qt::CaseInsensitive) ||
            name.contains(KD4TagRed, Qt::CaseInsensitive))
                return QLCChannel::Red;
    else if (ID.contains(KD4TagGreen, Qt::CaseInsensitive) ||
            name.contains(KD4TagGreen, Qt::CaseInsensitive))
                return QLCChannel::Green;
    else if (ID.contains(KD4TagBlue, Qt::CaseInsensitive) ||
            name.contains(KD4TagBlue, Qt::CaseInsensitive))
                return QLCChannel::Blue;
    else
        return QLCChannel::NoColour;
}
/*
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
*/
bool AvolitesD4Parser::is16Bit(QString dmx) const
{
    QStringList dmxValues = dmx.split(KD4TagFunctionDmxValueSeparator);

    if (dmxValues.isEmpty())
        return false;

    // I remember avolites sometimes switches the sides on dmx values, sometimes, the left of the ~
    // is not always the lowest, better check both sides

    if (dmxValues.value(0).toInt() > 256)
        return true;

    // Is there a right side ? (there should always be something in the right side of the ~,
    // or avolites desks won't parse the file, anyway, there should be a check, if some smart
    // dude will complain this crashes with his D4 file)

    if (dmxValues.size() > 1)
    {
        if (dmxValues.value(1).toInt() > 256)
            return true;
    }

    return false;
}

QLCCapability *AvolitesD4Parser::getCapability(QString dmx, QString name, bool isFine)
{
    if (dmx.isEmpty())
        return NULL;

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

    if (isFine)
        name += " Fine";

    QLCCapability *cap = new QLCCapability(minValue, maxValue, name);

    return cap;
}

bool AvolitesD4Parser::parseChannel(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef)
{
    if (doc->name() != KD4TagControl)
        return false;

    while (doc->readNextStartElement())
    {
        if (doc->name() == KD4TagAttribute)
        {
            QString ID = doc->attributes().value(KD4TagID).toString();
            if (ID.isEmpty())
            {
                doc->skipCurrentElement();
                continue;
            }

            parseAttribute(doc, fixtureDef);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown control tag:" << doc->name().toString();
            doc->skipCurrentElement();
        }
    }

    return true;
}

bool AvolitesD4Parser::parseFunction(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef,
                                     QLCChannel *channel, QString ID, QString group)
{
    QXmlStreamAttributes attrs = doc->attributes();
    QString name = attrs.value(KD4TagFunctionName).toString();
    if (name.isEmpty())
    {
        doc->skipCurrentElement();
        return true;
    }

    QString dmx = attrs.value(KD4TagFunctionDmx).toString();
    QLCCapability *cap = getCapability(dmx, name);

    if (cap != NULL)
    {
        // We just ignore capability adding errors, because avolites often repeats attributes due to conditionals
        // so we just add the first one we get, the repeating ones are ignored naturally and
        // obviously further human verification is needed on the fixture definition to fix this issues
        channel->addCapability(cap);
    }

    if (is16Bit(dmx))
    {
        QLCChannel *fineChan = new QLCChannel();
        fineChan->setName(name + " Fine");
        fineChan->setGroup(getGroup(ID, name, group));
        fineChan->setColour(getColour(ID, name, group));
        fineChan->setControlByte(QLCChannel::LSB);
        QLCCapability *fineCap = getCapability(dmx, name, true);
        if (fineCap != NULL)
            fineChan->addCapability(fineCap);
        fixtureDef->addChannel(fineChan);
        m_channels.insert(ID + " Fine", fineChan);
    }
    //qDebug() << "Capability found" << cap->name() << cap->min() << cap->max();
    doc->skipCurrentElement();

    return true;
}

bool AvolitesD4Parser::parseAttribute(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef)
{
    if (doc->name() != KD4TagAttribute)
        return false;

    QXmlStreamAttributes attrs = doc->attributes();
    QString ID = doc->attributes().value(KD4TagID).toString();
    QString name = attrs.value(KD4TagName).toString();
    QString group = attrs.value(KD4TagGroup).toString();

    QLCChannel *chan = new QLCChannel();
    chan->setName(name);
    chan->setGroup(getGroup(ID, name, group));
    chan->setColour(getColour(ID, name, group));
    chan->setControlByte(QLCChannel::MSB);

    // add channel to fixture definition
    fixtureDef->addChannel(chan);
    m_channels.insert(ID, chan);

    // if this channel is a NoGroup then we don't need to continue
    // no capabilities nor 16 bit channel
    if (chan->group() == QLCChannel::NoGroup)
    {
        doc->skipCurrentElement();
        return true;
    }

    while (doc->readNextStartElement())
    {
        if (doc->name() == KD4TagFunction)
        {
            parseFunction(doc, fixtureDef, chan, ID, group);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown attribute tag:" << doc->name().toString();
            doc->skipCurrentElement();
        }
    }
    chan->sortCapabilities();

    return true;
}

bool AvolitesD4Parser::parseMode(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef)
{
    if (doc->name() != KD4TagMode)
        return false;

    QString name = doc->attributes().value(KD4TagModeName).toString();

    if (name.isEmpty())
        return false;

    QLCFixtureMode *mode = new QLCFixtureMode(fixtureDef);
    mode->setName(name);

    while (doc->readNextStartElement())
    {
        if (doc->name() == KD4TagModeInclude)
        {
            parseInclude(doc, mode);
        }
        else if (doc->name() == KD4TagPhysical)
        {
            // Parse physical
            parsePhysical(doc, fixtureDef, mode);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown mode tag:" << doc->name().toString();
            doc->skipCurrentElement();
        }
    }

    // Add the mode
    fixtureDef->addMode(mode);

    return true;
}

bool AvolitesD4Parser::comparePhysical(const QLCPhysical &globalPhy, const QLCPhysical &modePhy) const
{
    if (globalPhy.isEmpty())
        return true;

    if (globalPhy.bulbLumens() != modePhy.bulbLumens() ||
        globalPhy.bulbColourTemperature() != modePhy.bulbColourTemperature() ||
        globalPhy.weight() != modePhy.weight() ||
        globalPhy.width() != modePhy.width() ||
        globalPhy.height() != modePhy.height() ||
        globalPhy.depth() != modePhy.depth() ||
        globalPhy.lensDegreesMin() != modePhy.lensDegreesMin() ||
        globalPhy.lensDegreesMax() != modePhy.lensDegreesMax() ||
        globalPhy.focusPanMax() != modePhy.focusPanMax() ||
        globalPhy.focusTiltMax() != modePhy.focusTiltMax() ||
        globalPhy.powerConsumption() != modePhy.powerConsumption())
            return false;

    return true;
}

void AvolitesD4Parser::parsePhysical(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef, QLCFixtureMode *mode)
{
    if (doc->name() != KD4TagPhysical)
        return;

    QLCPhysical phys;

    while (doc->readNextStartElement())
    {
        QXmlStreamAttributes attrs = doc->attributes();

        if (doc->name() == KD4TagPhysicalBulb)
        {
            phys.setBulbType(attrs.value(KD4TagPhysicalBulbType).toString());
            phys.setBulbLumens(attrs.value(KD4TagPhysicalBulbLumens).toString().toInt());
            // this is kind of wrong cause a ColourTemp tag can be "0, 0, 0"
            phys.setBulbColourTemperature(attrs.value(KD4TagPhysicalBulbColourTemp).toString().toInt());
        }
        else if (doc->name() == KD4TagPhysicalLens)
        {
            phys.setLensName(attrs.value(KD4TagName).toString());

            QString degrees = attrs.value(KD4TagPhysicalLensDegrees).toString();
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
        }
        else if (doc->name() == KD4TagPhysicalWeight)
        {
            phys.setWeight(attrs.value(KD4TagPhysicalWeightKg).toString().toDouble());
        }
        else if (doc->name() == KD4TagPhysicalSize)
        {
            phys.setHeight(attrs.value(KD4TagPhysicalSizeHeight).toString().toDouble() * 1000);
            phys.setWidth(attrs.value(KD4TagPhysicalSizeWidth).toString().toDouble() * 1000);
            phys.setDepth(attrs.value(KD4TagPhysicalSizeDepth).toString().toDouble() * 1000);
        }
        else if (doc->name() == KD4TagPhysicalFocus)
        {
            phys.setFocusType(attrs.value(KD4TagPhysicalFocusType).toString());
            phys.setFocusPanMax(attrs.value(KD4TagPhysicalFocusPanMax).toString().toInt());
            phys.setFocusTiltMax(attrs.value(KD4TagPhysicalFocusTiltMax).toString().toInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown physical tag:" << doc->name().toString();
        }
        doc->skipCurrentElement();
    }

    if (comparePhysical(fixtureDef->physical(), phys) == true)
        fixtureDef->setPhysical(phys);
    else
        mode->setPhysical(phys);
}

void AvolitesD4Parser::parseInclude(QXmlStreamReader *doc, QLCFixtureMode *mode)
{
    if (doc->name() != KD4TagModeInclude)
        return;

    QMap <int, QLCChannel*> channelList;

    // loop through Attribute tags
    while (doc->readNextStartElement())
    {
        if (doc->name() == KD4TagModeAttribute)
        {
            QXmlStreamAttributes attrs = doc->attributes();
            // Some channels are conditionals not real channels
            if (attrs.value(KD4TagModeChannelOffset).toString().isEmpty())
            {
                doc->skipCurrentElement();
                continue;
            }

            QString modeID = attrs.value(KD4TagModeID).toString();
            if (m_channels.contains(modeID))
            {
                // might be a 16 bit channel, so we have 2 offsets
                QString offset = attrs.value(KD4TagModeChannelOffset).toString();
                if (offset.contains(KD4TagModeChannelSeparator, Qt::CaseInsensitive))
                {
                    // 16 bit address, we need to add 2 channels, this one, and we need the fine one
                    QStringList offsetValues = offset.split(KD4TagModeChannelSeparator);
                    // if there's more than 2 addresses, or less than 2, bail out, don't know how to handle this, shouldn't happen ever.
                    if (offsetValues.size() > 2 || offsetValues.size() < 2)
                        continue;

                    // Add this one
                    channelList.insert(offsetValues.value(0).toInt(), m_channels.value(modeID));
                    QString name = m_channels.value(modeID)->name();

                    // Search for the fine one
                    QMapIterator <QString,QLCChannel*> it(m_channels);
                    while (it.hasNext() == true)
                    {
                        it.next();
                        QLCChannel *ch(it.value());
                        Q_ASSERT(ch != NULL);

                        if (ch->name() == QString(name + " Fine"))
                            channelList.insert(offsetValues.value(1).toInt(), ch);
                    }
                }
                else
                {
                    channelList.insert(offset.toInt(), m_channels.value(modeID));
                }
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown include tag:" << doc->name().toString();
        }
        doc->skipCurrentElement();
    }

    QMapIterator <int,QLCChannel*> it(channelList);
    while (it.hasNext() == true)
    {
        it.next();
        Q_ASSERT(mode != NULL);
        mode->insertChannel(it.value(), it.key());
    }
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

QLCFixtureDef::FixtureType AvolitesD4Parser::guessType(QLCFixtureDef *def) const
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
        const QLCChannel *ch(it.next());
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
        return QLCFixtureDef::MovingHead; // Quite probable, few scanners with 16bit addressing
    else if (pan == 1 && tilt == 1)
        return QLCFixtureDef::Scanner; // Quite probable, though some moving heads are only 8bit
    else if (gobo > 0)
        return QLCFixtureDef::Flower; // No pan/tilt, but gobo, fairly certain
    else if (colour > 0 || (r > 0 && g > 0 && b > 0) || (c > 0 && m > 0 && y > 0))
        return QLCFixtureDef::ColorChanger; // No pan/tilt/gobos, but RGB/CMY mixing or dichro
    else if (strobe > 0)
        return QLCFixtureDef::Strobe; // Duh.
    else if (smoke > 0)
        return QLCFixtureDef::Smoke; // Duh.
    else if (haze > 0)
        return QLCFixtureDef::Hazer; // Duh.
    else if (nocol > 0)
        return QLCFixtureDef::Dimmer; // Kinda..mmmmh..
    else
        return QLCFixtureDef::Other; // Give up
}
