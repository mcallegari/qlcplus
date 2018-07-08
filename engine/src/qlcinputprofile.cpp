/*
  Q Light Controller
  qlcinputprofile.cpp

  Copyright (c) Heikki Junnila

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
#include <QXmlStreamWriter>
#include <QString>
#include <QDebug>
#include <QMap>

#include "qlcinputchannel.h"
#include "qlcinputprofile.h"
#include "qlcchannel.h"
#include "qlcfile.h"

#define KXMLQLCInputProfileTypeMidi "MIDI"
#define KXMLQLCInputProfileTypeOsc "OSC"
#define KXMLQLCInputProfileTypeHid "HID"
#define KXMLQLCInputProfileTypeDmx "DMX"
#define KXMLQLCInputProfileTypeEnttec "Enttec"


/****************************************************************************
 * Initialization
 ****************************************************************************/

QLCInputProfile::QLCInputProfile()
    : m_manufacturer(QString())
    , m_model(QString())
    , m_path(QString())
    , m_type(Midi)
    , m_midiSendNoteOff(true)
{
}

QLCInputProfile::QLCInputProfile(const QLCInputProfile& profile)
{
    *this = profile;
}

QLCInputProfile::~QLCInputProfile()
{
    destroyChannels();
}

QLCInputProfile& QLCInputProfile::operator=(const QLCInputProfile& profile)
{
    if (this != &profile)
    {
        /* Copy basic properties */
        m_manufacturer = profile.m_manufacturer;
        m_model = profile.m_model;
        m_path = profile.m_path;
        m_type = profile.m_type;
        m_midiSendNoteOff = profile.m_midiSendNoteOff;
        m_globalSettingsMap = profile.m_globalSettingsMap;

        /* Destroy all existing channels */
        destroyChannels();

        /* Copy the other profile's channels */
        QMapIterator <quint32,QLCInputChannel*> it(profile.m_channels);
        while (it.hasNext() == true)
        {
            it.next();
            insertChannel(it.key(), new QLCInputChannel(*it.value()));
        }
    }

    return *this;
}

/****************************************************************************
 * profile information
 ****************************************************************************/

void QLCInputProfile::setManufacturer(const QString& manufacturer)
{
    m_manufacturer = manufacturer;
}

QString QLCInputProfile::manufacturer() const
{
    return m_manufacturer;
}

void QLCInputProfile::setModel(const QString& model)
{
    m_model = model;
}

QString QLCInputProfile::model() const
{
    return m_model;
}

QString QLCInputProfile::name() const
{
    return QString("%1 %2").arg(m_manufacturer).arg(m_model);
}

QString QLCInputProfile::path() const
{
    return m_path;
}

void QLCInputProfile::setType(QLCInputProfile::Type type)
{
    m_type = type;
}

QLCInputProfile::Type QLCInputProfile::type() const
{
    return m_type;
}

QString QLCInputProfile::typeToString(Type type)
{
    switch (type)
    {
    case Midi:
        return KXMLQLCInputProfileTypeMidi;
    case Osc:
        return KXMLQLCInputProfileTypeOsc;
    case Hid:
        return KXMLQLCInputProfileTypeHid;
    case Dmx:
        return KXMLQLCInputProfileTypeDmx;
    case Enttec:
        return KXMLQLCInputProfileTypeEnttec;
    default:
        return QString();
    }
}

QLCInputProfile::Type QLCInputProfile::stringToType(const QString& str)
{
    if (str == KXMLQLCInputProfileTypeMidi)
        return Midi;
    else if (str == KXMLQLCInputProfileTypeOsc)
        return Osc;
    else if (str == KXMLQLCInputProfileTypeHid)
        return Hid;
    else if (str == KXMLQLCInputProfileTypeDmx)
        return Dmx;
    else // if (str == KXMLQLCInputProfileTypeEnttec)
        return Enttec;
}

QList<QLCInputProfile::Type> QLCInputProfile::types()
{
    QList<Type> result;
    result 
        << Midi 
        << Osc
        << Hid
        << Dmx
        << Enttec;
    return result;
}

/********************************************************************
 * Plugin-specific global settings
 ********************************************************************/

void QLCInputProfile::setMidiSendNoteOff(bool enable)
{
    m_midiSendNoteOff = enable;
    m_globalSettingsMap["MIDISendNoteOff"] = QVariant(enable);
}

bool QLCInputProfile::midiSendNoteOff() const
{
    return m_midiSendNoteOff;
}

QMap<QString, QVariant> QLCInputProfile::globalSettings() const
{
    return m_globalSettingsMap;
}

/****************************************************************************
 * Channels
 ****************************************************************************/

bool QLCInputProfile::insertChannel(quint32 channel,
                                    QLCInputChannel* ich)
{
    if (ich != NULL && m_channels.contains(channel) == false)
    {
        m_channels.insert(channel, ich);
        return true;
    }
    else
    {
        return false;
    }
}

