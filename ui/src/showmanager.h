/*
  Q Light Controller
  showmanager.h

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

#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include <QGraphicsView>
#include <QWidget>
#include <QList>

#include "multitrackview.h"
#include "sceneitems.h"
#include "scene.h"
#include "show.h"
#include "doc.h"

class QComboBox;
class QSplitter;
class QToolBar;
class QAction;
class QLabel;
class Doc;

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
    
signals:
    /** Emitted when the FunctionManager's tab is de/activated */
    void functionManagerActive(bool active);

protected:
    /** @reimp */
    void showEvent(QShowEvent* ev);

    /** @reimp */
    void hideEvent(QHideEvent* ev);

protected:
    static ShowManager* s_instance;
    
    Doc* m_doc;
    /* Currently selected show */
    Show* m_show;
    /* Currently selected scene */
    Scene* m_scene;
    QWidget* m_scene_editor;
    QWidget* m_sequence_editor;

private:
    void showSceneEditor(Scene *scene);
    void showSequenceEditor(Chaser *chaser);

private:
    QSplitter* m_splitter; // main view splitter (horizontal)
    QSplitter* m_vsplitter; // multitrack view splitter (vertical)
    MultiTrackView *m_showview;

    /*********************************************************************
     * Menus, toolbar & actions
     *********************************************************************/
protected:
    void initActions();
    void initToolbar();
    void updateShowsCombo();
    void updateMultiTrackView();

    QToolBar* m_toolbar;
    QComboBox* m_showsCombo;
    QLabel* m_timeLabel;
    QAction* m_addShowAction;
    QAction* m_addTrackAction;
    QAction* m_addSequenceAction;
    QAction* m_addAudioAction;
    QAction* m_cloneAction;
    QAction* m_deleteAction;
    QAction* m_colorAction;
    QAction* m_stopAction;
    QAction* m_playAction;

protected slots:
    void slotShowsComboChanged(int idx);
    void slotAddShow();
    void slotAddTrack();
    void slotAddSequence();
    void slotAddAudio();

    void slotClone();
    void slotDelete();

    /*********************************************************************
     * Playback
     *********************************************************************/
    void slotStopPlayback();
    void slotStartPlayback();

    /*********************************************************************
     * UI events
     *********************************************************************/
    void slotViewClicked(QMouseEvent *event);
    void slotSequenceMoved(SequenceItem *);
    void slotAudioMoved(AudioItem *);
    void slotUpdateTime(quint32 msec_time);
    void slotupdateTimeAndCursor(quint32 msec_time);
    void slotTrackClicked(Track *track);
    void slotChangeColor();
    void slotChangeSize(int width, int height);

    /*********************************************************************
     * DOC events
     *********************************************************************/
    void slotDocClearing();
    void slotDocLoaded();
    void slotFunctionChanged(quint32 id);
    void slotFunctionRemoved(quint32 id);
};

#endif
