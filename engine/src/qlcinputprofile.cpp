/*
  Q Light Controller
  qlcinputprofile.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QString>
#include <QtXml>
#include <QMap>

#include "qlcinputchannel.h"
#include "qlcinputprofile.h"
#include "qlcfile.h"

#include "inputmap.h"

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
    if (old != InputMap::invalidChannel() && m_channels.contains(number) == false)
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
        return InputMap::invalidChannel();

    QMapIterator <quint32,QLCInputChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        it.next();
        if (it.value() == channel)
            return it.key();
    }

    return InputMap::invalidChannel();
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
