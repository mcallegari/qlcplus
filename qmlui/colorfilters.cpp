/*
  Q Light Controller Plus
  colorfilters.cpp

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

#include "colorfilters.h"
#include "qlcfile.h"

#define KXMLColorFiltersName  QString("Name")
#define KXMLColorFiltersColor QString("Color")
#define KXMLColorFiltersRGB   QString("RGB")
#define KXMLColorFiltersWAUV  QString("WAUV")

ColorFilters::ColorFilters(QObject *parent)
    : QObject(parent)
    , m_name(QString())
    , m_path(QString())
    , m_isUser(false)
{

}

/********************************************************************
 * Editing
 ********************************************************************/

QString ColorFilters::name() const
{
    return m_name;
}

void ColorFilters::setName(QString name)
{
    m_name = name;
}

QString ColorFilters::path() const
{
    return m_path;
}

bool ColorFilters::isUser() const
{
    return m_isUser;
}

void ColorFilters::setIsUser(bool user)
{
    m_isUser = user;
}

QVariantList ColorFilters::filtersList()
{
    QVariantList list;

    for (ColorInfo cInfo : m_filterList)
    {
        QVariantMap fMap;
        fMap.insert("name", cInfo.m_name);
        fMap.insert("rgb", cInfo.m_rgb);
        fMap.insert("wauv", cInfo.m_wauv);
        list.append(fMap);
    }

    return list;
}

void ColorFilters::addFilter(QString name, quint8 red, quint8 green, quint8 blue,
                             quint8 white, quint8 amber, quint8 uv)
{
    ColorInfo cInfo;

    if (name.isEmpty())
        return;

    cInfo.m_name = name;
    cInfo.m_rgb = QColor(red, green, blue);
    cInfo.m_wauv = QColor(white, amber, uv);

    m_filterList.append(cInfo);
    emit filtersListChanged();
}

void ColorFilters::changeFilterAt(int index, quint8 red, quint8 green, quint8 blue,
                                  quint8 white, quint8 amber, quint8 uv)
{
    if (index < 0 || index >= m_filterList.count())
        return;

    m_filterList[index].m_rgb = QColor(red, green, blue);
    m_filterList[index].m_wauv = QColor(white, amber, uv);
    emit filtersListChanged();
}

void ColorFilters::removeFilterAt(int index)
{
    if (index < 0 || index >= m_filterList.count())
        return;

    m_filterList.removeAt(index);
    emit filtersListChanged();
}

void ColorFilters::renameFilterAt(int index, QString newName)
{
    if (index < 0 || index >= m_filterList.count())
        return;

    m_filterList[index].m_name = newName;
    emit filtersListChanged();
}

/********************************************************************
 * Load & Save
 ********************************************************************/
void ColorFilters::save()
{
    saveXML(m_path);
}

QFileDevice::FileError ColorFilters::saveXML(const QString &fileName)
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
    QLCFile::writeXMLHeader(&doc, KXMLColorFilters);

    doc.writeTextElement(KXMLColorFiltersName, m_name);

    for (ColorInfo cInfo : m_filterList)
    {
        doc.writeStartElement(KXMLColorFiltersColor);
        doc.writeAttribute(KXMLColorFiltersName, cInfo.m_name);
        if (cInfo.m_rgb.isValid())
            doc.writeAttribute(KXMLColorFiltersRGB, cInfo.m_rgb.name());
        if (cInfo.m_wauv.isValid())
            doc.writeAttribute(KXMLColorFiltersWAUV, cInfo.m_wauv.name());

        doc.writeEndElement();
    }

    m_path = fileName;
    /* End the document and close all the open elements */
    error = QFile::NoError;
    doc.writeEndDocument();
    file.close();

    return error;

}

QFileDevice::FileError ColorFilters::loadXML(const QString &fileName)
{
    QFile::FileError error = QFile::NoError;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QXmlStreamReader *doc = QLCFile::getXMLReader(fileName);
    if (doc == nullptr || doc->device() == nullptr || doc->hasError())
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

    if (doc->dtdName() == KXMLColorFilters)
    {
        if (doc->readNextStartElement() == false)
            return QFile::ResourceError;

        if (doc->name() == KXMLColorFilters)
        {
            int count = 0;
            while (doc->readNextStartElement())
            {
                if (doc->name() == KXMLColorFiltersColor)
                {
                    ColorInfo cInfo;
                    QXmlStreamAttributes attrs = doc->attributes();
                    if (attrs.hasAttribute(KXMLColorFiltersName))
                        cInfo.m_name = attrs.value(KXMLColorFiltersName).toString();
                    if (attrs.hasAttribute(KXMLColorFiltersRGB))
                        cInfo.m_rgb = QColor(attrs.value(KXMLColorFiltersRGB).toString());
                    if (attrs.hasAttribute(KXMLColorFiltersWAUV))
                        cInfo.m_wauv = QColor(attrs.value(KXMLColorFiltersWAUV).toString());

                    if (cInfo.m_name.isEmpty() == false && (cInfo.m_rgb.isValid() || cInfo.m_wauv.isValid()))
                        m_filterList.append(cInfo);

                    doc->skipCurrentElement();
                    count++;
                }
                else if (doc->name() == KXMLColorFiltersName)
                {
                    m_name = doc->readElementText();
                }
                else if (doc->name() == KXMLQLCCreator)
                {
                    /* Ignore this block */
                    doc->skipCurrentElement();
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown ColorFilters tag:" << doc->name();
                    doc->skipCurrentElement();
                }
            }
            qDebug() << count << "filters loaded";
        }
    }

    m_path = fileName;

    QLCFile::releaseXMLReader(doc);

    return error;
}

