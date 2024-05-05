/*
  Q Light Controller Plus
  qlcfixturedef.cpp

  Copyright (c) Heikki Junnila
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
#include <QXmlStreamWriter>
#include <iostream>
#include <QString>
#include <QDebug>
#include <QFile>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"
#include "qlcfile.h"
#include "fixture.h"

QLCFixtureDef::QLCFixtureDef()
    : m_isLoaded(false)
    , m_isUser(false)
    , m_fileAbsolutePath(QString())
    , m_type(Dimmer)
{
}

QLCFixtureDef::QLCFixtureDef(const QLCFixtureDef* fixtureDef)
    : m_isLoaded(false)
    , m_isUser(false)
    , m_fileAbsolutePath(QString())
    , m_type(Dimmer)
{
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
            m_channels.append(chit.next()->createCopy());

        /* Clear all modes */
        while (m_modes.isEmpty() == false)
            delete m_modes.takeFirst();

        /* Copy modes from the other fixture */
        while (modeit.hasNext() == true)
            m_modes.append(new QLCFixtureMode(this, modeit.next()));
    }

    return *this;
}

QString QLCFixtureDef::definitionSourceFile() const
{
    return m_fileAbsolutePath;
}

void QLCFixtureDef::setDefinitionSourceFile(const QString &absPath)
{
    m_fileAbsolutePath = absPath;
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

void QLCFixtureDef::setType(const FixtureType type)
{
    m_type = type;
}

QLCFixtureDef::FixtureType QLCFixtureDef::type()
{
    return m_type;
}

QLCFixtureDef::FixtureType QLCFixtureDef::stringToType(const QString& type)
{
    if (type == "Color Changer") return ColorChanger;
    else if (type == "Dimmer") return Dimmer;
    else if (type == "Effect") return Effect;
    else if (type == "Fan") return Fan;
    else if (type == "Flower") return Flower;
    else if (type == "Hazer") return Hazer;
    else if (type == "Laser") return Laser;
    else if (type == "Moving Head") return MovingHead;
    else if (type == "Scanner") return Scanner;
    else if (type == "Smoke") return Smoke;
    else if (type == "Strobe") return Strobe;
    else if (type == "LED Bar (Beams)") return LEDBarBeams;
    else if (type == "LED Bar (Pixels)") return LEDBarPixels;

    return Other;
}

QString QLCFixtureDef::typeToString(QLCFixtureDef::FixtureType type)
{
    switch(type)
    {
        case ColorChanger: return "Color Changer";
        case Dimmer: return "Dimmer";
        case Effect: return "Effect";
        case Fan: return "Fan";
        case Flower: return "Flower";
        case Hazer: return "Hazer";
        case Laser: return "Laser";
        case MovingHead: return "Moving Head";
        case Scanner: return "Scanner";
        case Smoke: return "Smoke";
        case Strobe: return "Strobe";
        case LEDBarBeams: return "LED Bar (Beams)";
        case LEDBarPixels: return "LED Bar (Pixels)";
        default: return "Other";
    }
}

void QLCFixtureDef::setAuthor(const QString& author)
{
    m_author = author;
}

QString QLCFixtureDef::author()
{
    return m_author;
}

void QLCFixtureDef::checkLoaded(QString mapPath)
{
    // Already loaded ? Nothing to do
    if (m_isLoaded == true)
        return;

    if (manufacturer() == KXMLFixtureGeneric &&
       (model() == KXMLFixtureGeneric || model() == KXMLFixtureRGBPanel))
    {
        m_isLoaded = true;
        return;
    }
    if (m_fileAbsolutePath.isEmpty())
    {
        qWarning() << Q_FUNC_INFO << "Empty file path provided! This is a trouble.";
        return;
    }

    // check if path is relative (from map) or absolute (user def)
    QDir defPath(m_fileAbsolutePath);
    if (defPath.isRelative())
        m_fileAbsolutePath = QString("%1%2%3").arg(mapPath).arg(QDir::separator()).arg(m_fileAbsolutePath);

    qDebug() << "Loading fixture definition now... " << m_fileAbsolutePath;
    bool error = loadXML(m_fileAbsolutePath);
    if (error == false)
        m_isLoaded = true;
}

void QLCFixtureDef::setLoaded(bool loaded)
{
    m_isLoaded = loaded;
}

bool QLCFixtureDef::isUser() const
{
    return m_isUser;
}

void QLCFixtureDef::setIsUser(bool flag)
{
    m_isUser = flag;
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

    while (it.hasNext() == true)
    {
        QLCChannel* ch = it.next();
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
    QListIterator <QLCFixtureMode*> it(m_modes);

    while (it.hasNext() == true)
    {
        QLCFixtureMode *mode = it.next();
        if (mode->name() == name)
            return mode;
    }

    return NULL;
}

QList <QLCFixtureMode*> QLCFixtureDef::modes()
{
    return m_modes;
}

/****************************************************************************
 * Physical
 ****************************************************************************/

void QLCFixtureDef::setPhysical(const QLCPhysical& physical)
{
    m_physical = physical;
}

QLCPhysical QLCFixtureDef::physical() const
{
    return m_physical;
}

/****************************************************************************
 * XML operations
 ****************************************************************************/

QFile::FileError QLCFixtureDef::saveXML(const QString& fileName)
{
    QFile::FileError error;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QString tempFileName(fileName);
    tempFileName += ".temp";
    QFile file(tempFileName);
    if (file.open(QIODevice::WriteOnly) == false)
        return file.error();

    QXmlStreamWriter doc(&file);
    doc.setAutoFormatting(true);
    doc.setAutoFormattingIndent(1);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    doc.setCodec("UTF-8");
#endif
    QLCFile::writeXMLHeader(&doc, KXMLQLCFixtureDefDocument, author());

    doc.writeTextElement(KXMLQLCFixtureDefManufacturer, m_manufacturer);
    doc.writeTextElement(KXMLQLCFixtureDefModel, m_model);
    doc.writeTextElement(KXMLQLCFixtureDefType, typeToString(m_type));

    /* Channels */
    QListIterator <QLCChannel*> chit(m_channels);
    while (chit.hasNext() == true)
        chit.next()->saveXML(&doc);

    /* Modes */
    QListIterator <QLCFixtureMode*> modeit(m_modes);
    while (modeit.hasNext() == true)
        modeit.next()->saveXML(&doc);

    m_physical.saveXML(&doc);

    /* End the document and close all the open elements */
    error = QFile::NoError;
    doc.writeEndDocument();
    file.close();

    // Save to actual requested file name
    QFile currFile(fileName);
    if (currFile.exists() && !currFile.remove())
    {
        qWarning() << "Could not erase" << fileName;
        return currFile.error();
    }
    if (!file.rename(fileName))
    {
        qWarning() << "Could not rename" << tempFileName << "to" << fileName;
        return file.error();
    }

    return error;
}

QFile::FileError QLCFixtureDef::loadXML(const QString& fileName)
{
    QFile::FileError error = QFile::NoError;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QXmlStreamReader *doc = QLCFile::getXMLReader(fileName);
    if (doc == NULL || doc->device() == NULL || doc->hasError())
    {
        qWarning() << Q_FUNC_INFO << "Unable to read from" << fileName;
        return QFile::ReadError;
    }

    while (!doc->atEnd())
    {
        if (doc->readNext() == QXmlStreamReader::DTD)
            break;
    }
    if (doc->hasError())
    {
        QLCFile::releaseXMLReader(doc);
        return QFile::ResourceError;
    }

    if (doc->dtdName() == KXMLQLCFixtureDefDocument)
    {
        // qDebug() << Q_FUNC_INFO << "Loading " << fileName;
        if (loadXML(*doc) == true)
            error = QFile::NoError;
        else
        {
            qWarning() << fileName << QString("%1\nLine %2, column %3")
                        .arg(doc->errorString())
                        .arg(doc->lineNumber())
                        .arg(doc->columnNumber());
            error = QFile::ReadError;
        }
    }
    else
    {
        error = QFile::ReadError;
        qWarning() << Q_FUNC_INFO << fileName
                   << "is not a fixture definition file";
    }

    QLCFile::releaseXMLReader(doc);

    return error;
}

bool QLCFixtureDef::loadXML(QXmlStreamReader& doc)
{
    bool retval = false;

    if (doc.readNextStartElement() == false)
        return false;

    if (doc.name() == KXMLQLCFixtureDef)
    {
        while (doc.readNextStartElement())
        {
            if (doc.name() == KXMLQLCCreator)
            {
                loadCreator(doc);
            }
            else if (doc.name() == KXMLQLCFixtureDefManufacturer)
            {
                setManufacturer(doc.readElementText());
            }
            else if (doc.name() == KXMLQLCFixtureDefModel)
            {
                setModel(doc.readElementText());
            }
            else if (doc.name() == KXMLQLCFixtureDefType)
            {
                setType(stringToType(doc.readElementText()));
            }
            else if (doc.name() == KXMLQLCChannel)
            {
                QLCChannel* ch = new QLCChannel();
                if (ch->loadXML(doc) == true)
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
            else if (doc.name() == KXMLQLCFixtureMode)
            {
                QLCFixtureMode* mode = new QLCFixtureMode(this);
                if (mode->loadXML(doc) == true)
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
            else if (doc.name() == KXMLQLCPhysical)
            {
                /* Global physical */
                QLCPhysical physical;
                physical.loadXML(doc);
                setPhysical(physical);
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Unknown Fixture tag: " << doc.name();
                doc.skipCurrentElement();
            }
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

bool QLCFixtureDef::loadCreator(QXmlStreamReader &doc)
{
    if (doc.name() != KXMLQLCCreator)
    {
        qWarning() << Q_FUNC_INFO << "file creator information not found!";
        return false;
    }

    while (doc.readNextStartElement())
    {
        if (doc.name() == KXMLQLCCreatorName)
        {
            /* Ignore name */
            doc.skipCurrentElement();
        }
        else if (doc.name() == KXMLQLCCreatorVersion)
        {
            /* Ignore version */
            doc.skipCurrentElement();
        }
        else if (doc.name() == KXMLQLCCreatorAuthor)
        {
            setAuthor(doc.readElementText());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "unknown creator tag:" << doc.name();
            doc.skipCurrentElement();
        }
    }

    return true;
}
