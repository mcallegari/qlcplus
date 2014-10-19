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

#include <QString>
#include <QtXml>
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
    QDomDocument doc(QLCFile::readXML(path));
    if (doc.isNull() == true)
    {
        qWarning() << Q_FUNC_INFO << "Unable to load input profile from" << path;
        return NULL;
    }

    QLCInputProfile* profile = new QLCInputProfile();
    if (profile->loadXML(doc) == false)
    {
        delete profile;
        profile = NULL;
    }
    else
    {
        profile->m_path = path;
    }

    return profile;
}

bool QLCInputProfile::loadXML(const QDomDocument& doc)
{
    QDomElement root = doc.documentElement();
    if (root.tagName() == KXMLQLCInputProfile)
    {
        QDomNode node = root.firstChild();
        while (node.isNull() == false)
        {
            QDomElement tag = node.toElement();
            if (tag.tagName() == KXMLQLCCreator)
            {
                /* Ignore */
            }
            if (tag.tagName() == KXMLQLCInputProfileManufacturer)
            {
                setManufacturer(tag.text());
            }
            else if (tag.tagName() == KXMLQLCInputProfileModel)
            {
                setModel(tag.text());
            }
            else if (tag.tagName() == KXMLQLCInputProfileType)
            {
                setType(stringToType(tag.text()));
            }
            else if (tag.tagName() == KXMLQLCInputChannel)
            {
                QString str = tag.attribute(KXMLQLCInputChannelNumber);
                if (str.isEmpty() == false)
                {
                    quint32 ch = str.toInt();
                    QLCInputChannel* ich = new QLCInputChannel();
                    if (ich->loadXML(tag) == true)
                        insertChannel(ch, ich);
                    else
                        delete ich;
                }
            }

            node = node.nextSibling();
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

    QDomDocument doc(QLCFile::getXMLHeader(KXMLQLCInputProfile));
    Q_ASSERT(doc.isNull() == false);

    /* Create a text stream for the file */
    QTextStream stream(&file);

    /* THE MASTER XML ROOT NODE */
    QDomElement root = doc.documentElement();

    /* Manufacturer */
    QDomElement tag = doc.createElement(KXMLQLCInputProfileManufacturer);
    root.appendChild(tag);
    QDomText text = doc.createTextNode(m_manufacturer);
    tag.appendChild(text);

    /* Model */
    tag = doc.createElement(KXMLQLCInputProfileModel);
    root.appendChild(tag);
    text = doc.createTextNode(m_model);
    tag.appendChild(text);

    /* Type */
    tag = doc.createElement(KXMLQLCInputProfileType);
    root.appendChild(tag);
    text = doc.createTextNode(typeToString(m_type));
    tag.appendChild(text);

    /* Write channels to the document */
    QMapIterator <quint32, QLCInputChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        it.next();
        it.value()->saveXML(&doc, &root, it.key());
    }

    /* Write the document into the stream */
    m_path = fileName;
    stream << doc.toString();
    file.close();

    return true;
}
