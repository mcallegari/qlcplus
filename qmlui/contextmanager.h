/*
  Q Light Controller Plus
  contextmanager.h

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

#ifndef CONTEXTMANAGER_H
#define CONTEXTMANAGER_H

#include <QObject>
#include <QQuickView>
#include <QVector3D>

#include "qlcchannel.h"
#include "scenevalue.h"

class Doc;
class MainView2D;
class MainView3D;
class MainViewDMX;
class FixtureManager;
class FunctionManager;
class GenericDMXSource;
class MonitorProperties;
class PreviewContext;

class ContextManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentContext READ currentContext NOTIFY currentContextChanged)
    Q_PROPERTY(QVector3D environmentSize READ environmentSize WRITE setEnvironmentSize NOTIFY environmentSizeChanged)
    Q_PROPERTY(quint32 universeFilter READ universeFilter WRITE setUniverseFilter NOTIFY universeFilterChanged)
    Q_PROPERTY(int selectedFixturesCount READ selectedFixturesCount NOTIFY selectedFixturesChanged)
    Q_PROPERTY(QVector3D fixturesPosition READ fixturesPosition WRITE setFixturesPosition NOTIFY fixturesPositionChanged)
    Q_PROPERTY(QVector3D fixturesRotation READ fixturesRotation WRITE setFixturesRotation NOTIFY fixturesRotationChanged)
    Q_PROPERTY(int dumpValuesCount READ dumpValuesCount NOTIFY dumpValuesCountChanged)
    Q_PROPERTY(quint32 dumpChannelMask READ dumpChannelMask NOTIFY dumpChannelMaskChanged)
    Q_PROPERTY(bool positionPicking READ positionPicking WRITE setPositionPicking NOTIFY positionPickingChanged)

public:
    explicit ContextManager(QQuickView *view, Doc *doc,
                            FixtureManager *fxMgr, FunctionManager *funcMgr,
                            QObject *parent = 0);
    ~ContextManager();

    /** Register/Unregister a context to the map of known contexts */
    void registerContext(PreviewContext *context);
    void unregisterContext(QString name);

    /** Enable/disable the context with the specified $name.
     *  This sets a flag in the context to know if it is visible
     *  on the screen, so to decide if changes should be applied to it */
    Q_INVOKABLE void enableContext(QString name, bool enable, QQuickItem *item);

    /** Detach/Reattach a context from/to the application main window */
    Q_INVOKABLE void detachContext(QString name);
    Q_INVOKABLE void reattachContext(QString name);

    /** Switch to the context with the given $name.
     *  Supports both QLC+ 4 and QLC+ 5 context names */
    void switchToContext(QString name);

    /** Return the currently active context */
    QString currentContext() const;

    /** Get/Set the environment width/height/depth size */
    QVector3D environmentSize() const;
    void setEnvironmentSize(QVector3D environmentSize);

    /** Enable/Disable a position picking process */
    bool positionPicking() const;
    void setPositionPicking(bool enable);

    Q_INVOKABLE void setPositionPickPoint(QVector3D point);

signals:
    void currentContextChanged();
    void environmentSizeChanged();
    void positionPickingChanged();

public slots:
    /** Resets the data structures and update the currently enabled views */
    void resetContexts();

    /** Handle a key press from a QQuickView context */
    void handleKeyPress(QKeyEvent *e);

    /** Handle a key release from a QQuickView context */
    void handleKeyRelease(QKeyEvent *e);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the Doc Monitor properties */
    MonitorProperties *m_monProps;

    /** Reference to a simple PreviewContext representing
     *  the universe grid view, since it doesn't have a dedicated class */
    PreviewContext *m_uniGridView;
    /** Reference to the DMX Preview context */
    MainViewDMX *m_DMXView;
    /** Reference to the 2D Preview context */
    MainView2D *m_2DView;
    /** Reference to the 3D Preview context */
    MainView3D *m_3DView;
    /** Reference to the Fixture Manager */
    FixtureManager *m_fixtureManager;
    /** Reference to the Function Manager */
    FunctionManager *m_functionManager;

    QMap <QString, PreviewContext *> m_contextsMap;

    /** Flag that indicates if a position picking is active */
    bool m_positionPicking;

    /*********************************************************************
     * Universe filtering
     *********************************************************************/
public:
    /** Get/Set the universe displayed by contexts */
    quint32 universeFilter() const;
    void setUniverseFilter(quint32 universeFilter);

signals:
    void universeFilterChanged(quint32 universeFilter);

private:
    /** The currently displayed universe
      * The value Universe::invalid() means "All universes" */
    quint32 m_universeFilter;

    /*********************************************************************
     * Common fixture helpers
     *********************************************************************/
