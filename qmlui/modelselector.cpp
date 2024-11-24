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
    , m_previousIndex(-1)
    , m_itemsCount(0)
{
}

ModelSelector::~ModelSelector()
{
}

void ModelSelector::selectSingleItem(int index, ListModel *model)
{
    if (model == nullptr)
        return;

    QModelIndex idx = model->index(index, 0, QModelIndex());
    model->setDataWithRole(idx, "isSelected", true);
    m_selectedIndices.append(index);
    m_itemsCount++;

    emit itemSelectionChanged(index, true);
}

void ModelSelector::selectItem(int index, ListModel *model, int keyModifiers)
{
    if (model == nullptr)
        return;

    //qDebug() << "select item with index:" << index;
    if (keyModifiers == 0)
        resetSelection(model);

    // handle multirow selection
    if (keyModifiers & Qt::ShiftModifier)
    {
        if (index == m_previousIndex)
            return;

        int startIndex = index > m_previousIndex ? m_previousIndex + 1 : index;
        int endIndex = index > m_previousIndex ? index : m_previousIndex - 1;

        for (int i = startIndex; i <= endIndex; i++)
            selectSingleItem(i, model);
    }
    else
    {
        // Ctrl + select a single item
        selectSingleItem(index, model);
        m_previousIndex = index;
    }
    emit itemsCountChanged(m_itemsCount);
}

void ModelSelector::resetSelection(ListModel *model)
{
    for (quint32 &sidx : m_selectedIndices)
    {
        QModelIndex idx = model->index(int(sidx), 0, QModelIndex());
        model->setDataWithRole(idx, "isSelected", false);
        emit itemSelectionChanged(sidx, false);
    }

    m_selectedIndices.clear();
    m_itemsCount = 0;
    m_previousIndex = -1;
}

QVariantList ModelSelector::itemsList()
{
    QVariantList list;
    for (quint32 &sidx : m_selectedIndices)
        list.append(sidx);

    return list;
}

int ModelSelector::itemsCount() const
{
    return m_itemsCount;
}


