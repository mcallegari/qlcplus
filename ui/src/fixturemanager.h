/*
  Q Light Controller
  fixturemanager.h

  Copyright (c) Heikki Junnila

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

#ifndef FIXTUREMANAGER_H
#define FIXTUREMANAGER_H

#include <QWidget>

#include "function.h"
#include "fixture.h"
#include "doc.h"

class QLCFixtureDefCache;
class FixtureGroupEditor;
class QTreeWidgetItem;
class QTextBrowser;
class QTreeWidget;
class QTabWidget;
class OutputMap;
class QSplitter;
class QAction;
class QMenu;

#define KXMLQLCFixtureManager "FixtureManager"
#define KXMLQLCFixtureManagerSplitterSize "SplitterSize"

class FixtureManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FixtureManager)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    FixtureManager(QWidget* parent, Doc* doc);
    ~FixtureManager();

    /** Get the singleton instance */
    static FixtureManager* instance();

private:
    /** The singleton FixtureManager instance */
    static FixtureManager* s_instance;

    /********************************************************************
     * Doc signal handlers
     ********************************************************************/
public slots:
    /** Callback for Doc::fixtureRemoved() signals */
    void slotFixtureRemoved(quint32 id);

    /** Callback for Doc::channelsGroupRemoved() signals */
    void slotChannelsGroupRemoved(quint32 id);

    /** Callback that listens to mode change signals */
    void slotModeChanged(Doc::Mode mode);

    /** Callback that listens to fixture group removals */
    void slotFixtureGroupRemoved(quint32 id);

    /** Callback that listens to fixture group modifications */
    void slotFixtureGroupChanged(quint32 id);

    /** Callback that listens to workspace loading */
    void slotDocLoaded();

private:
    Doc* m_doc;

    /********************************************************************
     * Data view
     ********************************************************************/
public:
    /** Update the list of fixtures */
    void updateView();

    /** Update the list of channels group */
    void updateChannelsGroupView();

private:
    /** Open a fixture selector to add new fixtures */
    void addFixture();

    /** Open a channels selector to add new channels group */
    void addChannelsGroup();

    /** Remove a previously created fixture */
    void removeFixture();

    /** Remove a previously created channels group */
    void removeChannelsGroup();

    /** Get a QTreeWidgetItem whose fixture ID is $id */
    QTreeWidgetItem* fixtureItem(quint32 id) const;

    /** Get a QTreeWidgetItem whose group ID is $id */
    QTreeWidgetItem* groupItem(quint32 id) const;

    /** Construct the list view and data view */
    void initDataView();

    /** Update a single fixture's data into a QTreeWidgetItem */
    void updateFixtureItem(QTreeWidgetItem* item, const Fixture* fxi);

    /** Update a group's data to and under $item */
    void updateGroupItem(QTreeWidgetItem* item, const FixtureGroup* grp);

    /** Handle single fixture selection */
    void fixtureSelected(quint32 id);

    /** Handle fixture group selection */
    void fixtureGroupSelected(FixtureGroup* grp);

    /** Create the text browser for displaying information */
    void createInfo();

private slots:
    /** Callback for fixture list selection changes */
    void slotSelectionChanged();

    /** Callback for channels group selection changes */
    void slotChannelsGroupSelectionChanged();

    /** Callback for mouse double clicks */
    void slotDoubleClicked(QTreeWidgetItem* item);

    /** Callback for channels group mouse double clicks */
    void slotChannelsGroupDoubleClicked(QTreeWidgetItem*);

    /** Callback for tab selection changes */
    void slotTabChanged(int index = 0);

    /** Callback for fixture tree item expand/collapse */
    void slotFixtureItemExpanded();

private:
    /** Select a fixture group */
    void selectGroup(quint32 id);

    /** Get a CSS style sheet & HTML header for fixture info */
    QString fixtureInfoStyleSheetHeader();

    /** Get a CSS style sheet & HTML header for channels group info */
    QString channelsGroupInfoStyleSheetHeader();

private:
    QSplitter* m_splitter;
    QTreeWidget* m_fixtures_tree;
    QTreeWidget* m_channel_groups_tree;

    QTextBrowser* m_info;
    FixtureGroupEditor* m_groupEditor;
    int m_currentTabIndex;

    /********************************************************************
     * Menu & Toolbar & Actions
     ********************************************************************/
private:
    /** Construct actions for toolbar & context menu */
    void initActions();

    /** Update the contents of the group menu */
    void updateGroupMenu();

    /** Construct the toolbar */
    void initToolBar();

    /** Edit properties for the selected fixture */
    void editFixtureProperties();

    /** Edit properties for the selected channels group */
    void editChannelGroupProperties();

    /** Count the number of heads in the list of fixture items */
    int headCount(const QList <QTreeWidgetItem*>& items) const;

    QString createDialog(bool import);

private slots:
    void slotAdd();
    void slotRemove();
    void slotProperties();
    void slotFadeConfig();
    void slotRemap();
    void slotUnGroup();
    void slotGroupSelected(QAction* action);
    void slotMoveGroupUp();
    void slotMoveGroupDown();
    void slotImport();
    void slotExport();

    /** Callback for right mouse button clicks over a fixture item */
    void slotContextMenuRequested(const QPoint& pos);

private:
    QAction* m_addAction;
    QAction* m_removeAction;
    QAction* m_propertiesAction;
    QAction* m_fadeConfigAction;
    QAction* m_remapAction;
    QAction* m_groupAction;
    QAction* m_unGroupAction;
    QAction* m_newGroupAction;

    QAction* m_moveUpAction;
    QAction* m_moveDownAction;

    QAction* m_importAction;
    QAction* m_exportAction;
    QMenu* m_groupMenu;
};

#endif
