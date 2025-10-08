/*
  Q Light Controller
  showmanager.h

  Copyright (C) Massimo Callegari

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

#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include <QGraphicsView>
#include <QWidget>
#include <QList>

#include "multitrackview.h"
#include "sequenceitem.h"
#include "trackitem.h"
#include "scene.h"
#include "show.h"
#include "doc.h"

class QComboBox;
class QCheckBox;
class QSplitter;
class QToolBar;
class QSpinBox;
class QAction;
class QLabel;
class Doc;

/** @addtogroup ui_shows
 * @{
 */

class ShowManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ShowManager)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ShowManager(QWidget* parent, Doc* doc);
    ~ShowManager();

    /** Get the singleton instance */
    static ShowManager* instance();

    /** Start from scratch; clear everything */
    void clearContents();

signals:
    /** Emitted when the FunctionManager's tab is de/activated */
    void functionManagerActive(bool active);

protected:
    /** @reimp */
    void showEvent(QShowEvent* ev);

    /** @reimp */
    void hideEvent(QHideEvent* ev);

protected:
    static ShowManager *s_instance;

    Doc *m_doc;
    /** Currently selected show */
    Show *m_show;
    /** Currently selected track */
    Track *m_currentTrack;
    /** Currently selected scene */
    Scene *m_currentScene;
    /** Scene editor instance reference */
    QWidget *m_sceneEditor;
    /** Right editor instance reference (can edit Chaser, Audio, Video) */
    QWidget *m_currentEditor;
    /** ID of the Function currently edited on the right */
    quint32 m_editorFunctionID;

    /** Index of the currently selected Show
     * (basically the m_showsCombo index) */
    int m_selectedShowIndex;

    /** Track if cursor is interactively being moved during pause */
    bool cursorMovedDuringPause;

private:
    void showSceneEditor(Scene *scene);
    void hideRightEditor();
    void showRightEditor(Function *function);

private:
    QSplitter *m_splitter; // main view splitter (horizontal)
    QSplitter *m_vsplitter; // multitrack view splitter (vertical)
    MultiTrackView *m_showview;

    /*********************************************************************
     * Menus, toolbar & actions
     *********************************************************************/
protected:
    void initActions();
    void initToolbar();
    void updateShowsCombo();
    void updateMultiTrackView();

private:
    bool checkOverlapping(quint32 startTime, quint32 duration);

    QToolBar *m_toolbar;
    QComboBox *m_showsCombo;
    QLabel *m_timeLabel;
    QAction *m_addShowAction;
    QAction *m_addTrackAction;
    QAction *m_addSequenceAction;
    QAction *m_addAudioAction;
    QAction *m_addVideoAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QAction *m_deleteAction;
    QAction *m_colorAction;
    QAction *m_lockAction;
    QAction *m_timingsAction;
    QAction *m_snapGridAction;
    QAction *m_stopAction;
    QAction *m_playAction;
    QComboBox *m_timeDivisionCombo;
    QSpinBox *m_bpmField;

protected slots:
    /** Slot called when the user selects a show from
     *  the shows combo box */
    void slotShowsComboChanged(int idx);

    /** Slot called when the user request to add a new show */
    void slotAddShow();

    void slotAddItem();
    void slotAddSequence();
    void slotAddAudio();
    void slotAddVideo();

    void slotCopy();
    void slotPaste();
    void slotDelete();

    /*********************************************************************
     * Playback
     *********************************************************************/
protected slots:
    void slotStopPlayback();
    void slotStartPlayback();
    void slotShowStopped();

    /*********************************************************************
     * Time division
     *********************************************************************/
protected slots:
    void slotTimeDivisionTypeChanged(int idx);
    void slotBPMValueChanged(int value);

    /*********************************************************************
     * UI events
     *********************************************************************/
protected slots:
    void slotViewClicked(QMouseEvent *event);
    void slotShowItemMoved(ShowItem *item, quint32 time, bool moved);

    void slotUpdateTime(quint32 msec_time);
    void slotUpdateTimeAndCursor(quint32 msec_time);
    void slotTrackClicked(Track *track);
    void slotTrackDoubleClicked(Track *track);
    void slotTrackMoved(Track *track, int direction);
    void slotTrackDelete(Track *track);
    void slotChangeColor();
    void slotChangeLock();
    void slotShowTimingsTool();
    void slotShowItemStartTimeChanged(ShowItem *item, int msec);
    void slotShowItemDurationChanged(ShowItem *item, int msec, bool stretch);
    void slotToggleSnapToGrid(bool enable);
    void slotChangeSize(int width, int height);
    void slotStepSelectionChanged(int index);

    /*********************************************************************
     * Doc events
     *********************************************************************/
protected slots:
    void slotDocClearing();
    void slotDocLoaded();
    void slotFunctionRemoved(quint32 id);

private:
    FunctionParent functionParent() const;
};

/** @} */

#endif
