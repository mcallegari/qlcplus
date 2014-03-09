/*
  Q Light Controller
  rgbalgorithm.cpp

  Copyright (c) Heikki Junnila

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

#include <QDomDocument>
#include <QDomElement>
#include <QStringList>
#include <QDebug>

#include "rgbalgorithm.h"
#include "rgbaudio.h"
#include "rgbimage.h"
#include "rgbscript.h"
#include "rgbtext.h"

RGBAlgorithm::RGBAlgorithm(const Doc * doc)
    : m_doc(doc)
    , m_startColor(QColor())
    , m_endColor(QColor())
{
}

void RGBAlgorithm::setColors(QColor start, QColor end)
{
    m_startColor = start;
    m_endColor = end;
}

/****************************************************************************
 * Available algorithms
 ****************************************************************************/

QStringList RGBAlgorithm::algorithms(const Doc * doc)
{
    QStringList list;
    RGBText text(doc);
    RGBImage image(doc);
    RGBAudio audio(doc);
    list << text.name();
    list << image.name();
    list << audio.name();
    list << RGBScript::scriptNames(doc);
    return list;
}

RGBAlgorithm* RGBAlgorithm::algorithm(const Doc * doc, const QString& name)
{
    RGBText text(doc);
    RGBImage image(doc);
    RGBAudio audio(doc);
    if (name == text.name())
        return text.clone();
    else if (name == image.name())
        return image.clone();
    else if (name == audio.name())
        return audio.clone();
    else
        return RGBScript::script(doc, name).clone();
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

RGBAlgorithm* RGBAlgorithm::loader(const Doc * doc, const QDomElement& root)
{
    RGBAlgorithm* algo = NULL;

    if (root.tagName() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return NULL;
    }

    QString type = root.attribute(KXMLQLCRGBAlgorithmType);
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
        RGBScript scr = RGBScript::script(doc, root.text());
        if (scr.apiVersion() > 0 && scr.name().isEmpty() == false)
            algo = scr.clone();
    }
    else
    {
        qWarning() << "Unrecognized RGB algorithm type:" << type;
    }

    return algo;
}
