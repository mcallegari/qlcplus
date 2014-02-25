/*
  Q Light Controller
  simpledesk.h

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

#ifndef SIMPLEDESK_H
#define SIMPLEDESK_H

#include <QModelIndex>
#include <QPointer>
#include <QWidget>
#include <QList>
#include <QHash>
#include <QScrollArea>

class GrandMasterSlider;
class SimpleDeskEngine;
class SpeedDialWidget;
class PlaybackSlider;
class ConsoleChannel;
class FixtureConsole;
class QDomDocument;
class QDomElement;
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

#define KXMLQLCSimpleDesk "SimpleDesk"

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

protected slots:
    void slotDocChanged();

private:
    static SimpleDesk* s_instance;
    SimpleDeskEngine* m_engine;
    QSplitter* m_splitter;
    Doc* m_doc;
    bool m_docChanged;

    /*********************************************************************
     * Universe controls
     *********************************************************************/
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
    void slotUniverseSliderValueChanged(quint32, quint32, uchar value);
    void slotUpdateUniverseSliders();
    void slotUniversesWritten(int idx, const QByteArray& ua);

private:
    QGroupBox* m_universeGroup;
    QComboBox* m_universesCombo;
    QToolButton* m_viewModeButton;
    QToolButton* m_universePageUpButton;
    QSpinBox* m_universePageSpin;
    QToolButton* m_universePageDownButton;
    QToolButton* m_universeResetButton;
    GrandMasterSlider* m_grandMasterSlider;
    QScrollArea* scrollArea;
    QScrollArea* m_chGroupsArea;

    QList <ConsoleChannel*> m_universeSliders;
    QList <FixtureConsole *> m_consoleList;

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
    QTabWidget* m_tabs;
    QGroupBox* m_playbackGroup;
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

    CueStack* currentCueStack() const;
    int currentCueIndex() const;

private slots:
    void slotCueStackStarted(uint stack);
    void slotCueStackStopped(uint stack);
    void slotCueStackSelectionChanged();

    void slotPreviousCueClicked();
    void slotNextCueClicked();
    void slotStopCueStackClicked();
    void slotCloneCueStackClicked();
    void slotEditCueStackClicked();
    void slotRecordCueClicked();
    void slotDeleteCueClicked();

    void slotFadeInDialChanged(int ms);
    void slotFadeOutDialChanged(int ms);
    void slotHoldDialChanged(int ms);
    void slotCueNameEdited(const QString& name);

protected:
    /** @reimp */
    void showEvent(QShowEvent* ev);

    /** @reimp */
    void hideEvent(QHideEvent* ev);

private:
    QGroupBox* m_cueStackGroup;
    QToolButton* m_previousCueButton;
    QToolButton* m_nextCueButton;
    QToolButton* m_stopCueStackButton;
    QToolButton* m_cloneCueStackButton;
    QToolButton* m_editCueStackButton;
    QToolButton* m_recordCueButton;
    QTreeView* m_cueStackView;
    QPointer<SpeedDialWidget> m_speedDials;
    QModelIndex m_cueDeleteIconIndex;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(const QDomElement& root);
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root) const;
};

/** @} */

#endif