bool QLCInputProfile::removeChannel(quint32 channel)
{
    if (m_channels.contains(channel) == true)
    {
        QLCInputChannel* ich = m_channels.take(channel);
        Q_ASSERT(ich != NULL);
        delete ich;
        return true;
    }
    else
    {
        return false;
    }
}

bool QLCInputProfile::remapChannel(QLCInputChannel* ich, quint32 number)
{
    if (ich == NULL)
        return false;

    quint32 old = channelNumber(ich);
    if (old != QLCChannel::invalid() && m_channels.contains(number) == false)
    {
        m_channels.take(old);
        insertChannel(number, ich);
        return true;
    }
    else
    {
        return false;
    }
}

QLCInputChannel* QLCInputProfile::channel(quint32 channel) const
{
    if (m_channels.contains(channel) == true)
        return m_channels[channel];
    else
        return NULL;
}

quint32 QLCInputProfile::channelNumber(const QLCInputChannel* channel) const
{
    if (channel == NULL)
        return QLCChannel::invalid();

    QMapIterator <quint32,QLCInputChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        it.next();
        if (it.value() == channel)
            return it.key();
    }

    return QLCChannel::invalid();
}

QMap <quint32,QLCInputChannel*> QLCInputProfile::channels() const
{
    return m_channels;
}

void QLCInputProfile::destroyChannels()
{
    /* Delete existing channels but leave the pointers there */
    QMutableMapIterator <quint32,QLCInputChannel*> it(m_channels);
    while (it.hasNext() == true)
        delete it.next().value();

    /* Clear the list of freed pointers */
    m_channels.clear();
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

QLCInputProfile* QLCInputProfile::loader(const QString& path)
{
    QXmlStreamReader *doc = QLCFile::getXMLReader(path);
    if (doc == NULL || doc->device() == NULL || doc->hasError())
    {
        qWarning() << Q_FUNC_INFO << "Unable to load input profile from" << path;
        return NULL;
    }

    QLCInputProfile* profile = new QLCInputProfile();
    if (profile->loadXML(*doc) == false)
    {
        qWarning() << path << QString("%1\nLine %2, column %3")
                    .arg(doc->errorString())
                    .arg(doc->lineNumber())
                    .arg(doc->columnNumber());

        delete profile;
        profile = NULL;
    }
    else
    {
        profile->m_path = path;
    }

    QLCFile::releaseXMLReader(doc);

    return profile;
}

bool QLCInputProfile::loadXML(QXmlStreamReader& doc)
{
    if (doc.readNextStartElement() == false)
        return false;

    if (doc.name() == KXMLQLCInputProfile)
    {
        while (doc.readNextStartElement())
        {
            if (doc.name() == KXMLQLCCreator)
            {
                /* Ignore this block */
                doc.skipCurrentElement();
            }
            else if (doc.name() == KXMLQLCInputProfileManufacturer)
            {
                setManufacturer(doc.readElementText());
            }
            else if (doc.name() == KXMLQLCInputProfileModel)
            {
                setModel(doc.readElementText());
            }
            else if (doc.name() == KXMLQLCInputProfileType)
            {
                setType(stringToType(doc.readElementText()));
            }
            else if (doc.name() == KXMLQLCInputProfileMidiSendNoteOff)
            {
                if (doc.readElementText() == KXMLQLCFalse)
                    setMidiSendNoteOff(false);
                else
                    setMidiSendNoteOff(true);
            }
            else if (doc.name() == KXMLQLCInputChannel)
            {
                QString str = doc.attributes().value(KXMLQLCInputChannelNumber).toString();
                if (str.isEmpty() == false)
                {
                    quint32 ch = str.toInt();
                    QLCInputChannel* ich = new QLCInputChannel();
                    if (ich->loadXML(doc) == true)
                        insertChannel(ch, ich);
                    else
                        delete ich;
                }
                else
                    doc.skipCurrentElement();
            }
        }

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Input profile not found";
        return false;
    }
}

bool QLCInputProfile::saveXML(const QString& fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) == false)
    {
        qWarning() << Q_FUNC_INFO << "Unable to write to" << fileName;
        return false;
    }

    QXmlStreamWriter doc(&file);
    doc.setAutoFormatting(true);
    doc.setAutoFormattingIndent(1);
    QLCFile::writeXMLHeader(&doc, KXMLQLCInputProfile);

    doc.writeTextElement(KXMLQLCInputProfileManufacturer, m_manufacturer);
    doc.writeTextElement(KXMLQLCInputProfileModel, m_model);
    doc.writeTextElement(KXMLQLCInputProfileType, typeToString(m_type));

    if (midiSendNoteOff() == false)
        doc.writeTextElement(KXMLQLCInputProfileMidiSendNoteOff, QString(KXMLQLCFalse));

    /* Write channels to the document */
    QMapIterator <quint32, QLCInputChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        it.next();
        it.value()->saveXML(&doc, it.key());
    }

    m_path = fileName;
    /* End the document and close all the open elements */
    doc.writeEndDocument();
    file.close();

    return true;
}
