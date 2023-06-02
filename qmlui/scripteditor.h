/*
  Q Light Controller Plus
  scripteditor.h

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

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "functioneditor.h"

class Script;

class ScriptEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QString scriptContent READ scriptContent WRITE setScriptContent NOTIFY scriptContentChanged)

public:
    ScriptEditor(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Set the ID of the Audio being edited */
    void setFunctionID(quint32 ID);

    /** Get/Set the content of the Script currently being edited */
    QString scriptContent() const;
    void setScriptContent(QString scriptContent);

    Q_INVOKABLE QString syntaxErrors();

signals:
    void scriptContentChanged(QString scriptContent);

private:
    /** Reference of the Script currently being edited */
    Script *m_script;
};

#endif // SCRIPTEDITOR_H
