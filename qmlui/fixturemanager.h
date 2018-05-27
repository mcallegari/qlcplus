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
#include <QDir>

#include "scenevalue.h"

class Doc;
class Fixture;
class TreeModel;
class TreeModelItem;
class FixtureGroup;
class ColorFilters;
class MonitorProperties;

class FixtureManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int fixturesCount READ fixturesCount NOTIFY fixturesCountChanged)
    Q_PROPERTY(QQmlListProperty<Fixture> fixtures READ fixtures)
    Q_PROPERTY(QVariantList fixturesMap READ fixturesMap NOTIFY fixturesMapChanged)
    Q_PROPERTY(QVariantList fixtureNamesMap READ fixtureNamesMap NOTIFY fixtureNamesMapChanged)
    Q_PROPERTY(QVariant groupsTreeModel READ groupsTreeModel NOTIFY groupsTreeModelChanged)
    Q_PROPERTY(quint32 universeFilter READ universeFilter WRITE setUniverseFilter NOTIFY universeFilterChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)
    Q_PROPERTY(quint32 itemID READ itemID WRITE setItemID NOTIFY itemIDChanged)

    Q_PROPERTY(QVariantList goboChannels READ goboChannels NOTIFY goboChannelsChanged)
    Q_PROPERTY(QVariantList colorWheelChannels READ colorWheelChannels NOTIFY colorWheelChannelsChanged)
    Q_PROPERTY(QVariantList shutterChannels READ shutterChannels NOTIFY shutterChannelsChanged)
    Q_PROPERTY(int colorsMask READ colorsMask NOTIFY colorsMaskChanged)

    Q_PROPERTY(QStringList colorFiltersList READ colorFiltersList NOTIFY colorFiltersListChanged)
    Q_PROPERTY(int colorFilterIndex READ colorFilterIndex WRITE setColorFilterIndex NOTIFY colorFilterIndexChanged)
    Q_PROPERTY(ColorFilters *selectedFilters READ selectedFilters NOTIFY selectedFiltersChanged)

