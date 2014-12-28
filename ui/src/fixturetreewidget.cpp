/*
  Q Light Controller Plus
  fixturetreewidget.cpp

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

#include <QDebug>

#include "fixturetreewidget.h"
#include "qlcfixturedef.h"
#include "fixturegroup.h"
#include "qlcchannel.h"
#include "fixture.h"
#include "doc.h"

#define KColumnName 0

FixtureTreeWidget::FixtureTreeWidget(Doc *doc, quint32 flags, QWidget *parent)
    : QTreeWidget(parent)
    , m_doc(doc)
    , m_universesCount(0)
    , m_fixturesCount(0)
    , m_channelsCount(0)
    , m_uniColumn(-1)
    , m_addressColumn(-1)
    , m_typeColumn(-1)
    , m_headsColumn(-1)
    , m_manufColumn(-1)
    , m_modelColumn(-1)
    , m_showGroups(false)
    , m_showHeads(false)
    , m_channelSelection(false)
{
    setFlags(flags);

    setRootIsDecorated(true);
    setAllColumnsShowFocus(true);
    setSortingEnabled(true);
    sortByColumn(KColumnName, Qt::AscendingOrder);

    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)),
            this, SLOT(slotItemExpanded()));
    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)),
            this, SLOT(slotItemExpanded()));
}

void FixtureTreeWidget::setFlags(quint32 flags)
{
    // column 0 is always reserved for Fixture name
    int columnIdx = 1;
    QStringList labels;
    labels << tr("Name");

    if (flags & UniverseNumber)
    {
        m_uniColumn = columnIdx++;
        labels << tr("Universe");
    }
    if (flags & AddressRange)
    {
        m_addressColumn = columnIdx++;
        labels << tr("Address");
    }
    if (flags & ChannelType)
    {
        m_typeColumn = columnIdx++;
        labels << tr("Type");
    }
    if (flags & HeadsNumber)
    {
        m_headsColumn = columnIdx++;
        labels << tr("Heads");
    }
    if (flags & Manufacturer)
    {
        m_manufColumn = columnIdx++;
        labels << tr("Manufacturer");
    }
    if (flags & Model)
    {
        m_modelColumn = columnIdx++;
        labels << tr("Model");
    }
    if (flags & ShowGroups)
        m_showGroups = true;

    if (flags & ShowHeads)
        m_showHeads = true;

    if (flags & ChannelSelection)
        m_channelSelection = true;

    setHeaderLabels(labels);
}

/****************************************************************************
 * Disabled items
 ****************************************************************************/

void FixtureTreeWidget::setDisabledFixtures(const QList <quint32>& disabled)
{
    m_disabledHeads.clear();
    m_disabledFixtures = disabled;
}

void FixtureTreeWidget::setDisabledHeads(const QList <GroupHead>& disabled)
{
    m_disabledFixtures.clear();
    m_disabledHeads = disabled;
}

void FixtureTreeWidget::setChannelsMask(QByteArray channels)
{
    m_channelsMask = channels;
}

/****************************************************************************
 * Tree rendering
 ****************************************************************************/

QTreeWidgetItem *FixtureTreeWidget::fixtureItem(quint32 id) const
{
    for (int i = 0; i < topLevelItemCount(); i++)
    {
        QTreeWidgetItem *tItem = topLevelItem(i);
        if (tItem->childCount() > 0)
        {
            for (int c = 0; c < tItem->childCount(); c++)
            {
                QTreeWidgetItem *cItem = tItem->child(c);
                QVariant var = cItem->data(KColumnName, PROP_ID);
                if (var.isValid() == true && var.toUInt() == id)
                    return cItem;
            }
        }
    }

    return NULL;
}

QTreeWidgetItem *FixtureTreeWidget::groupItem(quint32 id) const
{
    for (int i = 0; i < topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = topLevelItem(i);
        QVariant var = item->data(KColumnName, PROP_GROUP);
        if (var.isValid() == true && var.toUInt() == id)
            return item;
    }

    return NULL;
}

