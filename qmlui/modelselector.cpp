/*
  Q Light Controller Plus
  modelselector.cpp

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

#include "modelselector.h"
#include "listmodel.h"

ModelSelector::ModelSelector(QObject *parent)
    : QObject(parent)
    , m_itemsCount(0)
{
}

ModelSelector::~ModelSelector()
{
    m_selectedIndices.clear();
}

void ModelSelector::selectItem(quint32 index, ListModel *model, bool multiSelection)
{
    qDebug() << "select item with ID:" << index;
    if (multiSelection == false)
    {
        foreach(quint32 sidx, m_selectedIndices)
        {
            QModelIndex idx = model->index(sidx, 0, QModelIndex());
            model->setDataWithRole(idx, "isSelected", false);
        }

        m_selectedIndices.clear();
        m_itemsCount = 0;
    }

    QModelIndex idx = model->index(index, 0, QModelIndex());
    model->setDataWithRole(idx, "isSelected", true);
    m_selectedIndices.append(index);
    m_itemsCount++;
    emit itemsCountChanged(m_itemsCount);
}

QVariantList ModelSelector::itemsList()
{
    QVariantList list;
    foreach(quint32 sidx, m_selectedIndices)
        list.append(sidx);

    return list;
}

void ModelSelector::resetSelection()
{
    //qDebug() << "[ModelSelector] resetSelection";
    m_selectedIndices.clear();
}

int ModelSelector::itemsCount() const
{
    return m_itemsCount;
}

