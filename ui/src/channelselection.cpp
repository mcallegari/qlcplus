/*
  Q Light Controller
  functionselection.cpp

  Copyright (c) Massimo Callegari

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

#include <QPushButton>
#include <QDebug>

#include "channelselection.h"
#include "qlcfixturedef.h"
#include "channelsgroup.h"
#include "fixture.h"
#include "doc.h"

#define KColumnName     0
#define KColumnID       1
#define KColumnChannel  2
#define KColumnType     3

ChannelSelection::ChannelSelection(QWidget* parent, Doc* doc, ChannelsGroup *group)
    : QDialog(parent)
    , m_doc(doc)
    , m_chansGroup(group)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(m_chansGroup != NULL);

    setupUi(this);

    m_tree->header()->setSectionHidden(KColumnID, true);
    m_tree->setSelectionMode(QAbstractItemView::MultiSelection);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_tree->setAlternatingRowColors(true);

    m_groupNameEdit->setText(group->name());

    QList <SceneValue> chans = group->getChannels();
    int c = 0;

    foreach (Fixture* fixture, m_doc->fixtures())
    {
        for (quint32 i = 0; i < fixture->channels(); i++)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
            item->setText(KColumnName, fixture->name());
            item->setText(KColumnID, QString("%1").arg(fixture->id()));
            item->setText(KColumnChannel, QString("%1").arg(i + 1));
            const QLCFixtureDef *def = fixture->fixtureDef();
            item->setText(KColumnType, def->channels().at(i)->name());
            if (chans.count() > c &&
                chans.at(c).fxi == fixture->id() && chans.at(c).channel == i)
            {
                item->setSelected(true);
                c++;
            }
        }
    }

    if (chans.count() == 0)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
}

ChannelSelection::~ChannelSelection()
{
}

int ChannelSelection::exec()
{

    return QDialog::exec();
}

void ChannelSelection::accept()
{
    QListIterator <QTreeWidgetItem *> it(m_tree->selectedItems());
    m_chansGroup->resetList();
    while (it.hasNext() == true)
    {
        QTreeWidgetItem *channel(it.next());
        m_chansGroup->addChannel(QString(channel->text(KColumnID)).toUInt(),
                                 QString(channel->text(KColumnChannel)).toUInt() - 1);
        qDebug() << "Added channel with ID: " << channel->text(KColumnID) << ", and channel: " << channel->text(KColumnChannel);
    }

    m_chansGroup->setName(m_groupNameEdit->text());
    QDialog::accept();
}

void ChannelSelection::slotItemSelectionChanged()
{
    if (m_tree->selectedItems().count() > 0)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

