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

#include <QTimer>

#include "vcwidget.h"

#define KXMLQLCVCCueList                    QString("CueList")
#define KXMLQLCVCCueListChaser              QString("Chaser")
#define KXMLQLCVCCueListPlaybackLayout      QString("PlaybackLayout")
#define KXMLQLCVCCueListNextPrevBehavior    QString("NextPrevBehavior")
#define KXMLQLCVCCueListCrossfade           QString("Crossfade")
#define KXMLQLCVCCueListNext                QString("Next")
#define KXMLQLCVCCueListPrevious            QString("Previous")
#define KXMLQLCVCCueListPlayback            QString("Playback")
#define KXMLQLCVCCueListStop                QString("Stop")
#define KXMLQLCVCCueListSlidersMode         QString("SlidersMode")
#define KXMLQLCVCCueListCrossfadeLeft       QString("CrossLeft")
#define KXMLQLCVCCueListCrossfadeRight      QString("CrossRight")

class ListModel;

class VCCueList : public VCWidget
{
    Q_OBJECT

    Q_PROPERTY(quint32 chaserID READ chaserID WRITE setChaserID NOTIFY chaserIDChanged)
    Q_PROPERTY(QVariant stepsList READ stepsList NOTIFY stepsListChanged)

    Q_PROPERTY(NextPrevBehavior nextPrevBehavior READ nextPrevBehavior WRITE setNextPrevBehavior NOTIFY nextPrevBehaviorChanged)
    Q_PROPERTY(PlaybackLayout playbackLayout READ playbackLayout WRITE setPlaybackLayout NOTIFY playbackLayoutChanged)

    Q_PROPERTY(FaderMode sideFaderMode READ sideFaderMode WRITE setSideFaderMode NOTIFY sideFaderModeChanged)
    Q_PROPERTY(int sideFaderLevel READ sideFaderLevel WRITE setSideFaderLevel NOTIFY sideFaderLevelChanged)
    Q_PROPERTY(bool primaryTop READ primaryTop NOTIFY primaryTopChanged)
    Q_PROPERTY(int nextStepIndex READ nextStepIndex NOTIFY nextStepIndexChanged)

    Q_PROPERTY(PlaybackStatus playbackStatus READ playbackStatus NOTIFY playbackStatusChanged)
    Q_PROPERTY(int playbackIndex READ playbackIndex WRITE setPlaybackIndex NOTIFY playbackIndexChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCCueList(Doc* doc = nullptr, QObject *parent = nullptr);
    virtual ~VCCueList();

    /** @reimp */
    QString defaultCaption();

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page);

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent);

    /** @reimp */
    void adjustIntensity(qreal val);

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget);

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

    /** Get/Set the next/previous buttons behaviour */
    NextPrevBehavior nextPrevBehavior() const;
    void setNextPrevBehavior(NextPrevBehavior nextPrev);

    /** Get/Set the playback (play/pause/stop) layout */
    PlaybackLayout playbackLayout() const;
    void setPlaybackLayout(PlaybackLayout layout);

signals:
    void nextPrevBehaviorChanged();
    void playbackLayoutChanged();

private:
    NextPrevBehavior m_nextPrevBehavior;
    PlaybackLayout m_playbackLayout;

    /*************************************************************************
     * Side fader
     *************************************************************************/
public:
    enum FaderMode
    {
        None = 0,
        Crossfade,
        Steps
    };
    Q_ENUM(FaderMode)

    /** Get/Set the side fader mode */
    FaderMode sideFaderMode() const;
    void setSideFaderMode(FaderMode mode);

    /** Convert side fader mode <-> string */
    FaderMode stringToFaderMode(QString modeStr);
    QString faderModeToString(FaderMode mode);

    /** Get/Set the side fader level */
    int sideFaderLevel() const;
    void setSideFaderLevel(int level);

    bool primaryTop() const;
    int nextStepIndex() const;

protected:
    qreal getPrimaryIntensity() const;
    int getFadeMode() const;
    void stopStepIfNeeded(Chaser *ch);

signals:
    void sideFaderModeChanged();
    void sideFaderLevelChanged();
    void primaryTopChanged();
    void nextStepIndexChanged();

private:
    FaderMode m_slidersMode;
    int m_sideFaderLevel;
    int m_nextStepIndex;
    bool m_primaryTop;

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

    Q_INVOKABLE void setStepNote(int index, QString text);

private slots:
    void slotFunctionRemoved(quint32 fid);
    void slotFunctionNameChanged(quint32 fid);
    void slotStepChanged(int index);
    void slotStepsListChanged(quint32 fid);

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

    enum ProgressStatus
    {
        ProgressIdle,
        ProgressFadeIn,
        ProgressHold,
        ProgressInfinite
    };
    Q_ENUM(ProgressStatus)

    int playbackIndex() const;
    void setPlaybackIndex(int playbackIndex);

    PlaybackStatus playbackStatus();

    Q_INVOKABLE void playClicked();
    Q_INVOKABLE void stopClicked();
    Q_INVOKABLE void previousClicked();
    Q_INVOKABLE void nextClicked();
    Q_INVOKABLE void playCurrentStep();

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

    /** Method to update the playback progress status */
    void slotProgressTimeout();

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

    QTimer *m_timer;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** @reimp */
    void updateFeedback();

public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value);

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
