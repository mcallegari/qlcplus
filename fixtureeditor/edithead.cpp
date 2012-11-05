/*
  Q Light Controller - Fixture Editor
  edithead.cpp

  Copyright (C) Heikki Junnila

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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QDebug>

#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlcchannel.h"
#include "edithead.h"

EditHead::EditHead(QWidget* parent, const QLCFixtureHead& head, const QLCFixtureMode* mode)
    : QDialog(parent)
    , m_head(head)
{
    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    fillChannelTree(mode);

    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

EditHead::~EditHead()
{
}

QLCFixtureHead EditHead::head() const
{
    return m_head;
}

void EditHead::fillChannelTree(const QLCFixtureMode* mode)
{
    Q_ASSERT(mode != NULL);

    for (int i = 0; i < mode->channels().size(); i++)
    {
        const QLCChannel* ch = mode->channels().at(i);
        Q_ASSERT(ch != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(0, QString::number(i + 1));
        item->setText(1, ch->name());

        if (m_head.channels().contains(i) == false && mode->headForChannel(i) != -1)
            item->setFlags(0);
        else
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

        if (m_head.channels().contains(i) == true)
            item->setCheckState(0, Qt::Checked);
        else
            item->setCheckState(0, Qt::Unchecked);
    }
}

void EditHead::slotItemChanged(QTreeWidgetItem* item, int column)
{
    if (column != 0)
        return;

    int index = m_tree->indexOfTopLevelItem(item);

    if (item->checkState(0) == Qt::Checked)
        m_head.addChannel(index);
    else
        m_head.removeChannel(index);
}
