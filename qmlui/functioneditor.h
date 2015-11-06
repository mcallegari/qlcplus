/*
  Q Light Controller Plus
  functioneditor.h

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

#ifndef FUNCTIONEDITOR_H
#define FUNCTIONEDITOR_H

#include <QQmlContext>
#include <QQuickView>
#include <QQuickItem>
#include <QObject>

#include "function.h"

class Doc;

class FunctionEditor : public QObject
{
    Q_OBJECT

public:
    FunctionEditor(QQuickView *view, Doc *doc, QObject *parent = 0);
    virtual ~FunctionEditor();

    /** Set the ID of the Function being edit */
    virtual void setFunctionID(quint32 ID);
    /** Return the ID of the Function being edited */
    virtual quint32 functionID() const;
    /** Return the type of the Function being edited */
    virtual Function::Type functionType() const;

    /** Enable/Disable the preview of the edited Function */
    virtual void setPreview(bool enable);

protected:
    /** Reference of the QML view */
    QQuickView *m_view;
    /** Reference of the project workspace */
    Doc *m_doc;
    /** ID of the Function being edited */
    quint32 m_functionID;
    /** Type of the Function being edited */
    Function::Type m_functionType;
    /** Flag that holds if the editor should preview its function */
    bool m_preview;
};

#endif // SCENEEDITOR_H
