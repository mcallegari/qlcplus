/*
  Q Light Controller Plus
  qlcclipboard.cpp

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

#include "qlcclipboard.h"
#include "doc.h"

QLCClipboard::QLCClipboard(Doc *doc)
    : m_doc(doc)
    , m_copyFunction(NULL)
{

}

void QLCClipboard::resetContents()
{
    m_copySteps.clear();
    m_copySceneValues.clear();
    if (m_copyFunction != NULL && m_doc->function(m_copyFunction->id()) == NULL)
        delete m_copyFunction;
    m_copyFunction = NULL;
}

void QLCClipboard::copyContent(quint32 sourceID, QList<ChaserStep> steps)
{
    Q_UNUSED(sourceID)

    m_copySteps = steps;
}

void QLCClipboard::copyContent(quint32 sourceID, QList<SceneValue> values)
{
    Q_UNUSED(sourceID)

    m_copySceneValues = values;
}

void QLCClipboard::copyContent(quint32 sourceID, Function *function)
{
    Q_UNUSED(sourceID)

    if (function == NULL)
        return;

    if (m_copyFunction != NULL && m_doc->function(m_copyFunction->id()) == NULL)
        delete m_copyFunction;
    m_copyFunction = NULL;

    /* Attempt to create a copy of the function to Doc */
    Function* copy = function->createCopy(m_doc, false);
    if (copy != NULL)
    {
        copy->setName(tr("Copy of %1").arg(function->name()));
        m_copyFunction = copy;
    }
}

bool QLCClipboard::hasChaserSteps()
{
    if (m_copySteps.count() > 0)
        return true;

    return false;
}

bool QLCClipboard::hasSceneValues()
{
    if (m_copySceneValues.count() > 0)
        return true;

    return false;
}

bool QLCClipboard::hasFunction()
{
    if (m_copyFunction != NULL)
        return true;

    return false;
}

QList<ChaserStep> QLCClipboard::getChaserSteps()
{
    return m_copySteps;
}

QList<SceneValue> QLCClipboard::getSceneValues()
{
    return m_copySceneValues;
}

Function *QLCClipboard::getFunction()
{
    return m_copyFunction;
}
