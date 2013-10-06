/*
  Q Light Controller Plus - Fixture Definition Editor
  addchannelsdialog.cpp

  Copyright (C) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "qlcchannel.h"
#include "addchannelsdialog.h"
#include "ui_addchannelsdialog.h"

AddChannelsDialog::AddChannelsDialog(QList<QLCChannel *> allList, QList<QLCChannel *> modeList, QWidget *parent) :
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

void AddChannelsDialog::fillChannelsTrees(QList<QLCChannel *> allList, QList<QLCChannel *> modeList)
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
        item->setIcon(0, ch->getIconFromGroup(ch->group()));
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
