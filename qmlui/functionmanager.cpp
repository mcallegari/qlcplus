/*
  Q Light Controller Plus
  functionmanager.cpp

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

#include <QQmlContext>
#include <QDebug>

#include "functionmanager.h"
#include "treemodel.h"
#include "doc.h"

FunctionManager::FunctionManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    m_functionTree = new TreeModel(this);
    QStringList treeColumns;
    treeColumns << "funcID" << "funcType";
    m_functionTree->setColumnNames(treeColumns);
/*
    for (int i = 0; i < 10; i++)
    {
        QStringList vars;
        vars << QString::number(i) << 0;
        m_functionTree->addItem(QString("Entry %1").arg(i), vars);
    }
*/

    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotDocLoaded()));
}

QVariant FunctionManager::functionsList()
{
    return QVariant::fromValue(m_functionTree);
}

void FunctionManager::slotDocLoaded()
{
    m_functionTree->clear();
    foreach(Function *func, m_doc->functions())
    {
        QStringList params;
        params.append(QString::number(func->id()));
        params.append(QString::number(func->type()));
        m_functionTree->addItem(func->name(), params, func->path(true));
    }
    //m_functionTree->printTree(); // enable for debug purposes
    emit functionsListChanged();
}


