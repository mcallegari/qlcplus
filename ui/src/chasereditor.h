/*
  Q Light Controller
  chasereditor.h

  Copyright (c) Heikki Junnila

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

#ifndef CHASEREDITOR_H
#define CHASEREDITOR_H

#include <QPointer>
#include <QWidget>
#include "ui_chasereditor.h"
#include "scene.h"
#include "doc.h"

class Chaser;
class Function;
class InputMap;
class OutputMap;
class ChaserStep;
class MasterTimer;
class SpeedDialWidget;
class QTreeWidgetItem;

class ChaserEditor : public QWidget, public Ui_ChaserEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(ChaserEditor)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    ChaserEditor(QWidget* parent, Chaser* chaser, Doc* doc);
    ~ChaserEditor();

signals:
    void applyValues(QList<SceneValue>&);
    void stepSelectionChanged(int index);

public slots:
    /** Listens to functionManagerActive() so that speed dial box can be hidden/shown */
    void slotFunctionManagerActive(bool active);

    /** Listens to fixture values changes to be applied to the selected step */
    void slotUpdateCurrentStep(SceneValue sv);

private slots:
    void slotNameEdited(const QString& text);

private:
    Doc* m_doc;
    Chaser* m_chaser; // The Chaser being edited

    /************************************************************************
     * List manipulation
     ************************************************************************/
private slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotRaiseClicked();
    void slotLowerClicked();
    void slotSpeedDialToggle(bool state);
    void slotItemSelectionChanged();
    void slotItemChanged(QTreeWidgetItem*,int);

    /************************************************************************
     * Clipboard
     ************************************************************************/
private slots:
    void slotCutClicked();
    void slotCopyClicked();
    void slotPasteClicked();

private:
    //QList <ChaserStep> m_clipboard;
    QAction* m_cutAction;
    QAction* m_copyAction;
    QAction* m_pasteAction;

    /************************************************************************
     * Run order & Direction
     ************************************************************************/
private slots:
    void slotLoopClicked();
    void slotSingleShotClicked();
    void slotPingPongClicked();
    void slotForwardClicked();
    void slotBackwardClicked();

    /************************************************************************
     * Speed
     ************************************************************************/
private slots:
    void slotFadeInDialChanged(int ms);
    void slotFadeOutDialChanged(int ms);
    void slotHoldDialChanged(int ms);

    void slotFadeInToggled();
    void slotFadeOutToggled();
    void slotDurationToggled();

private:
    void createSpeedDials();
    void updateSpeedDials();

private:
    QPointer<SpeedDialWidget> m_speedDials;

    /************************************************************************
     * Test
     ************************************************************************/
private slots:
    void slotRestartTest();
    void slotTestPlay();
    void slotTestStop();
    void slotTestPreviousClicked();
    void slotTestNextClicked();
    void slotModeChanged(Doc::Mode mode);
    void slotStepChanged(int stepNumber);

private:
    bool interruptRunning();
    void continueRunning(bool running);
    int getCurrentIndex();

    /************************************************************************
     * Utilities
     ************************************************************************/
private:
    /** Update the contents of the whole tree, clearing it first if $clear == true)*/
    void updateTree(bool clear = false);

    /** Get the step at the given $item */
    ChaserStep stepAtItem(const QTreeWidgetItem* item) const;

    /** Get the step at the given $index */
    ChaserStep stepAtIndex(int index) const;

    /** Update $item contents from $step */
    void updateItem(QTreeWidgetItem* item, ChaserStep& step);

    /** Update the step numbers (col 0) for each list item */
    void updateStepNumbers();

    /** Update the contents of m_chaser */
    //void updateChaserContents();

    /** Set cut,copy,paste buttons enabled/disabled */
    void updateClipboardButtons();

    /** Apply the step DMX values (if available) immediately */
    void applyStepValues();

    /** helper routine to display the current steps of a chaser */
    void printSteps();

private:
    bool m_itemIsUpdating;
};

#endif
