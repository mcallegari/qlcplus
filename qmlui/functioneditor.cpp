/*
  Q Light Controller Plus
  functioneditor.cpp

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

#include "functioneditor.h"
#include "doc.h"

FunctionEditor::FunctionEditor(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    m_functionID = Function::invalidId();
    m_functionType = Function::Undefined;
    m_preview = false;
}

FunctionEditor::~FunctionEditor()
{

}

void FunctionEditor::setFunctionID(quint32 ID)
{
    m_functionID = ID;
    Function *f = m_doc->function(ID);
    if (f != NULL)
        m_functionType = f->type();
}

quint32 FunctionEditor::functionID() const
{
    return m_functionID;
}

Function::Type FunctionEditor::functionType() const
{
    return m_functionType;
}

void FunctionEditor::setPreview(bool enable)
{
    m_preview = enable;

    Function *f = m_doc->function(m_functionID);
    if (f == NULL)
        return;

    if (m_preview)
    {
        if (f->isRunning() == false)
            f->start(m_doc->masterTimer(), FunctionParent::master());
    }
    else
    {
        if (f->isRunning())
            f->stop(FunctionParent::master());
    }
}

