/*
  Q Light Controller Plus
  vccuelist.h

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#ifndef VCCUELIST_H
#define VCCUELIST_H

#include <QKeySequence>
#include <QWidget>

#include "dmxsource.h"
#include "vcwidget.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QTreeWidgetItem;
class QProgressBar;
class QTreeWidget;
class QToolButton;
class QCheckBox;
class QLabel;

class VCCueListProperties;
class ClickAndGoSlider;
class ChaserRunner;
class MasterTimer;
class Chaser;
class Doc;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCCueList                QString("CueList")
#define KXMLQLCVCCueListFunction        QString("Function") // Legacy
#define KXMLQLCVCCueListChaser          QString("Chaser")
#define KXMLQLCVCCueListPlaybackLayout  QString("PlaybackLayout")
#define KXMLQLCVCCueListNextPrevBehavior QString("NextPrevBehavior")
#define KXMLQLCVCCueListCrossfade       QString("Crossfade")
#define KXMLQLCVCCueListBlend           QString("Blend")
#define KXMLQLCVCCueListLinked          QString("Linked")
#define KXMLQLCVCCueListNext            QString("Next")
#define KXMLQLCVCCueListPrevious        QString("Previous")
#define KXMLQLCVCCueListPlayback        QString("Playback")
#define KXMLQLCVCCueListStop            QString("Stop")
#define KXMLQLCVCCueListCrossfadeLeft   QString("CrossLeft")
#define KXMLQLCVCCueListCrossfadeRight  QString("CrossRight")
#define KXMLQLCVCCueListSlidersMode     QString("SlidersMode")

/**
 * VCCueList provides a \ref VirtualConsole widget to control cue lists.
 *
 * @see VCWidget
 * @see VirtualConsole
 */
class VCCueList : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCCueList)

    friend class VCCueListProperties;

public:
    static const quint8 nextInputSourceId;
    static const quint8 previousInputSourceId;
    static const quint8 playbackInputSourceId;
    static const quint8 stopInputSourceId;
    static const quint8 sideFaderInputSourceId;

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /** Constructor */
    VCCueList(QWidget *parent, Doc *doc);

    /** Destructor */
    ~VCCueList();

    /** @reimp */
    void enableWidgetUI(bool enable);

    /*************************************************************************
     * Clipboard
     *************************************************************************/
public:
    /** Create a copy of this widget into the given parent */
    VCWidget *createCopy(VCWidget *parent);

protected:
    /** Copy the contents for this widget from another widget */
    bool copyFrom(const VCWidget *widget);

    /*************************************************************************
     * Cue list
     *************************************************************************/
public:
    /** Set the chaser function that is used as cue list steps */
    void setChaser(quint32 fid);

    /** Get the chaser ID that is used as cue list steps */
    quint32 chaserID() const;

    /** Get the chaser function that is used as cue list steps */
    Chaser *chaser();

public:
    /** Get the currently selected item index, otherwise 0 */
    int getCurrentIndex();
    /** Get the progress text of the selected item */
    QString progressText();
    double progressPercent();

private:
    /** Get the index of the next item, based on the chaser direction */
    int getNextIndex();

    /** Get the index of the previous item, based on the chaser direction */
    int getPrevIndex();

    /** Get the index of the first item, based on the chaser direction */
    int getFirstIndex();

    /** Get the index of the last item, based on the chaser direction */
    int getLastIndex();

    /** Get the index of the item above the selected item */
    int getNextTreeIndex();

    /** Get the index of the item below the selected item */
    int getPrevTreeIndex();

    /** Get the index of the item on top of the tree */
    int getFirstTreeIndex();

    /** Get the index of the item at the bottom of the tree */
    int getLastTreeIndex();

private:
    /** Get the intensity of the current primary slider */
    qreal getPrimaryIntensity() const;

public:
    /** @reimp */
    virtual void notifyFunctionStarting(quint32 fid, qreal intensity);

private:
    /** Update the list of steps */
    void updateStepList();

    /** timer for updating the step list */
    QTimer *m_updateTimer;

public slots:
    /** Play/stop/resume the cue list from the current selection */
    void slotPlayback();

    /** Stop the cue list */
    void slotStop();

    /** Skip to the next cue */
    void slotNextCue();

    /** Skip to the previous cue */
    void slotPreviousCue();

    /** Called when m_runner skips to another step */
    void slotCurrentStepChanged(int stepNumber);

    /** Update cue step note */
    void slotStepNoteChanged(int idx, QString note);

signals:
    /** progress percent value and text */
    void progressStateChanged();

private slots:
    /** Removes destroyed functions from the list */
    void slotFunctionRemoved(quint32 fid);

    /** Updates the list if chaser got changed */
    void slotFunctionChanged(quint32 fid);

    /** Updates name in the list if function got changed */
    void slotFunctionNameChanged(quint32 fid);

    /** Update the step list at m_updateTimer timeout */
    void slotUpdateStepList();

    /** Slot that is called whenever the current item changes (either by
        pressing the key binding or clicking an item with mouse) */
    void slotItemActivated(QTreeWidgetItem *item);

    /** Slot that is called whenever an item field has been changed.
        Note that only 'Notes" column is considered */
    void slotItemChanged(QTreeWidgetItem*item, int column);

    /** Slot called whenever a function is started */
    void slotFunctionRunning(quint32 fid);

    /** Slot called whenever a function is stopped */
    void slotFunctionStopped(quint32 fid);

    /** Slot called every 200ms to update the step progress bar */
    void slotProgressTimeout();

