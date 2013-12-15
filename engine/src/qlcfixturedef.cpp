/*
  Q Light Controller
  qlcfixturedef.cpp

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

#include <iostream>
#include <QString>
#include <QFile>
#include <QtXml>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "qlcfile.h"

QLCFixtureDef::QLCFixtureDef()
{
    m_isLoaded = false;
    m_defFileAbsolutePath = QString();
    m_type = QString("Dimmer");
}

QLCFixtureDef::QLCFixtureDef(const QLCFixtureDef* fixtureDef)
{
    m_isLoaded = false;
    m_defFileAbsolutePath = QString();
    m_type = QString("Dimmer");

    if (fixtureDef != NULL)
        *this = *fixtureDef;
}

QLCFixtureDef::~QLCFixtureDef()
{
    while (m_channels.isEmpty() == false)
        delete m_channels.takeFirst();

    while (m_modes.isEmpty() == false)
        delete m_modes.takeFirst();
}

QLCFixtureDef& QLCFixtureDef::operator=(const QLCFixtureDef& fixture)
{
    if (this != &fixture)
    {
        QListIterator <QLCChannel*> chit(fixture.m_channels);
        QListIterator <QLCFixtureMode*> modeit(fixture.m_modes);

        m_manufacturer = fixture.m_manufacturer;
        m_model = fixture.m_model;
        m_type = fixture.m_type;
        m_author = fixture.m_author;

        /* Clear all channels */
        while (m_channels.isEmpty() == false)
            delete m_channels.takeFirst();

        /* Copy channels from the other fixture */
        while (chit.hasNext() == true)
            m_channels.append(new QLCChannel(chit.next()));

        /* Clear all modes */
        while (m_modes.isEmpty() == false)
            delete m_modes.takeFirst();

        /* Copy modes from the other fixture */
        while (modeit.hasNext() == true)
            m_modes.append(new QLCFixtureMode(this, modeit.next()));
    }

    return *this;
}

void QLCFixtureDef::setDefinitionSourceFile(const QString &absPath)
{
    m_defFileAbsolutePath = absPath;
    m_isLoaded = false;
}

/****************************************************************************
 * General properties
 ****************************************************************************/

QString QLCFixtureDef::name() const
{
    return m_manufacturer + QString(" ") + m_model;
}

void QLCFixtureDef::setManufacturer(const QString& mfg)
{
    m_manufacturer = mfg;
}

QString QLCFixtureDef::manufacturer() const
{
    return m_manufacturer;
}

void QLCFixtureDef::setModel(const QString& model)
{
    m_model = model;
}

QString QLCFixtureDef::model() const
{
    return m_model;
}

void QLCFixtureDef::setType(const QString& type)
{
    m_type = type;
}

QString QLCFixtureDef::type()
{
    checkLoaded();
    return m_type;
}

void QLCFixtureDef::setAuthor(const QString& author)
{
    m_author = author;
}

QString QLCFixtureDef::author()
{
    checkLoaded();
    return m_author;
}

void QLCFixtureDef::checkLoaded()
{
    // Already loaded ? Nothing to do
    if (m_isLoaded == true)
        return;

    if (m_isLoaded == false)
    {
        if (m_defFileAbsolutePath.isEmpty())
        {
            qWarning() << Q_FUNC_INFO << "Empty file path provided ! This is a trouble.";
            return;
        }
        qDebug() << "Loading fixture definition now... " << m_defFileAbsolutePath;
        bool error = loadXML(m_defFileAbsolutePath);
        if (error == false)
        {
            m_isLoaded = true;
            m_defFileAbsolutePath = QString();
        }
    }
}

/****************************************************************************
 * Channels
 ****************************************************************************/

bool QLCFixtureDef::addChannel(QLCChannel* channel)
{
    if (channel != NULL && m_channels.contains(channel) == false)
    {
        m_channels.append(channel);
        return true;
    }
    else
    {
        return false;
    }
}

bool QLCFixtureDef::removeChannel(QLCChannel* channel)
{
    /* First remove the channel from all modes */
    QListIterator <QLCFixtureMode*> modeit(m_modes);
    while (modeit.hasNext() == true)
        modeit.next()->removeChannel(channel);

    /* Then remove the actual channel from this fixture definition */
    QMutableListIterator <QLCChannel*> chit(m_channels);
    while (chit.hasNext() == true)
    {
        if (chit.next() == channel)
        {
            chit.remove();
            delete channel;
            return true;
        }
    }

    return false;
}

QLCChannel* QLCFixtureDef::channel(const QString& name)
{
    QListIterator <QLCChannel*> it(m_channels);
    QLCChannel* ch = NULL;

    while (it.hasNext() == true)
    {
        ch = it.next();
        if (ch->name() == name)
            return ch;
    }

    return NULL;
}

QList <QLCChannel*> QLCFixtureDef::channels() const
{
    return m_channels;
}

/****************************************************************************
 * Modes
 ****************************************************************************/

bool QLCFixtureDef::addMode(QLCFixtureMode* mode)
{
    if (mode != NULL && m_modes.contains(mode) == false)
    {
        m_modes.append(mode);
        return true;
    }
    else
    {
        return false;
    }
}

bool QLCFixtureDef::removeMode(QLCFixtureMode* mode)
{
    QMutableListIterator <QLCFixtureMode*> it(m_modes);
    while (it.hasNext() == true)
    {
        if (it.next() == mode)
        {
            it.remove();
            delete mode;
            return true;
        }
    }

    return false;
}

