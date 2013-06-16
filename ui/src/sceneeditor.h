/*
  Q Light Controller
  sceneeditor.h

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#ifndef SCENEEDITOR_H
#define SCENEEDITOR_H

#include <QPointer>
#include <QWidget>
#include <QList>
#include <QMap>

#include "ui_sceneeditor.h"
#include "groupsconsole.h"
#include "fixture.h"
#include "scene.h"

class GenericDMXSource;
class SpeedDialWidget;
class FixtureConsole;
class MasterTimer;
class QComboBox;
class OutputMap;
class InputMap;
class QAction;
class Chaser;
class Doc;

class SceneEditor : public QWidget, public Ui_SceneEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneEditor)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    SceneEditor(QWidget* parent, Scene* scene, Doc* doc, bool applyValues);
    ~SceneEditor();

public slots:
    void slotFunctionManagerActive(bool active);
    void slotSetSceneValues(QList <SceneValue>&);

private:
    Doc* m_doc;
    Scene* m_scene; // The Scene that is being edited
    GenericDMXSource* m_source;

private:
    void init(bool applyValues);
    void setSceneValue(const SceneValue& scv);

private:
    bool m_initFinished;

    /*********************************************************************
     * Common
     *********************************************************************/
private slots:
    void slotTabChanged(int tab);

    void slotEnableCurrent();
    void slotDisableCurrent();

    void slotCopy();
    void slotPaste();
    void slotCopyToAll();
    void slotColorTool();
    void slotSpeedDialToggle(bool state);
    void slotBlindToggled(bool state);
    void slotRecord();
    void slotChaserComboActivated(int index);
    void slotModeChanged(Doc::Mode mode);
    void slotViewModeChanged(bool toggled, bool applyValues = true);

private:
    bool isColorToolAvailable();
    void createSpeedDials();
    Chaser* selectedChaser() const;

private:
    QAction* m_enableCurrentAction;
    QAction* m_disableCurrentAction;
    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_copyToAllAction;
    QAction* m_colorToolAction;
    QAction* m_blindAction;
    QAction* m_recordAction;
    QAction* m_speedDialAction;

    QAction* m_nextTabAction;
    QAction* m_prevTabAction;

    QAction* m_tabViewAction;

    QComboBox* m_chaserCombo;

    /*********************************************************************
     * General tab
     *********************************************************************/
private:
    QTreeWidgetItem* fixtureItem(quint32 fxi_id);
    QList <Fixture*> selectedFixtures() const;

    void addFixtureItem(Fixture* fixture);
    void removeFixtureItem(Fixture* fixture);

private slots:
    void slotNameEdited(const QString& name);
    void slotAddFixtureClicked();
    void slotRemoveFixtureClicked();

    void slotEnableAll();
    void slotDisableAll();

    void slotFadeInChanged(int ms);
    void slotFadeOutChanged(int ms);

private:
    SpeedDialWidget* m_speedDials;

    /*********************************************************************
     * Channels groups
     *********************************************************************/
private:
    void updateChannelsGroupsTab();
    GroupsConsole* groupConsoleTab(int tab);

private:
    /** Index of the Channel Groups tab. Equal to -1
        means the tab has not been created */
    int m_channelGroupsTab;

private slots:
    void slotEnableAllChannelGroups();

    void slotDisableAllChannelGroups();

    /** called when the user check/uncheck a group of m_channelGroupsTree */
    void slotChannelGroupsChanged(QTreeWidgetItem*item, int column);

    /** Called when the user moves a fader of the ChannelGroup console */
    void slotGroupValueChanged(quint32 groupID, uchar value);

    /*********************************************************************
     * Fixture tabs
     *********************************************************************/
private:
    FixtureConsole* fixtureConsole(Fixture* fixture);

    void addFixtureTab(Fixture* fixture);
    void removeFixtureTab(Fixture* fixture);
    FixtureConsole* fixtureConsoleTab(int tab);

signals:
    void fixtureValueChanged(SceneValue val);

private slots:
    void slotValueChanged(quint32 fxi, quint32 channel, uchar value);
    void slotChecked(quint32 fxi, quint32 channel, bool state);

    void slotGoToNextTab();
    void slotGoToPreviousTab();

private:
    /** Index of the currently selected tab */
    int m_currentTab;
    /** Index of the first fixture's tab */
    int m_fixtureFirstTabIndex;

    QList <FixtureConsole *> m_consoleList;

    /** Flag to indicate if some fixture channels were
     *  manually selected and copied to clipboard */
    bool m_copyFromSelection;
};

#endif
