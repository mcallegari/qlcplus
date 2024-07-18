/*
  Q Light Controller Plus
  track.h

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

#ifndef TRACK_H
#define TRACK_H

#include <QObject>
#include <QHash>

#include "showfunction.h"
#include "scene.h"

class QXmlStreamReader;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCTrack        QString("Track")
#define KXMLQLCTrackID      QString("ID")

class Track : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool mute READ isMute WRITE setMute NOTIFY muteChanged)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** Create a new Track and associate it to a Scene  */
    Track(quint32 sceneID = Function::invalidId(), QObject *parent = 0);

    /** destroy this Track */
    ~Track();

signals:
    /** Emitted whenever some property is changed */
    void changed(quint32 id);

    /************************************************************************
     * ID
     ************************************************************************/
public:
    /** Set a track id (only Doc is allowed to do this!) */
    void setId(quint32 id);

    /** Get a track unique id */
    quint32 id() const;

    /** Get an invalid track id */
    static quint32 invalidId();

    /** Get/Set the Show ID this Track belongs to */
    quint32 showId();
    void setShowId(quint32 id);

private:
    quint32 m_id;
    quint32 m_showId;

    /************************************************************************
     * Name
     ************************************************************************/
public:
    /** Set the name of the track */
    void setName(const QString& name);

    /** Get the name of this track */
    QString name() const;

signals:
    void nameChanged();

private:
    QString m_name;

    /*********************************************************************
     * Scene
     *********************************************************************/
public:
    /** Set the Scene ID associated to this track */
    void setSceneID(quint32 id);

    /** Return the Scene ID associated to this track */
    quint32 getSceneID();

private:
    /** ID of the Scene which this track represents
     *  Returns Function::invalidId() if no Scene is
     *  represented (e.g. audio/video tracks)
     */
    quint32 m_sceneID;

    /*********************************************************************
     * Mute state
     *********************************************************************/
public:
    /** Set the mute state of this track */
    void setMute(bool);

    /** Return the mute state of the track */
    bool isMute();

signals:
    void muteChanged(bool mute);

private:
    /** Flag to mute/unmute this track */
    bool m_isMute;

    /*********************************************************************
     * Functions
     *********************************************************************/
public:
    /**
     * Add a ShowFunction with the given Function ID to the track.
     * If the function doesn't exist, it creates it.
     * In any case it returns the ShowFunction pointer
     */
    ShowFunction *createShowFunction(quint32 functionID);

    /** remove a function ID association from this track */
    bool removeShowFunction(ShowFunction *function, bool performDelete = true);

    /** add a ShowFunction element to this track */
    bool addShowFunction(ShowFunction *func);

    /** Get a reference to a ShowFunction with the provided ID */
    ShowFunction *showFunction(quint32 id);

    /** Returns the list of ShowFunctions added to this Track */
    QList <ShowFunction *> showFunctions() const;

private:
    /** List of Function IDs present in this track */
    QList <ShowFunction *> m_functions;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool saveXML(QXmlStreamWriter *doc);

    bool loadXML(QXmlStreamReader &root);

    bool postLoad(Doc *doc);

public:
    bool contains(Doc *doc, quint32 functionId);

    QList<quint32> components();

};

/** @} */

#endif
