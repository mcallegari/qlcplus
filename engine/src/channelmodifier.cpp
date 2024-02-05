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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "channelmodifier.h"
#include "qlcfile.h"

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

    QXmlStreamWriter doc(&file);
    doc.setAutoFormatting(true);
    doc.setAutoFormattingIndent(1);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    doc.setCodec("UTF-8");
#endif
    QLCFile::writeXMLHeader(&doc, KXMLQLCChannelModifierDocument);

    doc.writeTextElement(KXMLQLCChannelModName, m_name);

    qDebug() << "Got map with" << m_map.count() << "handlers";
    for (int i = 0; i < m_map.count(); i++)
    {
        QPair<uchar, uchar> mapElement = m_map.at(i);
        doc.writeStartElement(KXMLQLCChannelModHandler);
        doc.writeAttribute(KXMLQLCChannelModOriginalDMX, QString::number(mapElement.first));
        doc.writeAttribute(KXMLQLCChannelModModifiedDMX, QString::number(mapElement.second));
        doc.writeEndElement();
    }

    /* End the document and close all the open elements */
    error = QFile::NoError;
    doc.writeEndDocument();
    file.close();

    return error;
}

QFile::FileError ChannelModifier::loadXML(const QString &fileName, Type type)
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

    QList< QPair<uchar, uchar> > modMap;

    if (doc->dtdName() == KXMLQLCChannelModifierDocument)
    {
        if (doc->readNextStartElement() == false)
            return QFile::ResourceError;

        if (doc->name() == KXMLQLCChannelModifierDocument)
        {
            while (doc->readNextStartElement())
            {
                if (doc->name() == KXMLQLCChannelModName)
                {
                    setName(doc->readElementText());
                }
                else if (doc->name() == KXMLQLCChannelModHandler)
                {
                    QPair <uchar, uchar> dmxPair(0, 0);
                    QXmlStreamAttributes attrs = doc->attributes();
                    if (attrs.hasAttribute(KXMLQLCChannelModOriginalDMX))
                        dmxPair.first = attrs.value(KXMLQLCChannelModOriginalDMX).toString().toUInt();
                    if (attrs.hasAttribute(KXMLQLCChannelModModifiedDMX))
                        dmxPair.second = attrs.value(KXMLQLCChannelModModifiedDMX).toString().toUInt();
                    modMap.append(dmxPair);
                    doc->skipCurrentElement();
                }
                else if (doc->name() == KXMLQLCCreator)
                {
                    /* Ignore creator information */
                    doc->skipCurrentElement();
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown ChannelModifier tag:" << doc->name();
                    doc->skipCurrentElement();
                }
            }
        }
    }
    if (modMap.count() > 0)
    {
        setType(type);
        setModifierMap(modMap);
    }

    QLCFile::releaseXMLReader(doc);

    return error;
}