public:
    FixtureManager(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~FixtureManager();

    /** Get/Set the Universe index used to filter Fixture lists/tree */
    quint32 universeFilter() const;
    void setUniverseFilter(quint32 universeFilter);

    /** Get/Set a string to filter Group/Fixture/Channel names */
    QString searchFilter() const;
    void setSearchFilter(QString searchFilter);

    /** Get/Set a generic ID for Universe/Group/Fixture editing/info */
    quint32 itemID() const;
    void setItemID(quint32 itemID);

    /** Returns a data structure with all the information of
     *  the Fixtures of the Universe with the specified $id */
    Q_INVOKABLE QVariantList universeInfo(quint32 id);

    /** Returns a data structure with all the information of
     *  a Fixture with the specified $id */
    Q_INVOKABLE QVariant fixtureInfo(quint32 id);

signals:
    /** Notify the listeners that the universe filter has changed */
    void universeFilterChanged(quint32 universeFilter);

    /** Notify the listeners that the search filter has changed */
    void searchFilterChanged();

    void itemIDChanged(quint32 itemID);

public slots:
    /** Slot called whenever a new workspace has been loaded */
    void slotDocLoaded();

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the Doc Monitor properties */
    MonitorProperties *m_monProps;
    /** A filter for m_fixturesMap to restrict data to a specific universe */
    quint32 m_universeFilter;
    /** A string to filter the displayed tree items */
    QString m_searchFilter;
    /** A generic ID for Universe/Group/Fixture editing/info */
    quint32 m_itemID;

    QVariantList m_universeInfo;

    /*********************************************************************
     * Fixtures
     *********************************************************************/
public:
    enum
    {
        ShowCheckBoxes  = (1 << 0),
        ShowGroups      = (1 << 1),
        ShowChannels    = (1 << 2),
        ShowHeads       = (1 << 3),
        ShowFlags       = (1 << 4),
        ShowCanFade     = (1 << 5),
        ShowPrecedence  = (1 << 6),
        ShowModifier    = (1 << 7)
    };

    enum PrecedenceType
    {
        AutoHTP = 0,
        AutoLTP,
        ForcedHTP,
        ForcedLTP
    };
    Q_ENUM(PrecedenceType)

    enum
    {
        GroupMatch   = (1 << 0),
        FixtureMatch = (1 << 1),
        HeadMatch    = (1 << 2),
        ChannelMatch = (1 << 3)
    };

    /** Returns a constant value for an invalid Fixture ID */
    Q_INVOKABLE quint32 invalidFixture() const;

    /** Returns the Fixture ID at the provided $universeAddress */
    Q_INVOKABLE quint32 fixtureForAddress(quint32 universeAddress);

    Q_INVOKABLE bool addFixture(QString manuf, QString model, QString mode, QString name,
                                int uniIdx, int address, int channels, int quantity, quint32 gap,
                                qreal xPos, qreal yPos);

    Q_INVOKABLE bool moveFixture(quint32 fixtureID, quint32 newAddress);

    /** Delete some existing Fixtures with IDs provided by $IDList */
    Q_INVOKABLE bool deleteFixtures(QVariantList IDList);

    /** Returns the number of fixtures currently loaded in the project */
    int fixturesCount();

    /** Returns a QML-readable list of references to Fixture classes */
    QQmlListProperty<Fixture> fixtures();

    /** Returns the data model to display a tree of Fixture Groups/Fixtures */
    QVariant groupsTreeModel();

    /** Enable/Disable the fixture/channel properties editing mode */
    Q_INVOKABLE void enablePropertyEditing(bool enable);

    Q_INVOKABLE void setItemRoleData(int itemID, int index, QString role, QVariant value);

    static void addFixtureNode(Doc *doc, TreeModel *treeModel, Fixture *fixture, QString basePath, quint32 nodeSubID,
                               int &matchMask, QString searchFilter = QString(), int showFlags = ShowGroups | ShowHeads,
                               QList<SceneValue> checkedChannels = QList<SceneValue>());

    static void addFixtureGroupTreeNode(Doc *doc, TreeModel *treeModel, FixtureGroup *group,
                                        QString searchFilter = QString(), int showFlags = ShowGroups | ShowHeads,
                                        QList<SceneValue> checkedChannels = QList<SceneValue>());

    /** Update the tree of groups/fixtures/channels */
    static void updateGroupsTree(Doc *doc, TreeModel *treeModel, QString searchFilter = QString(),
                                 int showFlags = ShowGroups | ShowHeads,
                                 QList<SceneValue> checkedChannels = QList<SceneValue>());

    /** Return the type as string of the Fixture with ID $fixtureID */
    Q_INVOKABLE QString fixtureIcon(quint32 fixtureID);

    /** Generic helper to retrieve a channel icon resource as string, from
     *  the provided Fixture ID $fxID and channel index $chIdx */
    Q_INVOKABLE QString channelIcon(quint32 fxID, quint32 chIdx);

signals:
    /** Notify the listeners that the number of Fixtures has changed */
    void fixturesCountChanged();

    /** Notify the listeners that a fixture has been created at position x,y,z */
    void newFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z);

    /** Notify the listeners that a fixture has been deleted */
    void fixtureDeleted(quint32 fxID);

    void fixtureFlagsChanged(quint32 itemID, quint32 flags);

private:
    /** Comparison method to sort a Fixture list by DMX address */
    static bool compareFixtures(Fixture *left, Fixture *right);

private:
    /** List of the current Fixture references in Doc */
    QList<Fixture *> m_fixtureList;
    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_fixtureTree;
    /** Current flags being used for filling the tree data */
    int m_treeShowFlags;

    /*********************************************************************
     * Fixture groups
     *********************************************************************/
public:
    /** Add a list of fixture IDs to a new fixture group */
    void addFixturesToNewGroup(QList<quint32>fxList);

    /** Delete some existing Fixture Groups with IDs provided by $IDList */
    Q_INVOKABLE bool deleteFixtureGroups(QVariantList IDList);

signals:
    /** Notify the listeners that the fixture tree model has changed */
    void groupsTreeModelChanged();

public slots:
    /** Slot called whenever a new fixture groups has been created */
    void slotFixtureGroupAdded(quint32 id);

    /*********************************************************************
     * RGB Panel creation
     *********************************************************************/

public:
    enum Orientation
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };
    Q_ENUM(Orientation)

    enum Type
    {
        Snake,
        ZigZag
    };
    Q_ENUM(Type)

    enum Direction
    {
        Horizontal,
        Vertical
    };
    Q_ENUM(Direction)

    bool addRGBPanel(QString name, qreal xPos, qreal yPos);

    /*********************************************************************
     * Universe Grid Editing
     *********************************************************************/
public:
    /** Returns a list of the universe indices occupied by a Fixture
        at the requested $address */
    Q_INVOKABLE QVariantList fixtureSelection(quint32 address);

    /** Returns a list of fixture names for representation in a GridEditor QML component */
    QVariantList fixtureNamesMap();

    /** Returns data for representation in a GridEditor QML component */
    QVariantList fixturesMap();

