/*
  Q Light Controller
  vccuelist.h

  Copyright (c) Heikki Junnila, Massimo Callegari

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

class QTreeWidgetItem;
class QDomDocument;
class QDomElement;
class QTreeWidget;
class QToolButton;
class QCheckBox;
class QLabel;

class VCCueListProperties;
class ClickAndGoSlider;
class CueListRunner;
class MasterTimer;
class InputMap;
class Chaser;
class Doc;

#define KXMLQLCVCCueList "CueList"
#define KXMLQLCVCCueListFunction "Function" // Legacy
#define KXMLQLCVCCueListChaser "Chaser"
#define KXMLQLCVCCueListKey "Key"
#define KXMLQLCVCCueListNext "Next"
#define KXMLQLCVCCueListPrevious "Previous"
#define KXMLQLCVCCueListPlayback "Playback"
#define KXMLQLCVCCueListCrossfadeLeft "CrossLeft"
#define KXMLQLCVCCueListCrossfadeRight "CrossRight"

/**
 * VCCueList provides a \ref VirtualConsole widget to control cue lists.
 *
 * @see VCWidget
 * @see VirtualConsole
 */
class VCCueList : public VCWidget, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCCueList)

    friend class VCCueListProperties;

public:
    static const quint8 nextInputSourceId;
    static const quint8 previousInputSourceId;
    static const quint8 playbackInputSourceId;
    static const quint8 cf1InputSourceId;
    static const quint8 cf2InputSourceId;

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /** Constructor */
    VCCueList(QWidget* parent, Doc* doc);

    /** Destructor */
    ~VCCueList();

    /*************************************************************************
     * Clipboard
     *************************************************************************/
public:
    /** Create a copy of this widget into the given parent */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** Copy the contents for this widget from another widget */
    bool copyFrom(const VCWidget* widget);

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

private:
    /** Update the list of steps */
    void updateStepList();

    /** Get the currently selected item index, otherwise 0 */
    int getCurrentIndex();

public slots:
    /** Play/stop the cue list from the current selection */
    void slotPlayback();

    /** Skip to the next cue */
    void slotNextCue();

    /** Skip to the previous cue */
    void slotPreviousCue();

private slots:
    /** Removes destroyed functions from the list */
    void slotFunctionRemoved(quint32 fid);

    /** Updates name in the list if function got changed */
    void slotFunctionChanged(quint32 fid);

    /** Stop the cue list and return to start */
    void slotStop();

    /** Called when m_runner skips to another step */
    void slotCurrentStepChanged(int stepNumber);

    /** Slot that is called whenever the current item changes (either by
        pressing the key binding or clicking an item with mouse) */
    void slotItemActivated(QTreeWidgetItem* item);

    /** Slot that is called whenever an item field has been changed.
        Note that only 'Notes" column is considered */
    void slotItemChanged(QTreeWidgetItem*item, int column);

    /** Slot called whenever a function is stopped */
    void slotFunctionStopped(quint32 fid);

private:
    /** Create the runner that writes cue values to universes */
    void createRunner(int startIndex = -1);

private:
    quint32 m_chaserID;
    QTreeWidget* m_tree;
    QToolButton* m_crossfadeButton;
    QToolButton* m_playbackButton;
    QToolButton* m_previousButton;
    QToolButton* m_nextButton;
    bool m_listIsUpdating;

    CueListRunner* m_runner;
    QMutex m_mutex; // Guards m_runner


    /*************************************************************************
     * Crossfade
     *************************************************************************/
protected:
    void setSlidersInfo(int pIndex, Chaser *chaserID);

protected slots:
    void slotShowCrossfadePanel(bool enable);
    void slotSlider1ValueChanged(int value);
    void slotSlider2ValueChanged(int value);

private:
    QCheckBox *m_linkCheck;
    QLabel *m_sl1TopLabel;
    ClickAndGoSlider* m_slider1;
    QLabel *m_sl1BottomLabel;

    QLabel *m_sl2TopLabel;
    ClickAndGoSlider* m_slider2;
    QLabel *m_sl2BottomLabel;

    QBrush m_defCol;
    int m_primaryIndex, m_secondaryIndex;
    QString m_noStyle, m_blueStyle, m_orangeStyle;
    bool m_primaryLeft;


    /*************************************************************************
     * DMX Source
     *************************************************************************/
public:
    /** @reimp */
    void writeDMX(MasterTimer* timer, UniverseArray* universes);

private:
    /** Flag indicating, whether stop button has been pressed */
    bool m_stop;

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

    /** Set the keyboard key combination for stopping the cue list */
    void setPlaybackKeySequence(const QKeySequence& keySequence);

    /** Get the keyboard key combination for stopping the cue list */
    QKeySequence playbackKeySequence() const;

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);

private:
    QKeySequence m_nextKeySequence;
    QKeySequence m_previousKeySequence;
    QKeySequence m_playbackKeySequence;

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

    /*************************************************************************
     * VCWidget-inherited
     *************************************************************************/
public:
    /** @reimp */
    void setCaption(const QString& text);

    /** @reimp */
    void slotModeChanged(Doc::Mode mode);

    /** @reimp */
    void editProperties();

    /*********************************************************************
     * Web access
     *********************************************************************/
public:
    /** @reimpl */
    QString getCSS();

    /** @reimpl */
    QString getJS();

    /** Play a specific cue at the given index */
    void playCueAtIndex(int idx);

signals:
    void stepChanged(int idx);

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** @reimp */
    bool loadXML(const QDomElement* root);

    /** @reimp */
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif
