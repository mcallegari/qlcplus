/*
  Q Light Controller Plus
  miditemplate.h

  Copyright (c) Joep Admiraal

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

#ifndef MIDITEMPLATE_H
#define MIDITEMPLATE_H

#include <QStringList>
#include <QString>
#include <QHash>
#include <QMap>

class MidiTemplate;
class QXmlStreamReader;

#define KXMLMidiTemplate            QString("MidiTemplate")
#define KXMLMidiTemplateName        QString("Name")
#define KXMLMidiTemplateDescription QString("Description")
#define KXMLMidiTemplateInitMessage QString("InitMessage")


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

    bool loadXML(QXmlStreamReader &doc);

};

#endif
