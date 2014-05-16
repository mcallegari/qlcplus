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

VCSpeedDialFunction::VCSpeedDialFunction(quint32 aFid, SpeedMultiplier aFadeIn, SpeedMultiplier aDuration, SpeedMultiplier aFadeOut)
    : functionId(aFid)
    , fadeInMultiplier(aFadeIn)
    , fadeOutMultiplier(aFadeOut)
    , durationMultiplier(aDuration)
{
}

bool VCSpeedDialFunction::loadXML(const QDomElement& root)
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

    if (root.hasAttribute(KXMLQLCFunctionSpeedFadeIn) == true)
        fadeInMultiplier = static_cast<SpeedMultiplier>(root.attribute(KXMLQLCFunctionSpeedFadeIn).toUInt());
    if (root.hasAttribute(KXMLQLCFunctionSpeedFadeOut) == true)
        fadeOutMultiplier = static_cast<SpeedMultiplier>(root.attribute(KXMLQLCFunctionSpeedFadeOut).toUInt());
    if (root.hasAttribute(KXMLQLCFunctionSpeedDuration) == true)
        durationMultiplier = static_cast<SpeedMultiplier>(root.attribute(KXMLQLCFunctionSpeedDuration).toUInt());

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

