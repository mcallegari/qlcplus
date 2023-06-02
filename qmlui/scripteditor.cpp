/*
  Q Light Controller Plus
  scripteditor.cpp

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

#include "scripteditor.h"
#include "scriptwrapper.h"
#include "doc.h"

ScriptEditor::ScriptEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_script(nullptr)
{
    m_view->rootContext()->setContextProperty("scriptEditor", this);
}

void ScriptEditor::setFunctionID(quint32 ID)
{
    m_script = qobject_cast<Script *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);
}

QString ScriptEditor::scriptContent() const
{
    return m_script != nullptr ? m_script->data() : QString();
}

void ScriptEditor::setScriptContent(QString scriptContent)
{
    if (m_script == nullptr)
        return;

    m_script->setData(scriptContent);
    emit scriptContentChanged(scriptContent);
}

QString ScriptEditor::syntaxErrors()
{
    if (m_script == nullptr)
        return QString();

    QStringList errList = m_script->syntaxErrorsLines();
    if (errList.isEmpty())
        return tr("No errors found.");

    return errList.join("\n");
}
