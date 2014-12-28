/*
  Q Light Controller Plus - Fixture Definition Editor
  addchannelsdialog.cpp

  Copyright (C) Massimo Callegari

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

#include "qlcchannel.h"
#include "addchannelsdialog.h"
#include "ui_addchannelsdialog.h"

AddChannelsDialog::AddChannelsDialog(QList<QLCChannel *> allList, QVector<QLCChannel *> modeList, QWidget *parent) :
    QDialog(parent)
  , m_channelsList(allList)
{
    setupUi(this);

    m_allTree->setIconSize(QSize(32, 32));
    m_modeTree->setIconSize(QSize(32, 32));

    m_allTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_allTree->setDragEnabled(true);
    m_allTree->setDragDropMode(QAbstractItemView::InternalMove);
    m_modeTree->setAcceptDrops(true);
    m_modeTree->setDropIndicatorShown(true);
    m_modeTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_addChannel, SIGNAL(clicked()),
            this, SLOT(slotAddChannel()));
    connect(m_removeChannel, SIGNAL(clicked()),
            this, SLOT(slotRemoveChannel()));

    fillChannelsTrees(m_channelsList, modeList);
}

AddChannelsDialog::~AddChannelsDialog()
{

}

QList<QLCChannel *> AddChannelsDialog::getModeChannelsList()
{
    QList<QLCChannel *> retList;
    for (int i = 0; i < m_modeTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_modeTree->topLevelItem(i);
        if (item == NULL)
            continue;
        int idx = item->data(0, Qt::UserRole).toInt();
        if (idx < 0 || idx >= m_channelsList.count())
            continue;
        QLCChannel *ch = m_channelsList.at(idx);
        retList.append(ch);
    }
    return retList;
}

void AddChannelsDialog::fillChannelsTrees(QList<QLCChannel *> allList, QVector<QLCChannel *> modeList)
{
    int i = 0;
    foreach (QLCChannel *ch, allList)
    {
        QTreeWidgetItem *item = NULL;
        if (modeList.contains(ch) == false)
            item = new QTreeWidgetItem(m_allTree);
        else
            item = new QTreeWidgetItem(m_modeTree);

        item->setText(0, ch->name());
        item->setIcon(0, ch->getIcon());
        item->setData(0, Qt::UserRole, QVariant(i));
        i++;
    }
}

void AddChannelsDialog::slotAddChannel()
{
    QList<QTreeWidgetItem*> selection = m_allTree->selectedItems();
    if (selection.count() == 0)
        return;

    foreach(QTreeWidgetItem *item, selection)
    {
        QTreeWidgetItem *newItem = item->clone();
        m_modeTree->addTopLevelItem(newItem);
        m_allTree->takeTopLevelItem(m_allTree->indexOfTopLevelItem(item));
    }
}

void AddChannelsDialog::slotRemoveChannel()
{
    QList<QTreeWidgetItem*> selection = m_modeTree->selectedItems();
    if (selection.count() == 0)
        return;

    foreach(QTreeWidgetItem *item, selection)
    {
        QTreeWidgetItem *newItem = item->clone();
        m_allTree->addTopLevelItem(newItem);
        m_modeTree->takeTopLevelItem(m_modeTree->indexOfTopLevelItem(item));
    }
}
