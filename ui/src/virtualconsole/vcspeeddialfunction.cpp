/*
  Q Light Controller
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

#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QDebug>

#include "vcspeeddialfunction.h"
#include "function.h"
#include "doc.h"

#define KXMLQLCSequenceSceneValues "Values"
#define KXMLQLCStepNote "Note"

VCSpeedDialFunction::VCSpeedDialFunction(quint32 aFid, SpeedMultiplier aFadeIn, SpeedMultiplier aFadeOut, SpeedMultiplier aDuration)
    : functionId(aFid)
    , fadeInMultiplier(aFadeIn)
    , fadeOutMultiplier(aFadeOut)
    , durationMultiplier(aDuration)
{
}

bool VCSpeedDialFunction::loadXML(const QDomElement& root, SpeedMultiplier aFadeIn, SpeedMultiplier aFadeOut, SpeedMultiplier aDuration)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.text().isEmpty() == true)
    {
        qWarning() << Q_FUNC_INFO << "Function ID not found";
        return false;
    }
    functionId = root.text().toUInt();

    // For each multiplier: If not present in XML, use default value.
    if (root.hasAttribute(KXMLQLCFunctionSpeedFadeIn) == true)
        fadeInMultiplier = static_cast<SpeedMultiplier>(root.attribute(KXMLQLCFunctionSpeedFadeIn).toUInt());
    else
        fadeInMultiplier = aFadeIn;
    if (root.hasAttribute(KXMLQLCFunctionSpeedFadeOut) == true)
        fadeOutMultiplier = static_cast<SpeedMultiplier>(root.attribute(KXMLQLCFunctionSpeedFadeOut).toUInt());
    else
        fadeOutMultiplier = aFadeOut;
    if (root.hasAttribute(KXMLQLCFunctionSpeedDuration) == true)
        durationMultiplier = static_cast<SpeedMultiplier>(root.attribute(KXMLQLCFunctionSpeedDuration).toUInt());
    else
        durationMultiplier = aDuration;

    return true;
}

bool VCSpeedDialFunction::saveXML(QDomDocument* doc, QDomElement* root) const
{
    QDomElement tag;
    QDomText text;

    /* Function tag */
    tag = doc->createElement(KXMLQLCFunction);
    root->appendChild(tag);

    /* Multipliers */
    tag.setAttribute(KXMLQLCFunctionSpeedFadeIn, fadeInMultiplier);
    tag.setAttribute(KXMLQLCFunctionSpeedFadeOut, fadeOutMultiplier);
    tag.setAttribute(KXMLQLCFunctionSpeedDuration, durationMultiplier);

    /* Function ID */
    text = doc->createTextNode(QString::number(functionId));
    tag.appendChild(text);

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
