/*
  Q Light Controller Plus
  miditemplate.cpp

  Copyright (c) Joep Admiraal

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

#include <QXmlStreamReader>
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QHash>
#include <QMap>

#include "miditemplate.h"
#include "qlcfile.h"

class QLCFile;

/****************************************************************************
 * Initialization
 ****************************************************************************/

MidiTemplate::MidiTemplate()
{
}

MidiTemplate::MidiTemplate(const MidiTemplate& templ)
{
    *this = templ;
}

MidiTemplate::~MidiTemplate()
{
}

MidiTemplate& MidiTemplate::operator=(const MidiTemplate& templ)
{
    if (this != &templ)
    {
        /* Copy basic properties */
        m_description = templ.m_description;
        m_initMessage = templ.m_initMessage;
    }

    return *this;
}

void MidiTemplate::setName(const QString& description)
{
    m_description = description;
}

QString MidiTemplate::name() const
{
    return m_description;
}

void MidiTemplate::setInitMessage(const QByteArray& message)
{
    m_initMessage = message;
}

QByteArray MidiTemplate::initMessage() const
{
    return m_initMessage;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

MidiTemplate* MidiTemplate::loader(const QString& path)
{
    QXmlStreamReader *doc = QLCFile::getXMLReader(path);
    if (doc == NULL || doc->device() == NULL || doc->hasError())
    {
        qWarning() << Q_FUNC_INFO << "Unable to load midi template from" << path;
        return NULL;
    }

    MidiTemplate* midiTemplate = new MidiTemplate();
    if (midiTemplate->loadXML(*doc) == false)
    {
        qWarning() << path << QString("%1\nLine %2, column %3")
                    .arg(doc->errorString())
                    .arg(doc->lineNumber())
                    .arg(doc->columnNumber());

        delete midiTemplate;
        midiTemplate = NULL;
    }
    doc->device()->close();
    delete doc->device();
    delete doc;

    return midiTemplate;
}

bool MidiTemplate::loadXML(QXmlStreamReader& doc)
{
    if (doc.readNextStartElement() == false)
        return false;

    if (doc.name() == KXMLMidiTemplate)
    {
        while (doc.readNextStartElement())
        {
            if (doc.name() == KXMLQLCCreator)
            {
                /* Ignore */
                doc.skipCurrentElement();
            }
            if (doc.name() == KXMLMidiTemplateDescription)
            {
                /* Ignore */
                doc.skipCurrentElement();
            }
            if (doc.name() == KXMLMidiTemplateName)
            {
                setName(doc.readElementText());
            }
            else if (doc.name() == KXMLMidiTemplateInitMessage)
            {
                QByteArray initMessage;
                QStringList byteList = doc.readElementText().split(' ');
                for (int i = 0; i < byteList.count(); i++)
                {
                    bool ok;
                    int byte = byteList.at(i).toInt(&ok , 16);
                    initMessage.append((char)byte);
                }
                setInitMessage(initMessage);
                qDebug() << Q_FUNC_INFO << "Loaded message with size:" << initMessage.count();
            }
        }

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Midi Template not found";
        return false;
    }
}
