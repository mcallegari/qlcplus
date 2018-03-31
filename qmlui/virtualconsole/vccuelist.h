/*
  Q Light Controller Plus
  vccuelist.h

  Copyright (c) Massimo Callegari

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

#include "vcwidget.h"

#define KXMLQLCVCCueList "CueList"
#define KXMLQLCVCCueListChaser "Chaser"
#define KXMLQLCVCCueListPlaybackLayout "PlaybackLayout"
#define KXMLQLCVCCueListNextPrevBehavior "NextPrevBehavior"
#define KXMLQLCVCCueListCrossfade "Crossfade"
#define KXMLQLCVCCueListBlend "Blend"
#define KXMLQLCVCCueListLinked "Linked"
#define KXMLQLCVCCueListNext "Next"
#define KXMLQLCVCCueListPrevious "Previous"
#define KXMLQLCVCCueListPlayback "Playback"
#define KXMLQLCVCCueListStop "Stop"
#define KXMLQLCVCCueListCrossfadeLeft "CrossLeft"
#define KXMLQLCVCCueListCrossfadeRight "CrossRight"
#define KXMLQLCVCCueListSlidersMode "SlidersMode"

class ListModel;

class VCCueList : public VCWidget
{
    Q_OBJECT

    Q_PROPERTY(quint32 chaserID READ chaserID WRITE setChaserID NOTIFY chaserIDChanged)
    Q_PROPERTY(QVariant stepsList READ stepsList NOTIFY stepsListChanged)

    Q_PROPERTY(NextPrevBehavior nextPrevBehavior READ nextPrevBehavior WRITE setNextPrevBehavior NOTIFY nextPrevBehaviorChanged)
    Q_PROPERTY(PlaybackLayout playbackLayout READ playbackLayout WRITE setPlaybackLayout NOTIFY playbackLayoutChanged)

    Q_PROPERTY(PlaybackStatus playbackStatus READ playbackStatus NOTIFY playbackStatusChanged)
    Q_PROPERTY(int playbackIndex READ playbackIndex WRITE setPlaybackIndex NOTIFY playbackIndexChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCCueList(Doc* doc = NULL, QObject *parent = 0);
    virtual ~VCCueList();

    /** @reimp */
    QString defaultCaption();

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page);

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /*********************************************************************
     * UI settings
     *********************************************************************/
public:
    enum NextPrevBehavior
    {
        DefaultRunFirst = 0,
        RunNext,
        Select,
        Nothing
    };
    Q_ENUM(NextPrevBehavior)

    enum PlaybackLayout
    {
        PlayPauseStop = 0,
        PlayStopPause
    };
    Q_ENUM(PlaybackLayout)

    NextPrevBehavior nextPrevBehavior() const;
    void setNextPrevBehavior(NextPrevBehavior nextPrev);

    PlaybackLayout playbackLayout() const;
    void setPlaybackLayout(PlaybackLayout layout);

signals:
    void nextPrevBehaviorChanged();
    void playbackLayoutChanged();

private:
    NextPrevBehavior m_nextPrevBehavior;
    PlaybackLayout m_playbackLayout;

    /*********************************************************************
     * Chaser attachment
     *********************************************************************/
public:
    /* Get/Set the ID of the Chaser attached to this Cue List */
    quint32 chaserID() const;
    Q_INVOKABLE void setChaserID(quint32 fid);

    /** Get the chaser function that is used as cue list steps */
    Chaser *chaser();

    /** Return the Chaser step list formatted as explained in m_stepsList */
    QVariant stepsList() const;

    Q_INVOKABLE void addFunctions(QVariantList idsList, int insertIndex = -1);

private:
    FunctionParent functionParent() const;

signals:
    void chaserIDChanged(quint32 id);
    void stepsListChanged();

private:
    /** The ID of the Chaser attached to this Cue List */
    quint32 m_chaserID;

    /** Reference to a ListModel representing the steps list for the QML UI,
     *  organized as follows:
     *  funcID | isSelected | fadeIn | fadeOut | hold | duration | note
     */
    ListModel *m_stepsList;

    /*********************************************************************
     * Playback
     *********************************************************************/
public:
    enum PlaybackStatus
    {
        Stopped = 0,
        Playing,
        Paused
    };
    Q_ENUM(PlaybackStatus)

    int playbackIndex() const;
    void setPlaybackIndex(int playbackIndex);

    PlaybackStatus playbackStatus();

    Q_INVOKABLE void playClicked();
    Q_INVOKABLE void stopClicked();
    Q_INVOKABLE void previousClicked();
    Q_INVOKABLE void nextClicked();

signals:
    void playbackStatusChanged();
    void playbackIndexChanged(int playbackIndex);

private slots:
    /** Slot called whenever a function is started */
    void slotFunctionRunning(quint32 fid);

    /** Slot called whenever a function is stopped */
    void slotFunctionStopped(quint32 fid);

    /** Called when m_runner skips to another step */
    void slotCurrentStepChanged(int stepNumber);

private:
    /** Get the index of the next item, based on the chaser direction */
    int getNextIndex();

    /** Get the index of the previous item, based on the chaser direction */
    int getPrevIndex();

    /** Get the index of the first item, based on the chaser direction */
    int getFirstIndex();

    /** Get the index of the last item, based on the chaser direction */
    int getLastIndex();

    /** Start the associated Chaser */
    void startChaser(int startIndex = -1);

    /** Stop the associated Chaser */
    void stopChaser();

private:
    /** Index of the current step being played. -1 when stopped */
    int m_playbackIndex;

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);
};

#endif
