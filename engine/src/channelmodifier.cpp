/*
  Q Light Controller Plus
  channelmodifier.cpp

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

#include "channelmodifier.h"
#include "qlcfile.h"

#include <QDomText>
#include <QTextStream>
#include <QDebug>

ChannelModifier::ChannelModifier()
{
    m_values.fill(0, 256);
    m_name = QString();
    m_type = UserTemplate;
}

void ChannelModifier::setName(QString name)
{
    m_name = name;
}

QString ChannelModifier::name() const
{
    return m_name;
}

void ChannelModifier::setType(ChannelModifier::Type type)
{
    m_type = type;
}

ChannelModifier::Type ChannelModifier::type() const
{
    return m_type;
}

void ChannelModifier::setModifierMap(QList<QPair<uchar, uchar> > map)
{
    m_map = map;
    m_values.fill(0, 256);
    QPair<uchar, uchar> lastDMXPair;
    for (int i = 0; i < m_map.count(); i++)
    {
        QPair<uchar, uchar> dmxPair = m_map.at(i);
        m_values[dmxPair.first] = dmxPair.second;
        if (i != 0)
        {
            // calculate the increment to go from one pair to another
            // in a linear progression
            float dmxInc = 0;
            if (dmxPair.first - lastDMXPair.first > 0)
                dmxInc = (float)(dmxPair.second - lastDMXPair.second) / (float)(dmxPair.first - lastDMXPair.first);

            // use a float variable here to be as more accurate as possible
            float floatVal = lastDMXPair.second;
            for (int p = lastDMXPair.first; p < dmxPair.first; p++)
            {
                // the float value is rounded here but it
                // is what we wanted
                m_values[p] = floatVal;
                floatVal += dmxInc;
            }
        }
        lastDMXPair = dmxPair;
    }
// Enable the following to display the template full range of value
/*
    qDebug() << "Template:" << m_name;
    for (int d = 0; d < m_values.count(); d++)
        qDebug() << "Pos:" << d << "val:" << QString::number((uchar)m_values.at(d));
*/
}

QList< QPair<uchar, uchar> > ChannelModifier::modifierMap() const
{
    return m_map;
}

uchar ChannelModifier::getValue(uchar dmxValue)
{
    return m_values.at(dmxValue);
}

QFile::FileError ChannelModifier::saveXML(const QString &fileName)
{
    QFile::FileError error;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) == false)
        return file.error();

    QDomDocument doc(QLCFile::getXMLHeader(KXMLQLCChannelModifierDocument));
    Q_ASSERT(doc.isNull() == false);

    /* Create a text stream for the file */
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    stream.setCodec("UTF-8");

    // create the document root node
    QDomElement root = doc.documentElement();

    QDomElement tag;
    QDomText text;

    /* Name */
    tag = doc.createElement(KXMLQLCChannelModName);
    root.appendChild(tag);
    text = doc.createTextNode(m_name);
    tag.appendChild(text);

    qDebug() << "Got map with" << m_map.count() << "handlers";
    for(int i = 0; i < m_map.count(); i++)
    {
        QPair<uchar, uchar> mapElement = m_map.at(i);
        tag = doc.createElement(KXMLQLCChannelModHandler);
        tag.setAttribute(KXMLQLCChannelModOriginalDMX, mapElement.first);
        tag.setAttribute(KXMLQLCChannelModModifiedDMX, mapElement.second);
        root.appendChild(tag);
    }

    /* Write the document into the stream */
    stream << doc.toString();
    error = QFile::NoError;
    file.close();

    return error;
}

QFile::FileError ChannelModifier::loadXML(const QString &fileName, Type type)
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

    QList< QPair<uchar, uchar> > modMap;

    if (doc.doctype().name() == KXMLQLCChannelModifierDocument)
    {
        QDomElement root = doc.documentElement();
        if (root.tagName() == KXMLQLCChannelModifierDocument)
        {
            QDomNode node = root.firstChild();
            while (node.isNull() == false)
            {
                QDomElement tag = node.toElement();
                if (tag.tagName() == KXMLQLCChannelModName)
                {
                    setName(tag.text());
                }
                else if(tag.tagName() == KXMLQLCChannelModHandler)
                {
                    QPair <uchar, uchar> dmxPair(0, 0);
                    if (tag.hasAttribute(KXMLQLCChannelModOriginalDMX))
                        dmxPair.first = tag.attribute(KXMLQLCChannelModOriginalDMX).toUInt();
                    if (tag.hasAttribute(KXMLQLCChannelModModifiedDMX))
                        dmxPair.second = tag.attribute(KXMLQLCChannelModModifiedDMX).toUInt();
                    modMap.append(dmxPair);
                }

                node = node.nextSibling();
            }
        }
    }
    if (modMap.count() > 0)
    {
        setType(type);
        setModifierMap(modMap);
    }

    return error;
}
