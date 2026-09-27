/*
  Q Light Controller Plus
  sceneeditor.h

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

#ifndef SCENEEDITOR_H
#define SCENEEDITOR_H

#include "functioneditor.h"
#include "qlcinputsource.h"
#include "scenevalue.h"

class Doc;
class Scene;
class QTimer;
class ListModel;
class GenericDMXSource;

class SceneEditor final : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QVariant fixtureList READ fixtureList NOTIFY fixtureListChanged)
    Q_PROPERTY(QVariant componentList READ componentList NOTIFY componentListChanged)
    Q_PROPERTY(int selectedChannelCount READ selectedChannelCount NOTIFY selectedChannelCountChanged)

    /** External controller properties. @see setExternalControlEnabled */
    Q_PROPERTY(bool externalControlEnabled READ externalControlEnabled WRITE setExternalControlEnabled NOTIFY externalControlEnabledChanged)
    Q_PROPERTY(bool panTiltMode READ panTiltMode WRITE setPanTiltMode NOTIFY panTiltModeChanged)
    Q_PROPERTY(int faderCount READ faderCount NOTIFY externalMapChanged)
    Q_PROPERTY(int externalPage READ externalPage NOTIFY externalMapChanged)
    Q_PROPERTY(int externalPageCount READ externalPageCount NOTIFY externalMapChanged)
    Q_PROPERTY(bool hasJoystick READ hasJoystick NOTIFY externalMapChanged)

public:
    SceneEditor(QQuickView *view, Doc *doc, QObject *parent = nullptr);
    ~SceneEditor();

    /** Set the ID of the Scene to edit */
    void setFunctionID(quint32 id) override;

    /** Return a QVariant list of references to the Fixtures
     *  involved in the Scene editing */
    QVariant fixtureList() const;

    /** Return a QVariant list of all the references to components
     *  involved in the Scene editing */
    QVariant componentList() const;

    /** Enable/disable the preview of the current Scene.
     *  In this editor, the preview is done with a GenericDMXSource */
    void setPreviewEnabled(bool enable) override;

    /** Method called by QML to inform the SceneEditor that
     *  SceneFixtureConsole has been loaded/unloaded. */
    Q_INVOKABLE void sceneConsoleLoaded(bool status);

    Q_INVOKABLE void registerFixtureConsole(int index, QQuickItem *item);
    Q_INVOKABLE void unRegisterFixtureConsole(int index);

    /** QML invokable method that returns if the Scene has the
     *  requested $fixture's $channel */
    Q_INVOKABLE bool hasChannel(quint32 fxID, quint32 channel) const;

    /** QML invokable method that returns the value of the
     *  requested $fixture's $channel */
    Q_INVOKABLE double channelValue(quint32 fxID, quint32 channel) const;

    /** Remove a channel with the provided $fxID and $channel
     *  from the Scene currently being edited */
    Q_INVOKABLE void unsetChannel(quint32 fxID, quint32 channel);

    /** Set a Fixture selection by ID to scroll the UI to the
     *  related FixtureConsole item */
    Q_INVOKABLE void setFixtureSelection(quint32 fxID);

    /** Add/remove a channel from the clipboard selection, to allow
     *  the paste-to-all functionality */
    Q_INVOKABLE void setChannelSelection(quint32 fxID, quint32 channel, bool selected);

    /** Return the number of channels currently selected for paste-to-all */
    int selectedChannelCount() const;

    /** Add a component with of given type
     *  e.g. FixtureGroup, Fixture, Palette */
    Q_INVOKABLE void addComponent(int type, quint32 id);

    /** Paste all the values selected with setChannelSelection
     *  to all the fixture of the same type and mode */
    Q_INVOKABLE void pasteToAllFixtureSameType();

    /** @reimp */
    void deleteItems(QVariantList list) override;

    /*********************************************************************
     * External controllers
     *********************************************************************/
