/*
  Q Light Controller
  fadechannel.h

  Copyright (c) Massimo Callegari

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

#ifndef TRACK_H
#define TRACK_H

#include <QObject>

#include "chaser.h"
#include "scene.h"

class QDomDocument;
class QDomElement;

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

private:
    /** Pointer to a Scene which this track represents */
    quint32 m_sceneID;
    /** Flag to mute/unmute this track */
    bool m_isMute;

signals:
    /** Emitted whenever some property is changed */
    void changed(quint32 id);

    /************************************************************************
     * ID
     ************************************************************************/
public:
    /** Set a channels group's id (only Doc is allowed to do this!) */
    void setId(quint32 id);

    /** Get a channels group's unique id */
    quint32 id() const;

    /** Get an invalid channels group id */
    static quint32 invalidId();

private:
    quint32 m_id;

    /************************************************************************
     * Name
     ************************************************************************/
public:
    /** Set the name of a channels group */
    void setName(const QString& name);

    /** Get the name of a channels group */
    QString name() const;

private:
    QString m_name;

    /*********************************************************************
     * Scene
     *********************************************************************/
public:
    quint32 getSceneID();

    /*********************************************************************
     * Sequences
     *********************************************************************/
public:
    bool addSequence(Chaser *seq);

    QList <quint32> sequences();

private:
    /** List of Chaser IDs (in sequence mode) present in this track */
    QList <quint32> m_sequences;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    bool loadXML(const QDomElement& root);

};

#endif
