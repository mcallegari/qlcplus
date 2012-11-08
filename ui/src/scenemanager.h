/*
  Q Light Controller
  scenemanager.h

  Copyright (C) Massimo Callegari

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

#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QGraphicsView>
#include <QWidget>
#include <QList>

#include "multitrackview.h"
#include "sceneitems.h"
#include "scene.h"
#include "doc.h"

class SceneRunner;
class QComboBox;
class QSplitter;
class QToolBar;
class QAction;
class QLabel;
class Doc;

class SceneManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneManager)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    SceneManager(QWidget* parent, Doc* doc);
    ~SceneManager();
    
    /** Get the singleton instance */
    static SceneManager* instance();
    
signals:
    /** Emitted when the FunctionManager's tab is de/activated */
    void functionManagerActive(bool active);


protected:
    /** @reimp */
    void showEvent(QShowEvent* ev);

    /** @reimp */
    void hideEvent(QHideEvent* ev);

protected:
    static SceneManager* s_instance;
    
    Doc* m_doc;
    /* Currently selected scene */
    Scene* m_scene;
    QWidget* m_scene_editor;
    QWidget* m_sequence_editor;

private:
    QSplitter* m_splitter; // main view splitter (horizontal)
    QSplitter* m_vsplitter; // multitrack view splitter (vertical)
    MultiTrackView *m_showview;

    /*********************************************************************
     * Playback
     *********************************************************************/
    SceneRunner *m_runner;
    bool is_playing;

    /*********************************************************************
     * Menus, toolbar & actions
     *********************************************************************/
protected:
    void initActions();
    void initToolbar();
    void updateScenesCombo();
    void updateMultiTrackView();

    QToolBar* m_toolbar;
    QComboBox* m_scenesCombo;
    QLabel* m_timeLabel;
    QAction* m_addSceneAction;
    QAction* m_addSequenceAction;
    QAction* m_cloneAction;
    QAction* m_deleteAction;
    QAction* m_stopAction;
    QAction* m_playAction;

protected slots:
    void slotSceneComboChanged(int idx);
    void slotAddScene();
    void slotAddSequence();

    void slotClone();
    void slotDelete();
    
    void slotStopPlayback();
    void slotStartPlayback();

    void slotViewClicked(QMouseEvent *event);
    void slotSequenceMoved(SequenceItem *);
    void slotUpdateTime(quint32 msec_time);
    void slotupdateTimeAndCursor(quint32 msec_time);
};

#endif
