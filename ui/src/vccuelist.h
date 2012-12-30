/*
  Q Light Controller
  vccuelist.h

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
class QPushButton;

class VCCueListProperties;
class ChaserRunner;
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
#define KXMLQLCVCCueListStop "Stop"

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
    static const quint8 stopInputSourceId;

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
    bool copyFrom(VCWidget* widget);

    /*************************************************************************
     * Cue list
     *************************************************************************/
public:
    /** Set the chaser function that is used as cue list steps */
    void setChaser(quint32 fid);

    /** Get the chaser function that is used as cue list steps */
    quint32 chaser() const;

private:
    /** Update the list of steps */
    void updateStepList();

private slots:
    /** Removes destroyed functions from the list */
    void slotFunctionRemoved(quint32 fid);

    /** Updates name in the list if function got changed */
    void slotFunctionChanged(quint32 fid);

    /** Skip to the next cue */
    void slotNextCue();

    /** Skip to the previous cue */
    void slotPreviousCue();

    /** Stop the cue list and return to start */
    void slotStop();

    /** Called when m_runner skips to another step */
    void slotCurrentStepChanged(int stepNumber);

    /** Slot that is called whenever the current item changes (either by
        pressing the key binding or clicking an item with mouse) */
    void slotItemActivated(QTreeWidgetItem* item);

    /** Record current dmx output into a new Scene and append it to the cuelist.
        If an the cuelist is running the active Scene in the cuelist will be updated with
        the current dmx output **/
    void slotRecord();

    /** Refresh Cuelist View, if name is changed Sceneeditor **/
    void slotRefresh();

private:
    /** Create the runner that writes cue values to universes */
    void createRunner(int startIndex = -1);

private:
    quint32 m_chaser;
    QTreeWidget* m_tree;
    QPushButton* m_stopButton;
    QPushButton* m_recordButton;
    QPushButton* m_refreshButton;

    ChaserRunner* m_runner;
    QMutex m_mutex; // Guards m_runner

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
    void setStopKeySequence(const QKeySequence& keySequence);

    /** Get the keyboard key combination for stopping the cue list */
    QKeySequence stopKeySequence() const;

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);

private:
    QKeySequence m_nextKeySequence;
    QKeySequence m_previousKeySequence;
    QKeySequence m_stopKeySequence;

    /*************************************************************************
     * External Input
     *************************************************************************/
protected slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

private:
    quint32 m_nextLatestValue;
    quint32 m_previousLatestValue;
    quint32 m_stopLatestValue;

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
