/*
  Q Light Controller Plus
  fixturetreewidget.h

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

#ifndef FIXTURETREEWIDGET_H
#define FIXTURETREEWIDGET_H

#include <QTreeWidget>

#include "grouphead.h"

class FixtureGroup;
class Fixture;
class Doc;

#define PROP_ID       Qt::UserRole
#define PROP_UNIVERSE Qt::UserRole + 1
#define PROP_GROUP    Qt::UserRole + 2
#define PROP_HEAD     Qt::UserRole + 3
#define PROP_CHANNEL  Qt::UserRole + 4

class FixtureTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:

    FixtureTreeWidget(Doc* doc, quint32 flags, QWidget *parent = 0);

    void updateTree();

    /** Get a QTreeWidgetItem whose fixture ID is $id */
    QTreeWidgetItem* fixtureItem(quint32 id) const;

    /** Get a QTreeWidgetItem whose group ID is $id */
    QTreeWidgetItem* groupItem(quint32 id) const;

    /** Update a single fixture's data into a QTreeWidgetItem */
    void updateFixtureItem(QTreeWidgetItem* item, Fixture* fixture);

    /** Update a group's data to and under $item */
    void updateGroupItem(QTreeWidgetItem* item, const FixtureGroup* grp);

    /** Return the number of universes added to the tree during the last
     *  updateTree call */
    int universeCount();

    /** Return the number of fixtures added to the tree during the last
     *  updateTree call */
    int fixturesCount();

    /** Return the number of channels added to the tree during the last
     *  updateTree call */
    int channelsCount();

protected slots:
    void slotItemExpanded();

private:
    Doc *m_doc;

    // counters
    int m_universesCount;
    int m_fixturesCount;
    int m_channelsCount;

    /************************************************************************
     * Tree visualization flags
     ************************************************************************/
public:

    enum TreeFlags
    {
        UniverseNumber   = 1 << 0,
        AddressRange     = 1 << 1,
        ChannelType      = 1 << 2,
        HeadsNumber      = 1 << 3,
        Manufacturer     = 1 << 4,
        Model            = 1 << 5,
        ShowGroups       = 1 << 6,
        ShowHeads        = 1 << 7,
        ChannelSelection = 1 << 8
    };

    void setFlags(quint32 flags);

    quint32 flags();

private:
    int m_uniColumn;
    int m_addressColumn;
    int m_typeColumn;
    int m_headsColumn;
    int m_manufColumn;
    int m_modelColumn;
    bool m_showGroups;
    bool m_showHeads;
    bool m_channelSelection;

    /****************************************************************************
     * Disabled items
     ****************************************************************************/
public:
    /** Disable (==prevent selection of) a list of fixtures */
    void setDisabledFixtures(const QList <quint32>& disabled);

    /** Disable (==prevent selection of) a list of heads */
    void setDisabledHeads(const QList <GroupHead>& disabled);
    
private:
    QList <quint32> m_disabledFixtures;
    QList <GroupHead> m_disabledHeads;

    /************************************************************************
     * Selected items
     ************************************************************************/
public:
    /** List of selected fixtures */
    QList <quint32> selectedFixtures();

    /** Get a list of selected fixture heads (valid only with ShowHeads flag) */
    QList <GroupHead> selectedHeads();

protected:
    void updateSelections();

private:
    QList <quint32> m_selectedFixtures;
    QList <GroupHead> m_selectedHeads;

    /****************************************************************************
     * Channels selection
     ****************************************************************************/
public:
    /** Set the channels that need to be selected in the tree.
     *  $channels is an array of 0/1 values where 0 means deselected
     *  and 1 means selected.
     *  The array indices represent absolute channel addresses
     *  across universes
     */
    void setChannelsMask(QByteArray channels);

private:
    QByteArray m_channelsMask;
};

#endif // FIXTURETREEWIDGET_H
