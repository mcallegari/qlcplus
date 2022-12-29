/*
  Q Light Controller Plus
  channelmodifier.h

  Copyright (c) Massimo Callegari

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

#ifndef CHANNELMODIFIER_H
#define CHANNELMODIFIER_H

#include <QByteArray>
#include <QPair>
#include <QFile>

/** @addtogroup engine Engine
 * @{
 */

// Channel modifier document type
#define KXMLQLCChannelModifierDocument QString("ChannelModifier")

// Channel modifier tags and attributes
#define KXMLQLCChannelModName           QString("Name")
#define KXMLQLCChannelModHandler        QString("Handler")
#define KXMLQLCChannelModOriginalDMX    QString("Original")
#define KXMLQLCChannelModModifiedDMX    QString("Modified")

class ChannelModifier
{
public:
    ChannelModifier();

    enum Type {
        SystemTemplate = 0,
        UserTemplate = 1
    };

    void setName(QString name);

    QString name() const;

    void setType(Type type);

    Type type() const;

    void setModifierMap(QList< QPair<uchar, uchar> > map);

    QList< QPair<uchar, uchar> > modifierMap() const;

    uchar getValue(uchar dmxValue);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Save the modifier into an XML file */
    QFile::FileError saveXML(const QString& fileName);

    /** Load this modifier's content from the given file */
    QFile::FileError loadXML(const QString& fileName, Type type);

private:
    QString m_name;
    Type m_type;
    QList< QPair<uchar, uchar> > m_map;
    QByteArray m_values;
};

/** @} */

#endif // CHANNELMODIFIER_H