void FixtureTreeWidget::updateFixtureItem(QTreeWidgetItem* item, Fixture* fixture)
{
    Q_ASSERT(item != NULL);
    if (fixture == NULL)
        return;

    item->setText(KColumnName, fixture->name());
    item->setIcon(KColumnName, fixture->getIconFromType(fixture->type()));
    item->setData(KColumnName, PROP_ID, QString::number(fixture->id()));
    if (m_channelSelection)
    {
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsTristate);
        item->setCheckState(KColumnName, Qt::Unchecked);
    }
    if (m_disabledFixtures.contains(fixture->id()) == true)
    {
        // Disable selection
        item->setFlags(0);
    }

    if (m_uniColumn > 0)
        item->setText(m_uniColumn, QString("%1").arg(fixture->universe() + 1));

    if (m_addressColumn)
    {
        QString s;
        s.sprintf("%.3d - %.3d", fixture->address() + 1, fixture->address() + fixture->channels());
        item->setText(m_addressColumn, s);
    }

    if (m_headsColumn > 0)
        item->setText(m_headsColumn, QString::number(fixture->heads()));

    if (m_manufColumn > 0)
    {
        if (fixture->fixtureDef() == NULL)
            item->setText(m_manufColumn, tr("Generic"));
        else
            item->setText(m_manufColumn, fixture->fixtureDef()->manufacturer());
    }

    if (m_modelColumn > 0)
    {
        if (fixture->fixtureDef() == NULL)
            item->setText(m_modelColumn, tr("Generic"));
        else
            item->setText(m_modelColumn, fixture->fixtureDef()->model());
    }

    if (m_showHeads == true)
    {
        int disabled = 0;

        for (int i = 0; i < fixture->heads(); i++)
        {
            QTreeWidgetItem* headItem = new QTreeWidgetItem(item);
            headItem->setText(KColumnName, QString("%1 %2").arg(tr("Head")).arg(i + 1));
            headItem->setData(KColumnName, PROP_HEAD, i);
            if (m_disabledHeads.contains(GroupHead(fixture->id(), i)) == true)
            {
                headItem->setFlags(0); // Disable selection
                disabled++;
            }
        }

        // Disable the whole fixture if all heads are disabled
        if (disabled == fixture->heads())
            item->setFlags(0);
    }

    if (m_channelSelection == true)
    {
        quint32 baseAddress = fixture->universeAddress();
        for (quint32 c = 0; c < fixture->channels(); c++)
        {
            const QLCChannel* channel = fixture->channel(c);
            QTreeWidgetItem *cItem = new QTreeWidgetItem(item);
            cItem->setText(KColumnName, QString("%1:%2").arg(c + 1)
                          .arg(channel->name()));
            cItem->setIcon(KColumnName, channel->getIcon());
            cItem->setData(KColumnName, PROP_CHANNEL, c);
            if (m_typeColumn > 0)
            {
                if (channel->group() == QLCChannel::Intensity &&
                    channel->colour() != QLCChannel::NoColour)
                    cItem->setText(m_typeColumn, QLCChannel::colourToString(channel->colour()));
                else
                    cItem->setText(m_typeColumn, QLCChannel::groupToString(channel->group()));
            }

            cItem->setFlags(cItem->flags() | Qt::ItemIsUserCheckable);
            if (m_channelsMask.at(baseAddress + c) == 1)
                cItem->setCheckState(KColumnName, Qt::Checked);
            else
                cItem->setCheckState(KColumnName, Qt::Unchecked);
        }
    }
}

void FixtureTreeWidget::updateGroupItem(QTreeWidgetItem* item, const FixtureGroup* grp)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(grp != NULL);

    item->setText(KColumnName, grp->name());
    item->setIcon(KColumnName, QIcon(":/group.png"));
    item->setData(KColumnName, PROP_GROUP, grp->id());

    // This should be a safe check because simultaneous add/removal is not possible,
    // which could result in changes in fixtures but with the same fixture count.
    if (item->childCount() != grp->fixtureList().size())
    {
        // Remove existing children
        while (item->childCount() > 0)
            delete item->child(0);

        // Add group's children
        foreach (quint32 id, grp->fixtureList())
        {
            QTreeWidgetItem* grpItem = new QTreeWidgetItem(item);
            updateFixtureItem(grpItem, m_doc->fixture(id));
        }
    }
}

int FixtureTreeWidget::universeCount()
{
    return m_universesCount;
}

int FixtureTreeWidget::fixturesCount()
{
    return m_fixturesCount;
}

int FixtureTreeWidget::channelsCount()
{
    return m_channelsCount;
}

QList<quint32> FixtureTreeWidget::selectedFixtures()
{
    updateSelections();
    return m_selectedFixtures;
}

QList<GroupHead> FixtureTreeWidget::selectedHeads()
{
    updateSelections();
    return m_selectedHeads;
}

