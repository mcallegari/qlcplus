/*
  Q Light Controller Plus
  rgbalgorithm.cpp

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
#include <QStringList>
#include <QDebug>

#include "rgbscriptscache.h"
#include "rgbalgorithm.h"
#include "rgbaudio.h"
#include "rgbimage.h"
#include "rgbplain.h"
#include "rgbtext.h"
#include "doc.h"

#ifdef QT_QML_LIB
  #include "rgbscriptv4.h"
#else
  #include "rgbscript.h"
#endif

RGBAlgorithm::RGBAlgorithm(Doc * doc)
    : m_doc(doc)
{
}

void RGBAlgorithm::setColors(QVector<QColor> colors)
{
    int nCols = acceptColors();
    m_colors.clear();

    for (int i = 0; i < nCols; i++)
    {
        if (i < colors.count())
            m_colors.append(colors.at(i));
    }
}

QColor RGBAlgorithm::getColor(uint i) const
{
    if (i >= (uint)m_colors.count())
        return QColor();

    return m_colors[i];
}

/****************************************************************************
 * Available algorithms
 ****************************************************************************/

QStringList RGBAlgorithm::algorithms(Doc * doc)
{
    QStringList list;
    RGBPlain plain(doc);
    RGBText text(doc);
    RGBImage image(doc);
    RGBAudio audio(doc);
    list << plain.name();
    list << text.name();
    list << image.name();
    list << audio.name();
    list << doc->rgbScriptsCache()->names();
    return list;
}

RGBAlgorithm* RGBAlgorithm::algorithm(Doc * doc, const QString& name)
{
    RGBText text(doc);
    RGBImage image(doc);
    RGBAudio audio(doc);
    RGBPlain plain(doc);
    if (name == text.name())
        return text.clone();
    else if (name == image.name())
        return image.clone();
    else if (name == audio.name())
        return audio.clone();
    else if (name == plain.name())
        return plain.clone();
    else
        return doc->rgbScriptsCache()->script(name).clone();
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

RGBAlgorithm* RGBAlgorithm::loader(Doc * doc, QXmlStreamReader &root)
{
    RGBAlgorithm* algo = NULL;

    if (root.name() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return NULL;
    }

    QString type = root.attributes().value(KXMLQLCRGBAlgorithmType).toString();
    if (type == KXMLQLCRGBImage)
    {
        RGBImage image(doc);
        if (image.loadXML(root) == true)
            algo = image.clone();
    }
    else if (type == KXMLQLCRGBText)
    {
        RGBText text(doc);
        if (text.loadXML(root) == true)
            algo = text.clone();
    }
    else if (type == KXMLQLCRGBAudio)
    {
        RGBAudio audio(doc);
        if (audio.loadXML(root) == true)
            algo = audio.clone();
    }
    else if (type == KXMLQLCRGBScript)
    {
        RGBScript const& scr = doc->rgbScriptsCache()->script(root.readElementText());
        if (scr.apiVersion() > 0 && scr.name().isEmpty() == false)
            algo = scr.clone();
    }
    else if (type == KXMLQLCRGBPlain)
    {
        RGBPlain plain(doc);
        if (plain.loadXML(root) == true)
            algo = plain.clone();
    }
    else
    {
        qWarning() << "Unrecognized RGB algorithm type:" << type;
    }

    return algo;
}