signals:
    /** Notify the listeners that the fixture names map has changed */
    void fixtureNamesMapChanged();

    /** Notify the listeners that the fixtures map has changed */
    void fixturesMapChanged();

private:
    /** An array-like map of the current fixture names, filtered by m_universeFilter */
    QVariantList m_fixtureNamesMap;
    /** An array-like map of the current fixtures, filtered by m_universeFilter */
    QVariantList m_fixturesMap;

    /*********************************************************************
     * Color filters
     *********************************************************************/
public:
    /** Returns a list of the currently installed color filters XMLs */
    QStringList colorFiltersList();

    /** Create a new empty color filters file in the user folder */
    Q_INVOKABLE void createColorFilters();

    /** Get/Set the currently selected color filter index */
    int colorFilterIndex() const;
    void setColorFilterIndex(int colorFilterIndex);

    /** Return a refererence to the currently selected color filters */
    ColorFilters *selectedFilters();

protected:
    /** Returns a QDir for the system color filters location */
    QDir systemColorFiltersDirectory();

    /** Returns a QDir for the user color filters location */
    QDir userColorFiltersDirectory();

    /** Scan the given $dir for color filters file and loads them.
     *  $user is used to mark a filter as user or system type */
    bool loadColorFilters(const QDir& dir, bool user = false);

    /** Empty the color filters list and destroy them */
    void resetColorFilters();

signals:
    void colorFiltersListChanged();
    void colorFilterIndexChanged(int colorFilterIndex);
    void selectedFiltersChanged();

private:
    QList<ColorFilters *> m_colorFilters;
    int m_colorFilterIndex;

    /*********************************************************************
     * Channel capabilities
     *********************************************************************/
public:
    /** Wrapper method to emit a generic Fixture channel value change */
    Q_INVOKABLE void setChannelValue(quint32 fixtureID, quint32 channelIndex, quint8 value);

    /** Wrapper methods to emit a signal to listeners interested in changes of
     *  channel values per capability */
    Q_INVOKABLE void setIntensityValue(quint8 value);
    Q_INVOKABLE void setColorValue(quint8 red, quint8 green, quint8 blue,
                                   quint8 white, quint8 amber, quint8 uv);
    Q_INVOKABLE void setPanValue(int degrees);
    Q_INVOKABLE void setTiltValue(int degrees);
    Q_INVOKABLE void setPresetValue(quint32 fixtureID, int chIndex, quint8 value);
    Q_INVOKABLE void setBeamValue(quint8 value);

    /**
     * @brief setFixtureCapabilities
     * @param itemID the preview item ID
     * @param headIndex index of the head to consider or -1 to not considering it at all
     * @param enable used to increment/decrement the UI tools counters
     * @return A multihash containg the fixture capabilities by channel type
     */
    QMultiHash<int, SceneValue> getFixtureCapabilities(quint32 itemID, int headIndex, bool enable);

    /** Reset any previously elapsed capability */
    void resetCapabilities();

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

    /** Returns the names of the currently selected fixtures with shutter channels.
     *  The names are in the format: Product - Channel name */
    QVariantList shutterChannels();

    /** Returns the list of QLCCapability in QVariant format for
     *  the channel cached at the given index */
    Q_INVOKABLE QVariantList presetCapabilities(quint32 fixtureID, int chIndex);

    /** Returns the currently available colors as a bitmask */
    int colorsMask() const;

signals:
    /** Notify the listeners that $value of $channelIndex of $fixtureID has changed */
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

    /** Notify the listeners that the list of fixtures with shutter channels has changed */
    void shutterChannelsChanged();

    /** Notify the listeners that the available colors changed */
    void colorsMaskChanged(int colorsMask);

private:
    /** Generic method that returns the names of the cached channels for
     *  the required $group */
    QVariantList presetsChannels(QLCChannel::Group group);

    void updateColorsMap(int type, int delta);

    void updateCapabilityCounter(bool update, QString capName, int delta);
    void setCapabilityCounter(QString capName, int value);

private:
    /** Keep a map of references to the available preset channels and a related Fixture ID */
    QMap<const QLCChannel *, quint32>m_presetsCache;

    /** Variables to hold the maximum Pan/Tilt degrees discovered
     *  when enabling the position capability for the selected Fixtures */
    int m_maxPanDegrees;
    int m_maxTiltDegrees;

    double m_minBeamDegrees;
    double m_maxBeamDegrees;

    /** Bitmask holding the colors supported by the currently selected fixtures */
    int m_colorsMask;
    /** A map of the currently available colors and their counters */
    QMap<int, int> m_colorCounters;
};

#endif // FIXTUREMANAGER_H
