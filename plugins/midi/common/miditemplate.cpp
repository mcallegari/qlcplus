/*
  Q Light Controller
  miditemplate.cpp

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

#include <QStringList>
#include <QString>
#include <QHash>
#include <QMap>
#include <QtXml>

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
    QDomDocument doc(QLCFile::readXML(path));
    if (doc.isNull() == true)
    {
        qWarning() << Q_FUNC_INFO << "Unable to load midi template from" << path;
        return NULL;
    }

    MidiTemplate* midiTemplate = new MidiTemplate();
    if (midiTemplate->loadXML(doc) == false)
    {
        delete midiTemplate;
        midiTemplate = NULL;
    }

    return midiTemplate;
}

bool MidiTemplate::loadXML(const QDomDocument& doc)
{
    QDomElement root = doc.documentElement();
    if (root.tagName() == KXMLMidiTemplate)
    {
        QDomNode node = root.firstChild();
        while (node.isNull() == false)
        {
            QDomElement tag = node.toElement();
            if (tag.tagName() == KXMLQLCCreator)
            {
                /* Ignore */
            }
            if (tag.tagName() == KXMLMidiTemplateDescription)
            {
                /* Ignore */
            }
            if (tag.tagName() == KXMLMidiTemplateName)
            {
                setName(tag.text());
            }
            else if (tag.tagName() == KXMLMidiTemplateInitMessage)
            {
                QByteArray initMessage;
                initMessage.append(tag.text());
                setInitMessage(initMessage);
            }

            node = node.nextSibling();
        }

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Midi Template not found";
        return false;
    }
}
