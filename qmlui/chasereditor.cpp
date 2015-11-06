/*
  Q Light Controller Plus
  chasereditor.cpp

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

#include "chasereditor.h"
#include "chaserstep.h"
#include "chaser.h"

ChaserEditor::ChaserEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_chaser(NULL)
{
    m_view->rootContext()->setContextProperty("chaserEditor", this);
}

void ChaserEditor::setFunctionID(quint32 ID)
{
    m_chaser = qobject_cast<Chaser *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);
}

QVariant ChaserEditor::stepsList() const
{
    QVariantList stepList;

    if (m_chaser != NULL)
    {
        foreach(ChaserStep step, m_chaser->steps())
        {
            QVariantMap stepMap;
            stepMap.insert("funcID", step.fid);
            stepMap.insert("fadeIn", Function::speedToString(step.fadeIn));
            stepMap.insert("hold", Function::speedToString(step.hold));
            stepMap.insert("fadeOut", Function::speedToString(step.fadeOut));
            stepMap.insert("duration", Function::speedToString(step.duration));
            stepMap.insert("note", step.note);
            stepList.append(stepMap);
        }
    }

    return QVariant::fromValue(stepList);
}

QString ChaserEditor::chaserName() const
{
    if (m_chaser == NULL)
        return "";
    return m_chaser->name();
}

void ChaserEditor::setChaserName(QString chaserName)
{
    if (m_chaser == NULL || m_chaser->name() == chaserName)
        return;

    m_chaser->setName(chaserName);
    emit chaserNameChanged(chaserName);
}