public:
    /** Enable/disable the direct control of the Scene channels from an
     *  external controller (faders/knobs). While enabled, the Virtual
     *  Console is inhibited from receiving any input signal. */
    bool externalControlEnabled() const;
    void setExternalControlEnabled(bool enable);

    /** Get/set the Pan & Tilt mode. When disabled (normal mode) every
     *  fader is mapped 1:1 to a Fixture channel. When enabled, the faders
     *  control pan, pan fine, tilt and tilt fine of a single Fixture */
    bool panTiltMode() const;
    void setPanTiltMode(bool enable);

    /** Number of faders/knobs detected on the patched external controllers */
    int faderCount() const;

    /** Current page of the external controller mapping. In normal mode a
     *  page is a block of $faderCount channels. In Pan & Tilt mode a page
     *  is a single moving Fixture */
    int externalPage() const;

    /** Number of available pages with the current mode and mapping */
    int externalPageCount() const;

    /** Return true if a HID controller with analog axes (e.g. a joystick
     *  or a gamepad) has been detected among the patched universes */
    bool hasJoystick() const;

    /** Move the external controller mapping $direction pages
     *  forward (+1) or backward (-1) */
    Q_INVOKABLE void changeExternalPage(int direction);

    /** Return the index of the fader currently controlling the given
     *  $fxID/$channel, or -1 if the channel is not controlled.
     *  Used by the QML consoles to highlight the mapped channels */
    Q_INVOKABLE int controlledFaderIndex(quint32 fxID, quint32 channel) const;

protected slots:
    void slotSceneValueChanged(SceneValue scv);
    void slotAliasChanged();

    /** Handle an input signal coming from an external controller */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /** Handle a value produced by a relative input source. These arrive
     *  from the QLCInputSource worker thread, which turns the displacement
     *  of a spring loaded control into a continuous absolute value */
    void slotRelativeValueChanged(quint32 universe, quint32 channel, uchar value);

    /** Move pan/tilt by an amount of degrees proportional to how far the
     *  sticks are pushed. Called periodically while a stick is off center */
    void slotPositionTimeout();

private:
    void addFixtureToList(quint32 fid);
    void updateLists();
    void setCacheChannelValue(SceneValue scv);
    void cacheChannelValues();

    /** Scan the patched input universes and build the list of the
     *  available faders/knobs, page buttons and joystick axes */
    void updateExternalMap();

    /** Drop the external controls, stopping the worker threads
     *  of the relative input sources */
    void clearExternalMap();

    /** Rebuild the list of the channels currently reachable by the faders */
    void updateChannelMap();

    /** Release every fader pickup state, so that no fader will affect a
     *  channel until it crosses again the current channel value */
    void resetFaderPickup();

    /** Route a raw external controller $value to the $faderIndex-th fader,
     *  either through its relative input source or directly */
    void feedFaderValue(int faderIndex, uchar value);

    /** Handle a relative (spring loaded) control driving pan or tilt.
     *  $faderIndex selects the axis and $value is the raw stick position,
     *  whose distance from the center becomes a movement speed */
    void feedPositionAxis(int faderIndex, uchar value);

    /** Load the pan/tilt degrees accumulators with the position currently
     *  stored in the Scene for the Fixture of the active page */
    void cachePositionDegrees();

    /** Transmit the pan/tilt values of every moving Fixture of the Scene,
     *  so that the whole rig shows its stored position as soon as the
     *  Pan & Tilt editing begins */
    void outputAllPositions();

    /** Light up the Fixture of the active Pan & Tilt page, so that it can
     *  be spotted on stage while its position is being adjusted. Only the
     *  selected Fixture is lit, one at a time.
     *  The values are written to a dedicated DMX source, so they go to the
     *  output only and are never recorded in the Scene being edited */
    void highlightCurrentFixture();

    /** Switch the temporary highlight off, leaving the transmitted
     *  positions untouched */
    void clearFixtureHighlight();

    /** Apply an absolute external controller $value to the $faderIndex-th
     *  fader, performing the pickup (catch up) check */
    void applyFaderValue(int faderIndex, uchar value);

    /** Return the Fixture channel currently mapped to the $faderIndex-th
     *  fader, honouring the active mode and page. The returned SceneValue
     *  holds an invalid Fixture ID if the fader maps to nothing */
    SceneValue faderTarget(int faderIndex) const;

    /** Select the Fixture of the current page, so that the existing
     *  Fixture selection highlights it in every view and the bottom
     *  panel scrolls to it */
    void selectCurrentPageFixture();

signals:
    void fixtureListChanged();
    void componentListChanged();
    void selectedChannelCountChanged();
    void externalControlEnabledChanged();
    void panTiltModeChanged();
    void externalMapChanged();