public:
    /** Select/Deselect a fixture with the provided $itemID */
    Q_INVOKABLE void setFixtureSelection(quint32 itemID, bool enable);

    /** Deselect all the currently selected fixtures */
    Q_INVOKABLE void resetFixtureSelection();

    /** Toggle between none/all fixture selection */
    Q_INVOKABLE void toggleFixturesSelection();

    /** Select the fixtures that intersects the provided rectangle coordinates in a 2D environment */
    Q_INVOKABLE void setRectangleSelection(qreal x, qreal y, qreal width, qreal height);

    /** Returns if at least one fixture is currently selected */
    int selectedFixturesCount();

    /** Returns if the fixture with $fxID is currently selected */
    Q_INVOKABLE bool isFixtureSelected(quint32 itemID);

    /** Sets the position of the Fixture with the provided $itemID */
    Q_INVOKABLE void setFixturePosition(quint32 itemID, qreal x, qreal y, qreal z);

    /** Adds an offset (in mm) to the selected Fixture positions. This is called only by the 2D view */
    Q_INVOKABLE void setFixturesOffset(qreal x, qreal y);

    /** Set/Get the position of the currently selected fixtures */
    QVector3D fixturesPosition() const;
    void setFixturesPosition(QVector3D position);

    /** Align the currently selected Fixtures with the provided $alignment */
    Q_INVOKABLE void setFixturesAlignment(int alignment);

    /** Distribute the currently selected Fixtures with the provided $direction */
    Q_INVOKABLE void setFixturesDistribution(int direction);

    Q_INVOKABLE void updateFixturesCapabilities();

    Q_INVOKABLE void createFixtureGroup();

    /** Set/Get the rotation of the currently selected fixtures */
    QVector3D fixturesRotation() const;
    void setFixturesRotation(QVector3D degrees);

    /** Select/Deselect all the fixtures of the Group/Universe with the provided $id */
    Q_INVOKABLE void setFixtureGroupSelection(quint32 id, bool enable, bool isUniverse);

protected slots:
    void slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z = 0);
    void slotFixtureDeleted(quint32 itemID);
    void slotChannelValueChanged(quint32 fxID, quint32 channel, quint8 value);
    void slotChannelTypeValueChanged(int type, quint8 value, quint32 channel = UINT_MAX);
    void slotColorChanged(QColor col, QColor wauv);
    void slotPositionChanged(int type, int degrees);
    void slotPresetChanged(const QLCChannel *channel, quint8 value);

    /** Invoked by the QLC+ engine to inform the UI that the Universe at $idx
     *  has changed */
    void slotUniversesWritten(int idx, const QByteArray& ua);

    /** Invoked when Function editing begins or ends in the Function Manager.
     *  Context Manager doesn't care much about Functions, it just needs
     *  to know if it has to set channel values on the GenericDMXSource or
     *  forward them to the Function Manager */
    void slotFunctionEditingChanged(bool status);

signals:
    void selectedFixturesChanged();
    void fixturesPositionChanged();
    void fixturesRotationChanged();

private:
    /** The list of the currently selected Fixture IDs */
    QList<quint32> m_selectedFixtures;

    /** A flag indicating if a Function is currently being edited */
    bool m_editingEnabled;

    /** A multihash containing the selected fixtures' capabilities by channel type */
    /** The hash is: int (channel type) , SceneValue (Fixture ID and channel) */
    QMultiHash<int, SceneValue> m_channelsMap;

    /*********************************************************************
     * DMX channels dump
     *********************************************************************/
public:
    enum ChannelType
    {
        DimmerType      = (1 << QLCChannel::Intensity),
        ColorMacroType  = (1 << QLCChannel::Colour), // Color wheels, color macros
        GoboType        = (1 << QLCChannel::Gobo),
        SpeedType       = (1 << QLCChannel::Speed),
        PanType         = (1 << QLCChannel::Pan),
        TiltType        = (1 << QLCChannel::Tilt),
        ShutterType     = (1 << QLCChannel::Shutter),
        PrismType       = (1 << QLCChannel::Prism),
        BeamType        = (1 << QLCChannel::Beam),
        EffectType      = (1 << QLCChannel::Effect),
        MaintenanceType = (1 << QLCChannel::Maintenance),
        ColorType       = (1 << (QLCChannel::Maintenance + 1)) // RGB/CMY/WAUV
    };
    Q_ENUM(ChannelType)

    /** Store a channel value for Scene dumping */
    void setDumpValue(quint32 fxID, quint32 channel, uchar value);

    /** Return the number of DMX channels currently available for dumping */
    int dumpValuesCount() const;

    /** Return the current DMX dump channel type mask */
    int dumpChannelMask() const;

    Q_INVOKABLE void dumpDmxChannels(QString name, quint32 mask);

    Q_INVOKABLE void dumpDmxChannels(quint32 sceneID, quint32 mask);

    /** Resets the current values used for dumping or preview */
    Q_INVOKABLE void resetDumpValues();

    GenericDMXSource *dmxSource() const;

signals:
    void dumpValuesCountChanged();
    void dumpChannelMaskChanged();

private:
    /** List of the values available for dumping to a Scene */
    QList <SceneValue> m_dumpValues;

    /** Bitmask representing the available channel types for
     *  the DMX channels ready for dumping */
    quint32 m_dumpChannelMask;

    /** Reference to a Generic DMX source used to handle Scenes dump */
    GenericDMXSource* m_source;
};

#endif // CONTEXTMANAGER_H
