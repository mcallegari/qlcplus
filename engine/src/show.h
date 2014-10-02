/*
  Q Light Controller Plus
  show.h

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

#ifndef SHOW_H
#define SHOW_H

#include <QMutex>
#include <QList>
#include <QSet>

#include "function.h"
#include "track.h"

class QDomDocument;
class ShowRunner;

/** @addtogroup engine_functions Functions
 * @{
 */

class Show : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Show)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Show(Doc* doc);
    virtual ~Show();

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Time division
     *********************************************************************/
    /** Set the show time division type (Time, BPM) */
    void setTimeDivision(QString type, int BPM);

    QString getTimeDivisionType();
    int getTimeDivisionBPM();

private:
    QString m_timeDivType;
    int m_timeDivBPM;

    /*********************************************************************
     * Tracks
     *********************************************************************/
public:
    /**
     * Add a track to this show. If the track is already a
     * member of the show, this call fails.
     *
     * @param id The track to add
     * @return true if successful, otherwise false
     */
    bool addTrack(Track *track, quint32 id = Track::invalidId());

    /**
     * Remove a track from this show. If the track is not a
     * member of the show, this call fails.
     *
     * @param id The track to remove
     * @return true if successful, otherwise false
     */
    bool removeTrack(quint32 id);

    /** Get a track by id */
    Track* track(quint32 id) const;

    /** Get pointer to a Track from a Scene ID */
    Track* getTrackFromSceneID(quint32 id);

    /** Get the number of tracks in the Show */
    int getTracksCount();

    /** Move a track ID up or down */
    void moveTrack(Track *track, int direction);

    /** Get a list of available tracks */
    QList <Track*> tracks() const;

private:
    /** Create a new track ID */
    quint32 createTrackId();

protected:
    /** Map of the available tracks coupled by ID */
    QMap <quint32,Track*> m_tracks;

    /** Latest assigned track ID */
    quint32 m_latestTrackId;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save function's contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    /** Load function's contents from an XML document */
    bool loadXML(const QDomElement& root);

    /** @reimp */
    void postLoad();

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void write(MasterTimer* timer, QList<Universe*> universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, QList<Universe*> universes);

protected slots:
    /** Called whenever one of this function's child functions stops */
    void slotChildStopped(quint32 fid);

signals:
    void timeChanged(quint32);
    void showFinished();

protected:
    ShowRunner *m_runner;
    /** Number of currently running children */
    QSet <quint32> m_runningChildren;

    /*************************************************************************
     * Attributes
     *************************************************************************/
public:
    /** @reimpl */
    void adjustAttribute(qreal fraction, int attributeIndex = 0);
};

/** @} */

#endif