private:
    /** Reference of the Scene currently being edited */
    Scene *m_scene;

    /** A list of the $m_scene Fixture IDs for fast lookup */
    QList<quint32> m_fixtureIDs;

    /** A QML-readable list of references to Fixtures used in $m_scene */
    ListModel *m_fixtureList;

    /** A QML-readable list of all the components used by the Scene
     *  (Fixture groups, Fixtures, Palettes) */
    ListModel *m_componentList;

    /** A reference to the SceneFixtureConsole when loaded */
    QQuickItem *m_sceneConsole;

    /** Keep a track of the registered Fixture consoles in a Scene Console,
     *  to rapidly set a channel value */
    QMap<int, QQuickItem *> m_fxConsoleMap;

    /** Pre-cache initial channel values including palettes.
     *  arranged as <fixture ID, channel values array> */
    QMap<quint32, QByteArray> m_channelsCache;

    QList<SceneValue> m_selectedChannels;

    /** Reference to a DMX source used to edit a Scene */
    GenericDMXSource *m_source;

    /** A DMX source dedicated to the temporary highlight of the Fixture
     *  being positioned in Pan & Tilt mode. It is kept separate from
     *  $m_source so that the highlight goes straight to the output and
     *  never ends up in the Scene being edited */
    GenericDMXSource *m_highlightSource;

    /** ID of the Fixture currently lit up by the temporary highlight */
    quint32 m_highlightedFixture;

    /** The channels switched on by the temporary highlight. They are
     *  tracked to be unset one by one, so that turning the highlight off
     *  does not drop the pan/tilt values transmitted on the same source */
    QList<SceneValue> m_highlightChannels;

    /*********************************************************************
     * External controllers
     *********************************************************************/
private:
    /** An input signal source, identified by universe and channel */
    struct ExternalControl
    {
        quint32 universe;
        quint32 channel;

        /** For a relative movement control (e.g. the spring loaded analog
         *  stick of a gamepad) this holds the QLCInputSource that converts
         *  the stick displacement into a continuous absolute value.
         *  Null for plain absolute faders/knobs */
        QSharedPointer<QLCInputSource> source;

        bool operator==(const ExternalControl &rhs) const
        { return universe == rhs.universe && channel == rhs.channel; }
    };

    /** The pickup (catch up) state of a fader. A fader does not affect
     *  the mapped channel until its physical position crosses the
     *  current channel value */
    struct FaderState
    {
        bool caught;        //!< the fader crossed the channel value and is now active
        bool hasValue;      //!< a value has been received at least once
        uchar lastValue;    //!< the last value received from the controller
    };

    /** Flag indicating that the external controller faders are
     *  currently driving the Scene channels */
    bool m_externalControlEnabled;

    /** Flag indicating that the faders control pan/tilt of a single Fixture */
    bool m_panTiltMode;

    /** The current page of the external controller mapping */
    int m_externalPage;

    /** The faders/knobs found on the patched input universes, in profile order */
    QList<ExternalControl> m_faders;

    /** The "Next Page" input channels found on the patched input universes */
    QList<ExternalControl> m_nextPageControls;

    /** The "Previous Page" input channels found on the patched input universes */
    QList<ExternalControl> m_prevPageControls;

    /** The analog axes of a HID controller (joystick/gamepad), used to
     *  drive pan and tilt. Only the first 4 axes are considered */
    QList<ExternalControl> m_joystickAxes;

    /** The pickup state of every fader, with the same indices as $m_faders */
    QList<FaderState> m_faderStates;

    /** Normalized displacement of the pan/tilt sticks, in the -1.0 .. +1.0
     *  range. Zero means the stick is centered and no movement happens */
    float m_panSpeed;
    float m_tiltSpeed;

    /** Absolute pan/tilt position in degrees, accumulated while the sticks
     *  are moved. This cannot be re-read from the Fixture on every step,
     *  because Fixture::channelValueAt() reflects the DMX output, which
     *  stays still while the Scene is edited in blind mode */
    float m_panDegrees;
    float m_tiltDegrees;

    /** Timer that moves pan/tilt while a relative control is held away
     *  from its center. Runs only while there is movement to apply */
    QTimer *m_positionTimer;

    /** A page of the normal mode mapping: a block of consecutive channels
     *  of a single Fixture. Pages never cross a Fixture boundary, so a
     *  Fixture with 12 channels on an 8 fader controller produces two
     *  pages: channels 0-7 and channels 8-11 */
    struct ChannelPage
    {
        quint32 fixtureID;
        quint32 firstChannel;
        int channelCount;
    };

    /** The normal mode pages, built by walking the Scene Fixtures in
     *  the same order as the bottom panel consoles */
    QList<ChannelPage> m_channelPages;

    /** The IDs of the Fixtures with pan/tilt capabilities, used
     *  as pages in Pan & Tilt mode */
    QList<quint32> m_movingFixtures;
};

#endif // SCENEEDITOR_H
