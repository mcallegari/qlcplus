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
