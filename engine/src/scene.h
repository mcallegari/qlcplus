/*
  Q Light Controller Plus
  scene.h

  Copyright (C) Heikki Junnila
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

class QXmlStreamReader;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCFixtureValues "FixtureVal"
#define KXMLQLCSceneChannelGroupsValues "ChannelGroupsVal"

// Legacy: these do not contain ChannelGroups values
#define KXMLQLCSceneChannelGroups "ChannelGroups"

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

    /** @reimp */
    QIcon getIcon() const;

    void setChildrenFlag(bool flag);

    /** @reimp */
    quint32 totalDuration();

private:
    quint32 m_legacyFadeBus;

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimp */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** @reimp */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Values
     *********************************************************************/
public:
    /**
     * Set the value of one fixture channel, using a predefined SceneValue
     */
    void setValue(const SceneValue& scv, bool blind = false, bool checkHTP = true);

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

    /** @reimp */
    QList<quint32> components();

    /**
     * Try to retrieve a RGB/CMY color if the Scene has RGB/CMY channels set.
     * A fixture ID can be specified to retrieve a single fixture color.
     * If none, an empty color will be returned.
     */
    QColor colorValue(quint32 fxi = Fixture::invalidId());

    /**
     * Clear all values
     */
    void clear();

signals:
    void valueChanged(SceneValue scv);

protected:
    QMap <SceneValue, uchar> m_values;
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
     * Fixtures
     *********************************************************************/
public slots:
    void slotFixtureRemoved(quint32 fxi_id);

public:
    void addFixture(quint32 fixtureId);
    bool removeFixture(quint32 fixtureId);
    QList<quint32> fixtures() const;

private:
    QList<quint32> m_fixtures;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);

    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    void postLoad();

private:
    static bool saveXMLFixtureValues(QXmlStreamWriter* doc, quint32 fixtureID, QStringList const& values);

    /*********************************************************************
     * Flash
     *********************************************************************/
public:
    /** @reimp */
    void flash(MasterTimer* timer);

    /** @reimp */
    void unFlash(MasterTimer* timer);

    /** @reimp from DMXSource */
    void writeDMX(MasterTimer* timer, QList<Universe*> ua);

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimp */
    void preRun(MasterTimer* timer);

    /** @reimp */
    void write(MasterTimer* timer, QList<Universe*> ua);

    /** @reimp */
    void postRun(MasterTimer* timer, QList<Universe*> ua);

private:
    /** Insert starting values to $fc, either from $timer->fader() or $ua */
    void insertStartValue(FadeChannel& fc, const MasterTimer* timer, const QList<Universe *> ua);

private:
    GenericFader* m_fader;

    /*********************************************************************
     * Attributes
     *********************************************************************/
public:
    /** @reimp */
    int adjustAttribute(qreal fraction, int attributeId);

    /*************************************************************************
     * Blend mode
     *************************************************************************/
public:
    /** @reimp */
    void setBlendMode(Universe::BlendMode mode);
};

/** @} */

#endif
