/*
  Q Light Controller Plus
  sequence.cpp

  Copyright (C) Massimo Callegari

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

#include "sequence.h"

#define KXMLQLCSequenceBoundScene "BoundScene"

Sequence::Sequence(Doc* doc)
    : Chaser(doc)
    , m_boundSceneID(Function::invalidId())
    , m_needFixup(true)
{
    m_type = Function::SequenceType;
    setName(tr("New Sequence"));
}

Sequence::~Sequence()
{
}

QIcon Sequence::getIcon() const
{
    return QIcon(":/sequence.png");
}

Function *Sequence::createCopy(Doc *doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Sequence(doc);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Sequence::copyFrom(const Function *function)
{
    const Sequence* sequence = qobject_cast<const Sequence*> (function);
    if (sequence == NULL)
        return false;

    // Copy sequence stuff
    m_steps = sequence->m_steps;
    m_fadeInMode = sequence->m_fadeInMode;
    m_fadeOutMode = sequence->m_fadeOutMode;
    m_holdMode = sequence->m_holdMode;
    m_boundSceneID = sequence->m_boundSceneID;

    // Copy common function stuff
    return Function::copyFrom(function);
}

void Sequence::setBoundSceneID(quint32 sceneID)
{
    m_boundSceneID = sceneID;
}

quint32 Sequence::boundSceneID() const
{
    return m_boundSceneID;
}

QList<quint32> Sequence::components()
{
    QList<quint32> ids;
    if (m_boundSceneID != Function::invalidId())
        ids.append(m_boundSceneID);
    return ids;
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Sequence::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    doc->writeAttribute(KXMLQLCSequenceBoundScene, QString::number(boundSceneID()));

    /* Speed */
    saveXMLSpeed(doc);

    /* Direction */
    saveXMLDirection(doc);

    /* Run order */
    saveXMLRunOrder(doc);

    /* Speed modes */
    doc->writeStartElement(KXMLQLCChaserSpeedModes);
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeIn, speedModeToString(fadeInMode()));
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeOut, speedModeToString(fadeOutMode()));
    doc->writeAttribute(KXMLQLCFunctionSpeedDuration, speedModeToString(durationMode()));
    doc->writeEndElement();

    /* Steps */
    for (int i = 0; i < m_steps.count(); i++)
        m_steps.at(i).saveXML(doc, i, true);

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

bool Sequence::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    QXmlStreamAttributes funcAttrs = root.attributes();

    if (funcAttrs.value(KXMLQLCFunctionType).toString() != typeToString(Function::SequenceType))
    {
        qWarning() << Q_FUNC_INFO << funcAttrs.value(KXMLQLCFunctionType).toString()
                   << "is not a Sequence";
        return false;
    }

    if (funcAttrs.hasAttribute(KXMLQLCSequenceBoundScene) == false)
    {
        qWarning() << Q_FUNC_INFO << "Sequence doesn't have a bound Scene ID";
        return false;
    }

    setBoundSceneID(funcAttrs.value(KXMLQLCSequenceBoundScene).toString().toUInt());

    Scene *scene = qobject_cast<Scene *>(doc()->function(boundSceneID()));
    QList<SceneValue> sceneValues;
    if (scene != NULL)
    {
        sceneValues = scene->values();
        std::sort(sceneValues.begin(), sceneValues.end());
        m_needFixup = false;
    }

    /* Load Sequence contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(root);
        }
        else if (root.name() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(root);
        }
        else if (root.name() == KXMLQLCChaserSpeedModes)
        {
            loadXMLSpeedModes(root);
        }
        else if (root.name() == KXMLQLCFunctionStep)
        {
            //! @todo stepNumber is useless if the steps are in the wrong order
            ChaserStep step;
            int stepNumber = -1;

            if (sceneValues.isEmpty() == false)
                step.values = sceneValues;

            if (step.loadXML(root, stepNumber, doc()) == true)
            {
                step.fid = boundSceneID();

                if (stepNumber >= m_steps.size())
                    m_steps.append(step);
                else
                    m_steps.insert(stepNumber, step);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Sequence tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

void Sequence::postLoad()
{
    if (m_needFixup == false)
        return;

    Doc* doc = this->doc();
    Q_ASSERT(doc != NULL);

    Scene *scene = qobject_cast<Scene *>(doc->function(boundSceneID()));
    QList<SceneValue> sceneValues;
    if (scene != NULL)
    {
        sceneValues = scene->values();

        if (sceneValues.count() == 0)
        {
            qDebug() << "The bound Scene is empty ! This should never happen. Trying to fix it...";
            if (stepsCount())
            {
                foreach (SceneValue value, m_steps.at(0).values)
                {
                    value.value = 0;
                    if (doc->fixture(value.fxi) != NULL)
                        scene->setValue(value);
                }
            }
            m_needFixup = false;
            return;
        }

        std::sort(sceneValues.begin(), sceneValues.end());
    }

    int stepIndex = 0;

    QMutableListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        if (sceneValues.count() == step.values.count())
        {
            stepIndex++;
            continue;
        }

        QList <SceneValue> tmpList = step.values;
        step.values = sceneValues;
        for (int i = 0; i < tmpList.count(); i++)
        {
            int tmpIndex = step.values.indexOf(tmpList.at(i));
            if (tmpIndex == -1)
                continue;
            step.values.replace(tmpIndex, tmpList.at(i));
        }

        replaceStep(step, stepIndex);
        stepIndex++;
    }
    m_needFixup = false;

    qDebug() << "Sequence" << name() << "steps fixed. Values:" << sceneValues.count();
}
