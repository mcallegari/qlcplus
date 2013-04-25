/*
  Q Light Controller Plus
  channelsconfiguration.cpp

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

#include <QDebug>

#include "channelsconfiguration.h"
#include "qlcfixturedef.h"
#include "doc.h"

#define KColumnName  0
#define KColumnType  1
#define KColumnFade  2
#define KColumnChIdx 3
#define KColumnID    4

ChannelsConfiguration::ChannelsConfiguration(Doc *doc, QWidget *parent)
    : QDialog(parent),
    m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);
    updateFixturesTree();

    connect(m_channelsTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChecked(QTreeWidgetItem*, int)));
}

ChannelsConfiguration::~ChannelsConfiguration()
{
}

void ChannelsConfiguration::updateFixturesTree()
{
    m_channelsTree->clear();
    m_channelsTree->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_channelsTree->setIconSize(QSize(24, 24));

    foreach(Fixture *fxi, m_doc->fixtures())
    {
        QTreeWidgetItem *topItem = NULL;
        quint32 uni = fxi->universe();
        for (int i = 0; i < m_channelsTree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = m_channelsTree->topLevelItem(i);
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
            topItem = new QTreeWidgetItem(m_channelsTree);
            topItem->setText(KColumnName, tr("Universe %1").arg(uni + 1));
            topItem->setText(KColumnID, QString::number(uni));
            topItem->setExpanded(true);
        }

        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);
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
            if (fxi->channelCanFade(c))
                item->setCheckState(KColumnFade, Qt::Checked);
            else
                item->setCheckState(KColumnFade, Qt::Unchecked);
            item->setText(KColumnID, QString::number(fxi->id()));
            item->setText(KColumnChIdx, QString::number(c));
        }
    }
}

void ChannelsConfiguration::slotItemChecked(QTreeWidgetItem *item, int col)
{
    if (m_applyAllCheck->isChecked() == false || col != KColumnFade ||
        item->text(KColumnID).isEmpty())
        return;

    Fixture *fixture = m_doc->fixture(item->text(KColumnID).toUInt());
    if (fixture == NULL)
        return;

    const QLCFixtureDef *def = fixture->fixtureDef();
    if (def == NULL)
        return;

    QString manufacturer = def->manufacturer();
    QString model = def->model();
    int chIdx = item->text(KColumnChIdx).toInt();
    Qt::CheckState enable = item->checkState(KColumnFade);

    qDebug() << "Manuf:" << manufacturer << ", model:" << model << ", ch:" << chIdx;

    for (int t = 0; t < m_channelsTree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_channelsTree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fixItem = uniItem->child(f);
            quint32 fxID = fixItem->text(KColumnID).toUInt();
            Fixture *fxi = m_doc->fixture(fxID);
            if (fxi != NULL)
            {
                const QLCFixtureDef *tmpDef = fxi->fixtureDef();
                if (tmpDef != NULL)
                {
                    QString tmpManuf = tmpDef->manufacturer();
                    QString tmpModel = tmpDef->model();
                    if (tmpManuf == manufacturer && tmpModel == model)
                    {
                        QTreeWidgetItem* item = fixItem->child(chIdx);
                        if (item != NULL)
                            item->setCheckState(KColumnFade, enable);
                    }
                }
            }
        }
    }
}

void ChannelsConfiguration::accept()
{
    QList<int> excludeList;

    for (int t = 0; t < m_channelsTree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_channelsTree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fixItem = uniItem->child(f);
            quint32 fxID = fixItem->text(KColumnID).toUInt();
            Fixture *fxi = m_doc->fixture(fxID);
            if (fxi != NULL)
            {
                excludeList.clear();
                for (int c = 0; c < fixItem->childCount(); c++)
                {
                    QTreeWidgetItem *chanItem = fixItem->child(c);
                    if (chanItem->checkState(KColumnFade) == Qt::Unchecked)
                        excludeList.append(c);
                }
                fxi->setExcludeFadeChannels(excludeList);
            }
        }
    }

    /* Close dialog */
    QDialog::accept();
}