private:
    /** Start associated chaser */
    void startChaser(int startIndex = -1);

    /** Stop associated */
    void stopChaser();

    int getFadeMode();

public:
    enum NextPrevBehavior
    {
        DefaultRunFirst = 0,
        RunNext,
        Select,
        Nothing
    };

    enum PlaybackLayout
    {
        PlayPauseStop = 0,
        PlayStopPause
    };

    void setNextPrevBehavior(NextPrevBehavior nextPrev);
    NextPrevBehavior nextPrevBehavior() const;

    void setPlaybackLayout(PlaybackLayout layout);
    PlaybackLayout playbackLayout() const;
signals:
    void playbackButtonClicked();
    void stopButtonClicked();
    void playbackStatusChanged();

private:
    /** ID of the Chaser this Cue List will be controlling */
    quint32 m_chaserID;

    NextPrevBehavior m_nextPrevBehavior;
    PlaybackLayout m_playbackLayout;
    QTreeWidget *m_tree;
    QToolButton *m_crossfadeButton;
    QToolButton *m_playbackButton;
    QToolButton *m_stopButton;
    QToolButton *m_previousButton;
    QToolButton *m_nextButton;
    QProgressBar *m_progress;
    bool m_listIsUpdating;

    QTimer *m_timer;

    /*************************************************************************
     * Crossfade
     *************************************************************************/
public:
    enum FaderMode
    {
        None = 0,
        Crossfade,
        Steps
    };

    FaderMode sideFaderMode() const;
    void setSideFaderMode(FaderMode mode);

    FaderMode stringToFaderMode(QString modeStr);
    QString faderModeToString(FaderMode mode);
    bool isSideFaderVisible();
    bool sideFaderButtonIsChecked();
    QString topPercentageValue();
    QString bottomPercentageValue();
    QString topStepValue();
    QString bottomStepValue();
    int sideFaderValue();
    bool primaryTop();

signals:
    void sideFaderButtonToggled();
    void sideFaderValueChanged();
    void sideFaderButtonChecked();

public slots:
    void slotSideFaderButtonChecked(bool enable);
    void slotSetSideFaderValue(int value);

protected:
    void setFaderInfo(int index);

protected slots:
    void slotShowCrossfadePanel(bool enable);
    void slotSideFaderValueChanged(int value);

protected:
    void stopStepIfNeeded(Chaser *ch);

private:
    QLabel *m_topPercentageLabel;
    QLabel *m_topStepLabel;
    ClickAndGoSlider *m_sideFader;
    QLabel *m_bottomPercentageLabel;
    QLabel *m_bottomStepLabel;

    QBrush m_defCol;
    int m_primaryIndex, m_secondaryIndex;
    bool m_primaryTop;
    FaderMode m_slidersMode;

    /*************************************************************************
     * Key sequences
     *************************************************************************/
public:
    /** Set the keyboard key combination for skipping to the next cue */
    void setNextKeySequence(const QKeySequence& keySequence);

    /** Get the keyboard key combination for skipping to the next cue */
    QKeySequence nextKeySequence() const;

    /** Set the keyboard key combination for skipping to the previous cue */
    void setPreviousKeySequence(const QKeySequence& keySequence);

    /** Get the keyboard key combination for skipping to the previous cue */
    QKeySequence previousKeySequence() const;

    /** Set the keyboard key combination for playing/resuming the cue list */
    void setPlaybackKeySequence(const QKeySequence& keySequence);

    /** Get the keyboard key combination for playing/resuming the cue list */
    QKeySequence playbackKeySequence() const;

    /** Set the keyboard key combination for stopping the cue list */
    void setStopKeySequence(const QKeySequence& keySequence);

    /** Get the keyboard key combination for stopping the cue list */
    QKeySequence stopKeySequence() const;

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);

private:
    QKeySequence m_nextKeySequence;
    QKeySequence m_previousKeySequence;
    QKeySequence m_playbackKeySequence;
    QKeySequence m_stopKeySequence;

    /*************************************************************************
     * External Input
     *************************************************************************/
public:
    void updateFeedback();

protected slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

private:
    quint32 m_nextLatestValue;
    quint32 m_previousLatestValue;
    quint32 m_playbackLatestValue;
    quint32 m_stopLatestValue;

    /*************************************************************************
     * VCWidget-inherited
     *************************************************************************/
public:
    /** @reimp */
    void adjustIntensity(qreal val);

    /** @reimp */
    void setCaption(const QString& text);

    /** @reimp */
    void setFont(const QFont& font);

    /** @reimp */
    void slotModeChanged(Doc::Mode mode);

    /** @reimp */
    void editProperties();

    /*********************************************************************
     * Web access
     *********************************************************************/
public:
    /** Play a specific cue at the given index */
    void playCueAtIndex(int idx);

signals:
    /** Signal to webaccess */
    void stepChanged(int idx);

    void stepNoteChanged(int idx, QString note);

private:
    FunctionParent functionParent() const;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);
};

/** @} */

#endif
