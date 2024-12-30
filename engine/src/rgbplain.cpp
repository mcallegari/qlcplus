/*
  Q Light Controller Plus
  rgbplain.cpp

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

#include "rgbplain.h"
#include "doc.h"

RGBPlain::RGBPlain(Doc * doc)
    : RGBAlgorithm(doc)
{
}

RGBPlain::RGBPlain(const RGBPlain& a, QObject *parent)
    : QObject(parent)
    , RGBAlgorithm(a.doc())
{
}

RGBPlain::~RGBPlain()
{
}

RGBAlgorithm* RGBPlain::clone() const
{
    RGBPlain* plain = new RGBPlain(*this);
    return static_cast<RGBAlgorithm*> (plain);
}

/****************************************************************************
 * RGBAlgorithm
 ****************************************************************************/

int RGBPlain::rgbMapStepCount(const QSize& size)
{
    Q_UNUSED(size);
    return 1;
}

void RGBPlain::rgbMapSetColors(QVector<uint> &colors)
{
    Q_UNUSED(colors);
}

QVector<uint> RGBPlain::rgbMapGetColors()
{
    return QVector<uint>();
}

void RGBPlain::rgbMap(const QSize& size, uint rgb, int step, RGBMap &map)
{
    Q_UNUSED(step);
    map.resize(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        map[y].fill(rgb);
    }
}

QString RGBPlain::name() const
{
    return QString("Plain Color");
}

QString RGBPlain::author() const
{
    return QString("Massimo Callegari");
}

int RGBPlain::apiVersion() const
{
    return 1;
}

void RGBPlain::setColors(QVector<QColor> colors)
{
    RGBAlgorithm::setColors(colors);
}

RGBAlgorithm::Type RGBPlain::type() const
{
    return RGBAlgorithm::Plain;
}

int RGBPlain::acceptColors() const
{
    return 1; // only start color is accepted
}

bool RGBPlain::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCRGBAlgorithmType).toString() != KXMLQLCRGBPlain)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm is not Plain";
        return false;
    }

    root.skipCurrentElement();

    return true;
}

bool RGBPlain::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCRGBAlgorithm);
    doc->writeAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBPlain);
    doc->writeEndElement();

    return true;
}
