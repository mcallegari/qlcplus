/*
  Q Light Controller
  sceneeditor.h

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#ifndef SCENEEDITOR_H
#define SCENEEDITOR_H

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
class SceneUiState;

/** @addtogroup ui_functions
 * @{
 */

class SceneEditor : public QWidget, public Ui_SceneEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneEditor)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /*!
       \param applyValues - true for scenes, false for sequences
     */
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
    SceneUiState * sceneUiState();

private:
    bool m_initFinished;

    /*********************************************************************
     * Common
     *********************************************************************/
public:
    void setBlindModeEnabled(bool active);

private slots:
    void slotTabChanged(int tab);

    void slotEnableCurrent();
    void slotDisableCurrent();

    void slotCopy();
    void slotPaste();
    void slotCopyToAll();
    void slotColorTool();
    void slotPositionTool();
    QColor slotColorSelectorChanged(const QColor &color);
    void slotPositionSelectorChanged(const QPointF &position);
    void slotSpeedDialToggle(bool state);
    void slotBlindToggled(bool state);
    void slotRecord();
    void slotChaserComboActivated(int index);
    void slotModeChanged(Doc::Mode mode);
    void slotViewModeChanged(bool toggled, bool applyValues = true);

private:
    bool isColorToolAvailable();
    bool isPositionToolAvailable();
    void createSpeedDials();
    Chaser* selectedChaser() const;

private:
    QAction* m_enableCurrentAction;
    QAction* m_disableCurrentAction;
    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_copyToAllAction;
    QAction* m_colorToolAction;
    QAction* m_positionToolAction;
    QAction* m_blindAction;
    QAction* m_recordAction;
    QAction* m_speedDialAction;

    QAction* m_nextTabAction;
    QAction* m_prevTabAction;

    QAction* m_tabViewAction;

    QComboBox* m_chaserCombo;
    QLineEdit* m_nameEdit;

    /*********************************************************************
     * General tab
     *********************************************************************/
private:
    QTreeWidgetItem* fixtureItem(quint32 fxi_id);
    QList <Fixture*> selectedFixtures() const;

    bool addFixtureItem(Fixture* fixture);
    void removeFixtureItem(Fixture* fixture);

private slots:
    void slotNameEdited(const QString& name);
    void slotAddFixtureClicked();
    void slotRemoveFixtureClicked();

    void slotEnableAll();
    void slotDisableAll();

    void slotFadeInChanged(int ms);
    void slotFadeOutChanged(int ms);
    void slotDialDestroyed(QObject* dial);

private:
    SpeedDialWidget *m_speedDials;

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

    void addFixtureTab(Fixture* fixture, quint32 channel = QLCChannel::invalid());
    void removeFixtureTab(Fixture* fixture);
    FixtureConsole* fixtureConsoleTab(int tab);
    void setTabChannelState(bool status, Fixture* fixture, quint32 channel);

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

/** @} */

#endif
