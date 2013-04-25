/*
  Q Light Controller
  channelselection.cpp

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

#include "selectinputchannel.h"
#include "channelselection.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "channelsgroup.h"
#include "fixture.h"
#include "doc.h"

#define KColumnName  0
#define KColumnType  1
#define KColumnGroup 2
#define KColumnChIdx 3
#define KColumnID    4

ChannelSelection::ChannelSelection(QWidget* parent, Doc* doc, ChannelsGroup *group)
    : QDialog(parent)
    , m_doc(doc)
    , m_chansGroup(group)
    , m_checkedChannels(0)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(m_chansGroup != NULL);

    setupUi(this);

    m_tree->header()->setSectionHidden(KColumnID, true);
    m_tree->setSelectionMode(QAbstractItemView::MultiSelection);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_tree->setAlternatingRowColors(true);
    m_tree->setIconSize(QSize(20, 20));

    m_groupNameEdit->setText(group->name());

    QList <SceneValue> chans = group->getChannels();
    int ch = 0;

    foreach (Fixture* fxi, m_doc->fixtures())
    {
        QTreeWidgetItem *topItem = NULL;
        quint32 uni = fxi->universe();
        for (int i = 0; i < m_tree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = m_tree->topLevelItem(i);
            quint32 tUni = tItem->text(KColumnID).toUInt();
            if (tUni == uni)
            {
                topItem = tItem;
                break;
            }
        }
        // Haven't found this universe node ? Create it.
        if (topItem == NULL)
        {
            topItem = new QTreeWidgetItem(m_tree);
            topItem->setText(KColumnName, tr("Universe %1").arg(uni + 1));
            topItem->setText(KColumnID, QString::number(uni));
            topItem->setExpanded(true);
        }

        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);
        fItem->setExpanded(true);
        fItem->setText(KColumnName, fxi->name());
        fItem->setIcon(KColumnName, fxi->getIconFromType(fxi->type()));
        fItem->setText(KColumnID, QString::number(fxi->id()));

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel* channel = fxi->channel(c);
            QTreeWidgetItem *item = new QTreeWidgetItem(fItem);
            item->setText(KColumnName, QString("%1:%2").arg(c + 1)
                          .arg(channel->name()));
            item->setIcon(KColumnName, channel->getIconFromGroup(channel->group()));
            if (channel->group() == QLCChannel::Intensity &&
                channel->colour() != QLCChannel::NoColour)
                item->setText(KColumnType, QLCChannel::colourToString(channel->colour()));
            else
                item->setText(KColumnType, QLCChannel::groupToString(channel->group()));

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if (chans.count() > ch &&
                chans.at(ch).fxi == fxi->id() &&
                chans.at(ch).channel == c)
            {
                item->setCheckState(KColumnGroup, Qt::Checked);
                m_checkedChannels++;
                ch++;
            }
            else
                item->setCheckState(KColumnGroup, Qt::Unchecked);
            item->setText(KColumnID, QString::number(fxi->id()));
            item->setText(KColumnChIdx, QString::number(c));
        }
    }

    m_inputSource = group->inputSource();
    updateInputSource();

    if (m_checkedChannels == 0)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChecked(QTreeWidgetItem*, int)));

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseInputClicked()));
}

ChannelSelection::~ChannelSelection()
{
}

void ChannelSelection::accept()
{
    m_chansGroup->resetList();

    for (int t = 0; t < m_tree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_tree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fixItem = uniItem->child(f);
            quint32 fxID = fixItem->text(KColumnID).toUInt();
            Fixture *fxi = m_doc->fixture(fxID);
            if (fxi != NULL)
            {
                for (int c = 0; c < fixItem->childCount(); c++)
                {
                    QTreeWidgetItem *chanItem = fixItem->child(c);
                    if (chanItem->checkState(KColumnGroup) == Qt::Checked)
                    {
                        m_chansGroup->addChannel(QString(chanItem->text(KColumnID)).toUInt(),
                                                 QString(chanItem->text(KColumnChIdx)).toUInt());
                        qDebug() << "Added channel with ID:" << chanItem->text(KColumnID) << ", and channel:" << chanItem->text(KColumnChIdx);
                    }
                }
            }
        }
    }

    m_chansGroup->setName(m_groupNameEdit->text());
    m_chansGroup->setInputSource(m_inputSource);
    QDialog::accept();
}

void ChannelSelection::slotItemChecked(QTreeWidgetItem *item, int col)
{
    if (col != KColumnGroup || item->text(KColumnID).isEmpty())
        return;

    if (item->checkState(col) == Qt::Checked)
        m_checkedChannels++;
    else
        m_checkedChannels--;

    if (m_checkedChannels > 0)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void ChannelSelection::slotAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
}

void ChannelSelection::slotInputValueChanged(quint32 universe, quint32 channel)
{
    m_inputSource = QLCInputSource(universe, channel);
    updateInputSource();
}

void ChannelSelection::slotChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_inputSource = QLCInputSource(sic.universe(), sic.channel());
        updateInputSource();
    }
}

void ChannelSelection::updateInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputMap()->inputSourceNames(m_inputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_inputUniverseEdit->setText(uniName);
    m_inputChannelEdit->setText(chName);
}


