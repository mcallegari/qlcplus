/*
  Q Light Controller Plus
  vcspeeddialfunction.cpp

  Copyright (C) 2014 David Garyga

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

#include "vcspeeddialfunction.h"
#include "function.h"

VCSpeedDialFunction::VCSpeedDialFunction(quint32 aFid, SpeedMultiplier aFadeIn,
                                         SpeedMultiplier aFadeOut, SpeedMultiplier aDuration)
    : functionId(aFid)
    , fadeInMultiplier(aFadeIn)
    , fadeOutMultiplier(aFadeOut)
    , durationMultiplier(aDuration)
{
}

bool VCSpeedDialFunction::loadXML(QXmlStreamReader &root, SpeedMultiplier aFadeIn,
                                  SpeedMultiplier aFadeOut, SpeedMultiplier aDuration)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();
    QString text = root.readElementText();
    if (text.isEmpty())
    {
        qWarning() << Q_FUNC_INFO << "Function ID not found";
        return false;
    }
    functionId = text.toUInt();

    // For each multiplier: If not present in XML, use default value.
    if (attrs.hasAttribute(KXMLQLCFunctionSpeedFadeIn) == true)
        fadeInMultiplier = static_cast<SpeedMultiplier>(attrs.value(KXMLQLCFunctionSpeedFadeIn).toString().toUInt());
    else
        fadeInMultiplier = aFadeIn;
    if (attrs.hasAttribute(KXMLQLCFunctionSpeedFadeOut) == true)
        fadeOutMultiplier = static_cast<SpeedMultiplier>(attrs.value(KXMLQLCFunctionSpeedFadeOut).toString().toUInt());
    else
        fadeOutMultiplier = aFadeOut;
    if (attrs.hasAttribute(KXMLQLCFunctionSpeedDuration) == true)
        durationMultiplier = static_cast<SpeedMultiplier>(attrs.value(KXMLQLCFunctionSpeedDuration).toString().toUInt());
    else
        durationMultiplier = aDuration;

    return true;
}

bool VCSpeedDialFunction::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Multipliers */
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeIn, QString::number(fadeInMultiplier));
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeOut, QString::number(fadeOutMultiplier));
    doc->writeAttribute(KXMLQLCFunctionSpeedDuration, QString::number(durationMultiplier));

    /* Function ID */
    doc->writeCharacters(QString::number(functionId));

    /* Close the <Function> tag */
    doc->writeEndElement();

    return true;
}

const QStringList &VCSpeedDialFunction::speedMultiplierNames()
{
    static QStringList *names = 0;

    if (names == 0)
    {
        names = new QStringList;
        *names << "(Not Sent)";
        *names << "0";
        *names << "1/16";
        *names << "1/8";
        *names << "1/4";
        *names << "1/2";
        *names << "1";
        *names << "2";
        *names << "4";
        *names << "8";
        *names << "16";
    }

    return *names;
}

const QVector <quint32> &VCSpeedDialFunction::speedMultiplierValuesTimes1000()
{
    static QVector <quint32> *values = 0;

    if (values == 0)
    {
        values = new QVector <quint32>;
        *values << 0; // None
        *values << 0; // Zero
        *values << 1000 / 16;
        *values << 1000 / 8;
        *values << 1000 / 4;
        *values << 1000 / 2;
        *values << 1000;
        *values << 1000 * 2;
        *values << 1000 * 4;
        *values << 1000 * 8;
        *values << 1000 * 16;
    }

    return *values;
}
