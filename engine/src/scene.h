/*
  Q Light Controller
  scene.h

  Copyright (C) Heikki Junnila

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

#ifndef SCENE_H
#define SCENE_H

#include <QMutex>
#include <QList>

#include "genericfader.h"
#include "fadechannel.h"
#include "scenevalue.h"
#include "dmxsource.h"
#include "function.h"
#include "fixture.h"

#define KXMLQLCFixtureValues "FixtureVal"
#define KXMLQLCSceneChannelGroups "ChannelGroups"

class QDomDocument;
class QDomElement;

/**
 * Scene encapsulates the values of selected channels from one or more fixture
 * instances. When a scene is started, the duration it takes for its channels
 * to reach their target values depends on the function's fade in speed setting.
 * For HTP channels, the fade out setting is also used when the scene is toggled off.
 * If the speed value is 0 seconds, scene values are set immediately and no
 * fading occurs. Otherwise values are always faded from what they currently
 * are, to the target values defined in the scene (with SceneValue instances).
 */
class Scene : public Function, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(Scene)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Construct a new scene function, with given parent object. If the
     * parent is not a Doc* object, the debug build asserts.
     *
     * @param doc The parent object who owns the scene
     */
    Scene(Doc* doc);

    /**
     * Destroy the scene
     */
    ~Scene();

    void setChildrenFlag(bool flag);

private:
    quint32 m_legacyFadeBus;

    /** flag that says if a scene is used by some Chaser in sequence mode */
    bool m_hasChildren;

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** @reimpl */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Values
     *********************************************************************/
public:
    /**
     * Set the value of one fixture channel, using a predefined SceneValue
     */
    void setValue(const SceneValue& scv);

    /**
     * Set the value of one fixture channel, specify parameters separately
     */
    void setValue(quint32 fxi, quint32 ch, uchar value);

    /**
     * Clear the value of one fixture channel
     */
    void unsetValue(quint32 fxi, quint32 ch);

    /**
     * Get the value of one fixture channel
     */
    uchar value(quint32 fxi, quint32 ch);

    /**
     * Verify if the given value belongs to the Scene
     */
    bool checkValue(SceneValue val);

    /**
     * Get a list of values in this scene
     */
    QList <SceneValue> values() const;

    /**
     * Clear all values
     */
    void clear();

protected:
    QList <SceneValue> m_values;
    QMutex m_valueListMutex;

    /*********************************************************************
     * Channel Groups
     *********************************************************************/
public:
    /**
     * Associate a channel group to this Scene
     */
    void addChannelGroup(quint32 id);

    /**
     * Remove a previously associated channel group from this Scene
     */
    void removeChannelGroup(quint32 id);

    /**
     * Returns the list of Channel Groups associated to this Scene
     */
    QList<quint32> channelGroups();

    /**
     * Set the level of a channel group with the given id
     */
    void setChannelGroupLevel(quint32 id, uchar level);

    /**
     * Returns the list of Channel Groups levels
     */
    QList<uchar> channelGroupsLevels();

protected:
    QList <quint32> m_channelGroups;
    QList <uchar> m_channelGroupsLevels;

    /*********************************************************************
     * Display Mode
     *********************************************************************/
public:
    void setViewMode(bool tabbed);

    bool viewMode();

protected:
    /** Holds the display mode (tabbed or all channels ) to be used by Scene Editor */
    bool m_viewMode;

    /*********************************************************************
     * Fixtures
     *********************************************************************/
public slots:
    void slotFixtureRemoved(quint32 fxi_id);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** @reimpl */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    /** @reimpl */
    bool loadXML(const QDomElement& root);

    /** @reimpl */
    void postLoad();

    /*********************************************************************
     * Flash
     *********************************************************************/
public:
    /** @reimpl */
    void flash(MasterTimer* timer);

    /** @reimpl */
    void unFlash(MasterTimer* timer);

    /** @reimpl from DMXSource */
    void writeDMX(MasterTimer* timer, UniverseArray* ua);

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* ua);

    /** @reimpl */
    void postRun(MasterTimer* timer, UniverseArray* ua);

private:
    /** Insert starting values to $fc, either from $timer->fader() or $ua */
    void insertStartValue(FadeChannel& fc, const MasterTimer* timer, const UniverseArray* ua);

private:
    GenericFader* m_fader;

    /*********************************************************************
     * Intensity
     *********************************************************************/
public:
    /** @reimpl */
    void adjustIntensity(qreal intensity);
};

#endif
