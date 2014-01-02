/*
  Q Light Controller
  fadechannel.h

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

#include "chaser.h"
#include "scene.h"

class QDomDocument;
class QDomElement;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCTrack "Track"

class Track : public QObject
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** Create a new Track and associate it to a Scene  */
    Track(quint32 sceneID = Scene::invalidId());

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

private:
    quint32 m_id;

    /************************************************************************
     * Name
     ************************************************************************/
public:
    /** Set the name of the track */
    void setName(const QString& name);

    /** Get the name of this track */
    QString name() const;

private:
    QString m_name;

    /*********************************************************************
     * Scene
     *********************************************************************/
public:
    /** Return the Scene ID associated to this track */
    quint32 getSceneID();

private:
    /** Pointer to a Scene which this track represents */
    quint32 m_sceneID;

    /*********************************************************************
     * Mute state
     *********************************************************************/
public:
    /** Set the mute state of this track */
    void setMute(bool);

    /** Return the mute state of the track */
    bool isMute();

private:
    /** Flag to mute/unmute this track */
    bool m_isMute;

    /*********************************************************************
     * Sequences
     *********************************************************************/
public:
    /** associate a function ID to this track */
    bool addFunctionID(quint32 id);

    /** remove a function ID association from this track */
    bool removeFunctionID(quint32 id);

    QList <quint32> functionsID();

private:
    /** List of Function IDs present in this track */
    QList <quint32> m_functions;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    bool loadXML(const QDomElement& root);

};

/** @} */

/** @} */

#endif
