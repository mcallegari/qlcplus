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

ModelSelector::ModelSelector(QObject *parent)
    : QObject(parent)
    , m_nodesCount(0)
    , m_itemsCount(0)
{

}

ModelSelector::~ModelSelector()
{
    m_selectedItems.clear();
}

void ModelSelector::selectItem(quint32 id, QQuickItem *item, bool multiSelection)
{
    if (multiSelection == false)
    {
        foreach(selectedItem sf, m_selectedItems)
            sf.m_item->setProperty("isSelected", false);

        m_selectedItems.clear();
        m_nodesCount = 0;
        m_itemsCount = 0;
    }

    selectedItem si;
    si.m_ID = id;
    si.m_item = item;
    item->setProperty("isSelected", true);
    m_selectedItems.append(si);
    if ((int)id == -1)
    {
        m_nodesCount++;
        emit nodesCountChanged(m_nodesCount);
    }
    else
    {
        m_itemsCount++;
        emit itemsCountChanged(m_itemsCount);
    }
}

QVariantList ModelSelector::itemsList()
{
    QVariantList list;
    foreach(selectedItem si, m_selectedItems)
        if ((int)si.m_ID != -1)
            list.append(si.m_ID);

    return list;
}

void ModelSelector::resetSelection()
{
    //qDebug() << "[ModelSelector] resetSelection";
    m_selectedItems.clear();
}

int ModelSelector::nodesCount() const
{
    return m_nodesCount;
}

int ModelSelector::itemsCount() const
{
    return m_itemsCount;
}

