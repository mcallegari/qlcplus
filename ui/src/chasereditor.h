/*
  Q Light Controller
  chasereditor.h

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

#ifndef CHASEREDITOR_H
#define CHASEREDITOR_H

#include <QWidget>
#include "ui_chasereditor.h"
#include "scene.h"
#include "doc.h"

class Chaser;
class Function;
class ChaserStep;
class MasterTimer;
class SpeedDialWidget;
class QTreeWidgetItem;

/** @addtogroup ui_functions
 * @{
 */

class ChaserEditor : public QWidget, public Ui_ChaserEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(ChaserEditor)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    ChaserEditor(QWidget* parent, Chaser* chaser, Doc* doc, bool liveMode = false);
    ~ChaserEditor();

    /** Set the visible state of the Order and
     *  Direction group boxes */
    void showOrderAndDirection(bool show);

    void stopTest();

    /** Select the step at the given time
     *  and scroll the view to it */
    void selectStepAtTime(quint32 time);

private:
    FunctionParent functionParent() const;

signals:
    void applyValues(QList<SceneValue>&);
    void stepSelectionChanged(int index);

public slots:
    /** Listens to functionManagerActive() so that speed dial box can be hidden/shown */
    void slotFunctionManagerActive(bool active);

    /** Listens to fixture values changes to be applied to the selected step */
    void slotUpdateCurrentStep(SceneValue sv, bool enabled);

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
    void slotShuffleClicked();
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
    void slotRandomClicked();
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

    void slotDialDestroyed(QObject* dial);

private:
    void createSpeedDials();
    void updateSpeedDials();

private:
    SpeedDialWidget *m_speedDials;

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

    /** Set cut,copy,paste buttons enabled/disabled */
    void updateClipboardButtons();

    /** Apply the step DMX values (if available) immediately */
    void applyStepValues();

    /** helper routine to display the current steps of a chaser */
    void printSteps();

private:
    bool m_liveMode;
};

/** @} */

#endif
