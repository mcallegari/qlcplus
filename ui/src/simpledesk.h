/*
  Q Light Controller Plus
  simpledesk.h

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

#ifndef SIMPLEDESK_H
#define SIMPLEDESK_H

#include <QScrollArea>
#include <QModelIndex>
#include <QWidget>
#include <QList>
#include <QHash>

class GrandMasterSlider;
class SimpleDeskEngine;
class QXmlStreamReader;
class QXmlStreamWriter;
class SpeedDialWidget;
class PlaybackSlider;
class ConsoleChannel;
class FixtureConsole;
class QToolButton;
class SimpleDesk;
class QTabWidget;
class QComboBox;
class QGroupBox;
class QTreeView;
class QSplitter;
class QSpinBox;
class CueStack;
class Doc;
class Cue;

/** @addtogroup ui_simpledesk
 * @{
 */

#define KXMLQLCSimpleDesk QString("SimpleDesk")

class SimpleDesk : public QWidget
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    SimpleDesk(QWidget* parent, Doc* doc);
    ~SimpleDesk();

    /** Get the singleton instance */
    static SimpleDesk* instance();

    /** Start from scratch; clear everything */
    void clearContents();

private:
    /** Private constructor to prevent multiple instances. */

    /** Initialize the simple desk engine */
    void initEngine();

    /** Initialize the simple desk view components */
    void initView();
    void initTopSide();
    void initBottomSide();

protected:
    /** @reimp */
    void showEvent(QShowEvent* ev);

    /** @reimp */
    void hideEvent(QHideEvent* ev);

    /** @reimp */
    void resizeEvent(QResizeEvent *ev);

protected slots:
    void slotDocChanged();

private:
    static SimpleDesk *s_instance;
    SimpleDeskEngine *m_engine;
    QSplitter *m_splitter;
    Doc *m_doc;
    bool m_docChanged;

    /*********************************************************************
     * Universe controls
     *********************************************************************/
public:
    int getSlidersNumber();
    int getCurrentUniverseIndex();
    int getCurrentPage();
    uchar getAbsoluteChannelValue(uint address);
    void setAbsoluteChannelValue(uint address, uchar value);
    void resetChannel(quint32 address);
    void resetUniverse();
    void resetUniverse(int index);

private:
    void initUniversesCombo();
    void initUniverseSliders();
    void initUniversePager();
    void resetUniverseSliders();
    void initSliderView(bool fullMode);
    void initChannelGroupsView();

private slots:
    void slotUniversesComboChanged(int index);
    void slotViewModeClicked(bool toggle);
    void slotUniversePageUpClicked();
    void slotUniversePageDownClicked();
    void slotUniversePageChanged(int page);
    void slotUniverseResetClicked();
    void slotChannelResetClicked(quint32 fxID, quint32 channel);
    void slotAliasChanged();
    void slotUniverseSliderValueChanged(quint32, quint32, uchar value);
    void slotUpdateUniverseSliders();
    void slotUniverseWritten(quint32 idx, const QByteArray& universeData);

private:
    QFrame *m_universeGroup;
    QComboBox *m_universesCombo;
    QToolButton *m_viewModeButton;
    QToolButton *m_universePageUpButton;
    QSpinBox *m_universePageSpin;
    QToolButton *m_universePageDownButton;
    QToolButton *m_universeResetButton;
    GrandMasterSlider *m_grandMasterSlider;
    QScrollArea *scrollArea;
    QScrollArea *m_chGroupsArea;

    /**
     * List holding pointers to the current view sliders.
     * Their number is always equal to m_channelsPerPage
     */
    QList <ConsoleChannel*> m_universeSliders;

    /**
     * Map of the Fixture ID/FixtureConsole representing
     * each fixture in the selected universe
     */
    QHash <quint32, FixtureConsole *> m_consoleList;

    /** Currently selected universe. Basically the index of m_universesCombo */
    int m_currentUniverse;

    /** Define how many sliders will be displayed for each page */
    uint m_channelsPerPage;

    /** A list to remember the selected page of each universe */
    QList<int> m_universesPage;

    /*********************************************************************
     * Playback sliders
     *********************************************************************/
private:
    void initPlaybackSliders();
    void resetPlaybackSliders();

private slots:
    void slotPlaybackSelected();
    void slotSelectPlayback(uint pb);
    void slotPlaybackStarted();
    void slotPlaybackStopped();
    void slotPlaybackFlashing(bool enabled);
    void slotPlaybackValueChanged(uchar value);

    /** Called when the user moves a fader of the ChannelGroup console */
    void slotGroupValueChanged(quint32 groupID, uchar value);

private:
    QTabWidget *m_tabs;
    QGroupBox *m_playbackGroup;
    QList <PlaybackSlider*> m_playbackSliders;
    uint m_selectedPlayback;
    uint m_playbacksPerPage;

    /*********************************************************************
     * Cue Stack controls
     *********************************************************************/
private:
    void initCueStack();
    void updateCueStackButtons();
    void replaceCurrentCue();
    void updateSpeedDials();
    void createSpeedDials();

    CueStack *currentCueStack() const;
    int currentCueIndex() const;

private slots:
    void slotCueStackStarted(uint stack);
    void slotCueStackStopped(uint stack);
    void slotCueStackSelectionChanged();

    void slotPreviousCueClicked();
    void slotNextCueClicked();
    void slotStopCueStackClicked();
    void slotCloneCueStackClicked();
    void slotEditCueStackClicked(bool state);
    void slotRecordCueClicked();
    void slotDeleteCueClicked();

    void slotFadeInDialChanged(int ms);
    void slotFadeOutDialChanged(int ms);
    void slotHoldDialChanged(int ms);
    void slotDialDestroyed(QObject *);
    void slotCueNameEdited(const QString& name);

private:
    QGroupBox *m_cueStackGroup;
    QToolButton *m_previousCueButton;
    QToolButton *m_nextCueButton;
    QToolButton *m_stopCueStackButton;
    QToolButton *m_cloneCueStackButton;
    QToolButton *m_editCueStackButton;
    QToolButton *m_recordCueButton;
    QTreeView *m_cueStackView;
    SpeedDialWidget *m_speedDials;
    QModelIndex m_cueDeleteIconIndex;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;
};

/** @} */

#endif
