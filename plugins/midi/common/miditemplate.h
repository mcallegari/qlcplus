/*
  Q Light Controller
  qlcmiditemplate.h

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

#ifndef MIDITEMPLATE_H
#define MIDITEMPLATE_H

#include <QStringList>
#include <QString>
#include <QHash>
#include <QMap>
#include <QtXml>


class MidiTemplate;
class QDomDocument;
class QDomElement;

#define KXMLMidiTemplate "MidiTemplate"
#define KXMLMidiTemplateName "Name"
#define KXMLMidiTemplateDescription "Description"
#define KXMLMidiTemplateInitMessage "InitMessage"


class MidiTemplate
{
    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Standard constructor */
    MidiTemplate();

    /** Copy constructor */
    MidiTemplate(const MidiTemplate& templ);

    /** Destructor */
    virtual ~MidiTemplate();

    /** Assignment operator */
    MidiTemplate& operator=(const MidiTemplate& templ);


    /********************************************************************
     * Template
     ********************************************************************/
public:
    void setName(const QString& name);
    QString name() const;

    void setInitMessage(const QByteArray& message);
    QByteArray initMessage() const;

protected:
    QString m_description;
    QByteArray m_initMessage;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    static MidiTemplate* loader(const QString& path);

    bool loadXML(const QDomDocument& doc);

};

#endif