void FixtureTreeWidget::updateSelections()
{
    m_selectedFixtures.clear();
    m_selectedHeads.clear();

    QListIterator <QTreeWidgetItem*> it(selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem *item = it.next();
        // A selected item can be:
        // 1) a fixture
        // 2) a group
        // 3) a head
        // 4) a universe

        QVariant fxIDVar = item->data(KColumnName, PROP_ID);
        QVariant grpIDVar = item->data(KColumnName, PROP_GROUP);
        QVariant headVar = item->data(KColumnName, PROP_HEAD);
        QVariant uniIDVar = item->data(KColumnName, PROP_UNIVERSE);

        qDebug() << "uni ID:" << uniIDVar;

        // Case 1: is there a valid fixture ID ?
        if (fxIDVar.isValid())
        {
            quint32 fxi = fxIDVar.toUInt();
            m_selectedFixtures << fxi;

            // fill also the non-diabled heads, if present
            if (m_showHeads && item->childCount() > 0)
            {
                for (int h = 0; h < item->childCount(); h++)
                {
                    QTreeWidgetItem *hItem = item->child(h);
                    if (hItem->isDisabled() == false)
                    {
                        QVariant chHeadVar = hItem->data(KColumnName, PROP_HEAD);
                        if (chHeadVar.isValid())
                        {
                            GroupHead gh(fxi, chHeadVar.toInt());
                            if (m_selectedHeads.contains(gh) == false)
                                m_selectedHeads << gh;
                        }
                    }
                }
            }
        }
        // Case 2: is there a valid group ID ?
        else if (grpIDVar.isValid())
        {
            // in this case cycle through the children and get each
            // fixture ID
            for (int i = 0; i < item->childCount(); i++)
            {
                QTreeWidgetItem *child = item->child(i);
                QVariant chFxIDVar = child->data(KColumnName, PROP_ID);
                if (chFxIDVar.isValid() && child->isDisabled() == false)
                    m_selectedFixtures << chFxIDVar.toUInt();
            }
        }
        // Case 3: is there a valid head index ?
        else if (headVar.isValid())
        {
            Q_ASSERT(item->parent() != NULL);
            quint32 fxi = item->parent()->data(KColumnName, PROP_ID).toUInt();
            GroupHead gh(fxi, headVar.toInt());
            if (m_selectedHeads.contains(gh) == false)
                m_selectedHeads << gh;
        }
        // Case 4: is there a valid universe index ?
        else if (uniIDVar.isValid())
        {
            qDebug() << "Valid universe....";
            // in this case cycle through the children and get each
            // fixture ID
            for (int i = 0; i < item->childCount(); i++)
            {
                QTreeWidgetItem *child = item->child(i);
                QVariant chFxIDVar = child->data(KColumnName, PROP_ID);
                if (chFxIDVar.isValid() && child->isDisabled() == false)
                    m_selectedFixtures << chFxIDVar.toUInt();
            }
        }
    }
}

void FixtureTreeWidget::slotItemExpanded()
{
    resizeColumnToContents(KColumnName);
    resizeColumnToContents(m_uniColumn);
    resizeColumnToContents(m_typeColumn);
    resizeColumnToContents(m_headsColumn);
    resizeColumnToContents(m_manufColumn);
    resizeColumnToContents(m_modelColumn);
}

void FixtureTreeWidget::updateTree()
{
    clear();
    m_universesCount = 0;
    m_fixturesCount = 0;
    m_channelsCount = 0;

    if (m_showGroups == true)
    {
        foreach (FixtureGroup* grp, m_doc->fixtureGroups())
        {
            QTreeWidgetItem* grpItem = new QTreeWidgetItem(this);
            updateGroupItem(grpItem, grp);
        }
    }

    foreach (Fixture* fixture, m_doc->fixtures())
    {
        Q_ASSERT(fixture != NULL);

        QTreeWidgetItem *topItem = NULL;
        quint32 uni = fixture->universe();
        for (int i = 0; i < topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = topLevelItem(i);
            QVariant tVar = tItem->data(KColumnName, PROP_UNIVERSE);
            if (tVar.isValid())
            {
                quint32 tUni = tVar.toUInt();
                if (tUni == uni)
                {
                    topItem = tItem;
                    break;
                }
            }
        }
        // Haven't found this universe node ? Create it.
        if (topItem == NULL)
        {
            topItem = new QTreeWidgetItem(this);
            topItem->setText(KColumnName, m_doc->inputOutputMap()->getUniverseNameByID(uni));
            topItem->setIcon(KColumnName, QIcon(":/group.png"));
            topItem->setData(KColumnName, PROP_UNIVERSE, uni);
            topItem->setExpanded(true);
            if (m_channelSelection)
            {
                topItem->setFlags(topItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsTristate);
                topItem->setCheckState(KColumnName, Qt::Unchecked);
            }
            m_universesCount++;
        }

        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);
        updateFixtureItem(fItem, fixture);
        m_fixturesCount++;
        m_channelsCount += fixture->channels();
    }

    resizeColumnToContents(KColumnName);
    resizeColumnToContents(m_uniColumn);
    resizeColumnToContents(m_typeColumn);
    resizeColumnToContents(m_headsColumn);
    resizeColumnToContents(m_manufColumn);
    resizeColumnToContents(m_modelColumn);
}


