/*
  Q Light Controller Plus
  fixturemanager.h

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

#ifndef FIXTUREMANAGER_H
#define FIXTUREMANAGER_H

#include <QQmlListProperty>
#include <QQuickView>
#include <QMultiHash>
#include <QObject>
#include <QList>

#include "scenevalue.h"
#include "treemodel.h"

class Doc;
class Fixture;

class FixtureManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int fixturesCount READ fixturesCount NOTIFY fixturesCountChanged)
    Q_PROPERTY(QQmlListProperty<Fixture> fixtures READ fixtures)
    Q_PROPERTY(QVariantList fixturesMap READ fixturesMap NOTIFY fixturesMapChanged)
    Q_PROPERTY(QVariantList fixtureNamesMap READ fixtureNamesMap NOTIFY fixtureNamesMapChanged)
    Q_PROPERTY(QVariant groupsTreeModel READ groupsTreeModel NOTIFY groupsTreeModelChanged)
    Q_PROPERTY(QVariant groupsListModel READ groupsListModel NOTIFY groupsListModelChanged)
    Q_PROPERTY(quint32 universeFilter READ universeFilter WRITE setUniverseFilter NOTIFY universeFilterChanged)

    Q_PROPERTY(QVariantList goboChannels READ goboChannels NOTIFY goboChannelsChanged)
    Q_PROPERTY(QVariantList colorWheelChannels READ colorWheelChannels NOTIFY colorWheelChannelsChanged)
    Q_PROPERTY(int colorsMask READ colorsMask NOTIFY colorsMaskChanged)

public:
    FixtureManager(QQuickView *view, Doc *doc, QObject *parent = 0);

    Q_INVOKABLE quint32 invalidFixture();
    Q_INVOKABLE quint32 fixtureForAddress(quint32 index);
    /** Returns a list of the universe indices occupied by a Fixture
        at the requested $address */
    Q_INVOKABLE QVariantList fixtureSelection(quint32 address);

    Q_INVOKABLE bool addFixture(QString manuf, QString model, QString mode, QString name,
                                int uniIdx, int address, int channels, int quantity, quint32 gap,
                                qreal xPos, qreal yPos);
    Q_INVOKABLE bool moveFixture(quint32 fixtureID, quint32 newAddress);

    Q_INVOKABLE QString channelIcon(quint32 fxID, quint32 chIdx);

    Q_INVOKABLE void setChannelValue(quint32 fixtureID, quint32 channelIndex, quint8 value);
    Q_INVOKABLE void setIntensityValue(quint8 value);
    Q_INVOKABLE void setColorValue(quint8 red, quint8 green, quint8 blue,
                                   quint8 white, quint8 amber, quint8 uv);
    Q_INVOKABLE void setPanValue(int degrees);
    Q_INVOKABLE void setTiltValue(int degrees);
    Q_INVOKABLE void setPresetValue(int index, quint8 value);

    /**
     * @brief setFixtureCapabilities
     * @param fxID the Fixture unique ID
     * @param enable used to increment/decrement the UI tools counters
     * @return A multihash containg the fixture capabilities by channel type
     */
    QMultiHash<int, SceneValue> getFixtureCapabilities(quint32 fxID, bool enable);

    /** Returns the number of fixtures currently loaded in the project */
    int fixturesCount();

    /** Returns a QML-readable list of references to Fixture classes */
    QQmlListProperty<Fixture> fixtures();

    /** Returns the data model to display a tree of FixtureGroups/Fixtures */
    QVariant groupsTreeModel();

    /** Returns the data model to display a list of FixtureGroups with icons */
    QVariant groupsListModel();

    /** Add a list of fixture IDs to a new fixture group */
    void addFixturesToNewGroup(QList<quint32>fxList);

    /** Returns a list of SceneValues containing the requested position
     *  information for the specified Fixture with $fxID and $type (Pan/Tilt).
     *  This works on degrees because it considers 16-bit modes as well as
     *  DMX values properly scaled depending on the Fixture max Pan/Tilt degrees.
     *  It also provides multiple results if multiple heads are available */
    QList<SceneValue> getFixturePosition(quint32 fxID, int type, int degrees);

    /** Returns the names of the currently selected fixtures with gobo channels.
     *  The names are in the format: Product - Channel name */
    QVariantList goboChannels();

    /** Returns the names of the currently selected fixtures with color wheel channels.
     *  The names are in the format: Product - Channel name */
    QVariantList colorWheelChannels();

    /** Returns the list of QLCCapability in QVariant format for
     *  the channel cached at the given index */
    Q_INVOKABLE QVariantList presetCapabilities(int index);

    /** Returns a list of fixture names for representation in a GridEditor QML component */
    QVariantList fixtureNamesMap();

    /** Returns data for representation in a GridEditor QML component */
    QVariantList fixturesMap();

    quint32 universeFilter() const;
    void setUniverseFilter(quint32 universeFilter);

    /** Returns the currently available colors as a bitmask */
    int colorsMask() const;

