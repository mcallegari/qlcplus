/*
  Q Light Controller
  chaserstep.cpp

  Copyright (C) 2004 Heikki Junnila

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

#include "chaserstep.h"
#include "function.h"
#include "doc.h"

#define KXMLQLCSequenceSceneValues "Values"
#define KXMLQLCStepNote "Note"

ChaserStep::ChaserStep(quint32 aFid, uint aFadeIn, uint aHold, uint aFadeOut)
    : fid(aFid)
    , fadeIn(aFadeIn)
    , hold(aHold)
    , fadeOut(aFadeOut)
{
    duration = fadeIn + hold;
    note = QString();
}

ChaserStep::ChaserStep(const ChaserStep& cs)
    : fid(cs.fid)
    , fadeIn(cs.fadeIn)
    , hold(cs.hold)
    , fadeOut(cs.fadeOut)
    , duration(cs.duration)
    , values(cs.values)
    , note(cs.note)
{
}

bool ChaserStep::operator==(const ChaserStep& cs) const
{
    return (fid == cs.fid) ? true : false;
}

Function* ChaserStep::resolveFunction(const Doc* doc) const
{
    if (doc == NULL)
        return NULL;
    else
        return doc->function(fid);
}

#if 1
QVariant ChaserStep::toVariant() const
{
    qDebug() << "-------------  ChaserStep::toVariant";
    QList <QVariant> list;
    list << fid;
    list << fadeIn;
    list << hold;
    list << fadeOut;
    list << duration;
    list << note;
    return list;
}

ChaserStep ChaserStep::fromVariant(const QVariant& var)
{
    ChaserStep cs;
    qDebug() << "-------------  ChaserStep::fromVariant";
    QList <QVariant> list(var.toList());
    if (list.size() == 6)
    {
        cs.fid = list.takeFirst().toUInt();
        cs.fadeIn = list.takeFirst().toUInt();
        cs.hold = list.takeFirst().toUInt();
        cs.fadeOut = list.takeFirst().toUInt();
        cs.duration = list.takeFirst().toUInt();
        cs.note = list.takeFirst().toString();
    }
    return cs;
}
#endif

bool ChaserStep::loadXML(const QDomElement& root, int& stepNumber)
{
    bool holdFound = false;
    if (root.tagName() != KXMLQLCFunctionStep)
    {
        qWarning() << Q_FUNC_INFO << "ChaserStep node not found";
        return false;
    }

    if (root.hasAttribute(KXMLQLCFunctionSpeedFadeIn) == true)
        fadeIn = root.attribute(KXMLQLCFunctionSpeedFadeIn).toUInt();
    if (root.hasAttribute(KXMLQLCFunctionSpeedHold) == true)
    {
        hold = root.attribute(KXMLQLCFunctionSpeedHold).toUInt();
        holdFound = true;
    }
    if (root.hasAttribute(KXMLQLCFunctionSpeedFadeOut) == true)
        fadeOut = root.attribute(KXMLQLCFunctionSpeedFadeOut).toUInt();
    if (root.hasAttribute(KXMLQLCFunctionSpeedDuration) == true)
        duration = root.attribute(KXMLQLCFunctionSpeedDuration).toUInt();
    if (root.hasAttribute(KXMLQLCFunctionNumber) == true)
        stepNumber = root.attribute(KXMLQLCFunctionNumber).toInt();
    if (root.hasAttribute(KXMLQLCStepNote) == true)
        note = root.attribute(KXMLQLCStepNote);

    if (root.hasAttribute(KXMLQLCSequenceSceneValues) == true)
    {
        QString stepValues = root.text();
        if (stepValues.isEmpty() == false)
        {
            QStringList varray = stepValues.split(",");
            for (int i = 0; i < varray.count(); i+=3)
            {
                values.append(SceneValue(QString(varray.at(i)).toUInt(),
                                         QString(varray.at(i + 1)).toUInt(),
                                         uchar(QString(varray.at(i + 2)).toInt())));
            }
        }
    }
    else
    {
        if (root.text().isEmpty() == false)
            fid = root.text().toUInt();
    }

    if (holdFound == true)
    {
        if ((int)hold < 0)
            duration = hold;
        else
            duration = fadeIn + hold;
    }
    else
    {
        if ((int)duration < 0)
            hold = duration;
        else
            hold = duration - fadeIn;
    }

    return true;
}

bool ChaserStep::saveXML(QDomDocument* doc, QDomElement* root, int stepNumber, bool isSequence) const
{
    QDomElement tag;
    QDomText text;

    /* Step tag */
    tag = doc->createElement(KXMLQLCFunctionStep);
    root->appendChild(tag);

    /* Step number */
    tag.setAttribute(KXMLQLCFunctionNumber, stepNumber);

    /* Speeds */
    tag.setAttribute(KXMLQLCFunctionSpeedFadeIn, fadeIn);
    tag.setAttribute(KXMLQLCFunctionSpeedHold, hold);
    tag.setAttribute(KXMLQLCFunctionSpeedFadeOut, fadeOut);
    //tag.setAttribute(KXMLQLCFunctionSpeedDuration, duration); // deprecated from version 4.3.1
    if (note.isEmpty() == false)
        tag.setAttribute(KXMLQLCStepNote, note);

    if (isSequence)
    {
        /* it's a sequence step. Save values accordingly */
        tag.setAttribute(KXMLQLCSequenceSceneValues, values.count());
        QString stepValues;
        foreach(SceneValue scv, values)
        {
            if (scv.value != 0)
            {
                if (stepValues.isEmpty() == false)
                    stepValues.append(QString(","));
                stepValues.append(QString("%1,%2,%3").arg(scv.fxi).arg(scv.channel).arg(scv.value));
            }
        }
        if (stepValues.isEmpty() == false)
        {
            text = doc->createTextNode(stepValues);
            tag.appendChild(text);
        }
    }
    else
    {
        /* Step function ID */
        text = doc->createTextNode(QString::number(fid));
        tag.appendChild(text);
    }

    return true;
}