QLCFixtureMode *QLCFixtureDef::mode(const QString& name)
{
    checkLoaded();
    QListIterator <QLCFixtureMode*> it(m_modes);
    QLCFixtureMode* mode = NULL;

    while (it.hasNext() == true)
    {
        mode = it.next();
        if (mode->name() == name)
            return mode;
    }

    return NULL;
}

QList <QLCFixtureMode*> QLCFixtureDef::modes()
{
    checkLoaded();
    return m_modes;
}

/****************************************************************************
 * XML operations
 ****************************************************************************/

QFile::FileError QLCFixtureDef::saveXML(const QString& fileName)
{
    QFile::FileError error;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) == false)
        return file.error();

    QDomDocument doc(QLCFile::getXMLHeader(KXMLQLCFixtureDefDocument, author()));
    Q_ASSERT(doc.isNull() == false);

    /* Create a text stream for the file */
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    stream.setCodec("UTF-8");

    /* Fixture tag */
    QDomElement root = doc.documentElement();

    QDomElement tag;
    QDomText text;

    /* Manufacturer */
    tag = doc.createElement(KXMLQLCFixtureDefManufacturer);
    root.appendChild(tag);
    text = doc.createTextNode(m_manufacturer);
    tag.appendChild(text);

    /* Model */
    tag = doc.createElement(KXMLQLCFixtureDefModel);
    root.appendChild(tag);
    text = doc.createTextNode(m_model);
    tag.appendChild(text);

    /* Type */
    tag = doc.createElement(KXMLQLCFixtureDefType);
    root.appendChild(tag);
    text = doc.createTextNode(m_type);
    tag.appendChild(text);

    /* Channels */
    QListIterator <QLCChannel*> chit(m_channels);
    while (chit.hasNext() == true)
        chit.next()->saveXML(&doc, &root);

    /* Modes */
    QListIterator <QLCFixtureMode*> modeit(m_modes);
    while (modeit.hasNext() == true)
        modeit.next()->saveXML(&doc, &root);

    /* Write the document into the stream */
    stream << doc.toString();
    error = QFile::NoError;
    file.close();

    return error;
}

QFile::FileError QLCFixtureDef::loadXML(const QString& fileName)
{
    QFile::FileError error = QFile::NoError;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QDomDocument doc = QLCFile::readXML(fileName);
    if (doc.isNull() == true)
    {
        qWarning() << Q_FUNC_INFO << "Unable to read from" << fileName;
        return QFile::ReadError;
    }

    if (doc.doctype().name() == KXMLQLCFixtureDefDocument)
    {
        qDebug() << Q_FUNC_INFO << "Loading " << fileName;
        if (loadXML(doc) == true)
            error = QFile::NoError;
        else
            error = QFile::ReadError;
    }
    else
    {
        error = QFile::ReadError;
        qWarning() << Q_FUNC_INFO << fileName
                   << "is not a fixture definition file";
    }

    return error;
}

bool QLCFixtureDef::loadXML(const QDomDocument& doc)
{
    bool retval = false;

    QDomElement root = doc.documentElement();
    if (root.tagName() == KXMLQLCFixtureDef)
    {
        QDomNode node = root.firstChild();
        while (node.isNull() == false)
        {
            QDomElement tag = node.toElement();
            if (tag.tagName() == KXMLQLCCreator)
            {
                loadCreator(tag);
            }
            else if (tag.tagName() == KXMLQLCFixtureDefManufacturer)
            {
                setManufacturer(tag.text());
            }
            else if (tag.tagName() == KXMLQLCFixtureDefModel)
            {
                setModel(tag.text());
            }
            else if (tag.tagName() == KXMLQLCFixtureDefType)
            {
                setType(tag.text());
            }
            else if (tag.tagName() == KXMLQLCChannel)
            {
                QLCChannel* ch = new QLCChannel();
                if (ch->loadXML(tag) == true)
                {
                    /* Loading succeeded */
                    if (addChannel(ch) == false)
                    {
                        /* Channel already exists */
                        delete ch;
                    }
                }
                else
                {
                    /* Loading failed */
                    delete ch;
                }
            }
            else if (tag.tagName() == KXMLQLCFixtureMode)
            {
                QLCFixtureMode* mode = new QLCFixtureMode(this);
                if (mode->loadXML(tag) == true)
                {
                    /* Loading succeeded */
                    if (addMode(mode) == false)
                    {
                        /* Mode already exists */
                        delete mode;
                    }
                }
                else
                {
                    /* Loading failed */
                    delete mode;
                }
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Unknown Fixture tag: " << tag.tagName();
            }

            node = node.nextSibling();
        }

        retval = true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Fixture node not found";
        retval = false;
    }

    if (retval == true)
        m_isLoaded = true;
    return retval;
}

bool QLCFixtureDef::loadCreator(const QDomElement& creator)
{
    QDomNode node;
    QDomElement tag;

    if (creator.tagName() != KXMLQLCCreator)
    {
        qWarning() << Q_FUNC_INFO << "file creator information not found!";
        return false;
    }

    node = creator.firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCCreatorName)
        {
            /* Ignore name */
        }
        else if (tag.tagName() == KXMLQLCCreatorVersion)
        {
            /* Ignore version */
        }
        else if (tag.tagName() == KXMLQLCCreatorAuthor)
        {
            setAuthor(tag.text());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "unknown creator tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}