public slots:
    /** Slot called whenever a new workspace has been loaded */
    void slotDocLoaded();

signals:
    /** Notify the listeners that the number of Fixtures has changed */
    void fixturesCountChanged();

    /** Notify the listeners that the fixture tree model has changed */
    void groupsTreeModelChanged();

    /** Notify the listeners that the FixtureGroup list model has changed */
    void groupsListModelChanged();

    void newFixtureCreated(quint32 fxID, qreal x, qreal y);

    void channelValueChanged(quint32 fixtureID, quint32 channelIndex, quint8 value);

    /** Notify the listeners that channels of the specified $type should
     *  be set to the provided $value */
    void channelTypeValueChanged(int type, quint8 value);

    /** Notify the listeners that a color has been picked in the ColorTool.
     *  It emits all the possible components: RGB, White, Amber and UV */
    void colorChanged(QColor rgb, QColor wauv);

    /** Notify the listeners that the position of $type is changed
     *  to $degrees. $type can be Pan or Tilt */
    void positionTypeValueChanged(int type, int degrees);

    /** Notify the listeners that a preset value has been picked.
     *  To uniquely identify which preset channel has changed, a reference
     *  to a cached QLCChannel is emitted as well */
    void presetChanged(const QLCChannel *channel, quint8 value);

    /** Notify the listeners that the list of fixtures with gobo channels has changed */
    void goboChannelsChanged();

    /** Notify the listeners that the list of fixtures with color wheel channels has changed */
    void colorWheelChannelsChanged();

    /** Notify the listeners that the fixture names map has changed */
    void fixtureNamesMapChanged();

    /** Notify the listeners that the fixtures map has changed */
    void fixturesMapChanged();

    /** Notify the listeners that the universe filter has changed */
    void universeFilterChanged(quint32 universeFilter);

    /** Notify the listeners that the available colors changed */
    void colorsMaskChanged(int colorsMask);

private:
    /** Generic method that returns the names of the cached channels for
     *  the required $group */
    QVariantList presetsChannels(QLCChannel::Group group);

    /** Update the tree of groups/fixtures/channels */
    void updateFixtureTree(Doc *doc, TreeModel *treeModel);

    void updateColorsMap(int type, int delta);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** List of the current Fixture references in Doc */
    QList<Fixture *> m_fixtureList;
    /** Keep a map of references to the available preset channels and a related Fixture ID */
    QMap<const QLCChannel *, quint32>m_presetsCache;
    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_fixtureTree;
    /** An array-like map of the current fixture names, filtered by m_universeFilter */
    QVariantList m_fixtureNamesMap;
    /** An array-like map of the current fixtures, filtered by m_universeFilter */
    QVariantList m_fixturesMap;
    /** A filter for m_fixturesMap to restrict data to a specific universe */
    quint32 m_universeFilter;

    /** Variables to hold the maximum Pan/Tilt degrees discovered
     *  when enabling the position capability for the selected Fixtures */
    int m_maxPanDegrees;
    int m_maxTiltDegrees;

    /** Bitmask holding the colors supported by the currently selected fixtures */
    int m_colorsMask;
    /** A map of the currently available colors and their counters */
    QMap<int, int> m_colorCounters;
};

#endif // FIXTUREMANAGER_H
