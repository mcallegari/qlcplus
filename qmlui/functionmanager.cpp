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
#include "function.h"
#include "doc.h"

FunctionManager::FunctionManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    m_filter = 0;
    m_sceneCount = m_chaserCount = m_efxCount = 0;
    m_collectionCount = m_rgbMatrixCount = m_scriptCount = 0;
    m_showCount = m_audioCount = m_videoCount = 0;

    m_functionTree = new TreeModel(this);
    QStringList treeColumns;
    treeColumns << "funcID" << "funcType";
    m_functionTree->setColumnNames(treeColumns);
    m_functionTree->enableSorting(true);
/*
    for (int i = 0; i < 10; i++)
    {
        QStringList vars;
        vars << QString::number(i) << 0;
        m_functionTree->addItem(QString("Entry %1").arg(i), vars);
    }
*/

    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotUpdateFunctionsTree()));
}

QVariant FunctionManager::functionsList()
{
    return QVariant::fromValue(m_functionTree);
}

void FunctionManager::setFunctionFilter(quint32 filter, bool enable)
{
    if (enable)
        m_filter |= filter;
    else
        m_filter &= ~filter;
    slotUpdateFunctionsTree();
}

void FunctionManager::selectFunction(quint32 id, QQuickItem *item, bool multiSelection)
{
    if (multiSelection == false)
    {
        foreach(selectedFunction f, m_selectedFunctions)
        {
            f.m_item->setProperty("isSelected", false);
        }
        m_selectedFunctions.clear();
    }
    selectedFunction sf;
    sf.m_fID = id;
    sf.m_item = item;
    m_selectedFunctions.append(sf);
}

void FunctionManager::slotUpdateFunctionsTree()
{
    m_sceneCount = m_chaserCount = m_efxCount = 0;
    m_collectionCount = m_rgbMatrixCount = m_scriptCount = 0;
    m_showCount = m_audioCount = m_videoCount = 0;

    m_functionTree->clear();
    foreach(Function *func, m_doc->functions())
    {
        if (m_filter == 0 || m_filter & func->type())
        {
            QStringList params;
            params.append(QString::number(func->id()));
            params.append(QString::number(func->type()));
            m_functionTree->addItem(func->name(), params, func->path(true));
        }
        switch (func->type())
        {
            case Function::Scene: m_sceneCount++; break;
            case Function::Chaser: m_chaserCount++; break;
            case Function::EFX: m_efxCount++; break;
            case Function::Collection: m_collectionCount++; break;
            case Function::RGBMatrix: m_rgbMatrixCount++; break;
            case Function::Script: m_scriptCount++; break;
            case Function::Show: m_showCount++; break;
            case Function::Audio: m_audioCount++; break;
            case Function::Video: m_videoCount++; break;
            default:
            break;
        }
    }
    //m_functionTree->printTree(); // enable for debug purposes

    emit sceneCountChanged();
    emit chaserCountChanged();
    emit efxCountChanged();
    emit collectionCountChanged();
    emit rgbMatrixCountChanged();
    emit scriptCountChanged();
    emit showCountChanged();
    emit audioCountChanged();
    emit videoCountChanged();

    emit functionsListChanged();
}


