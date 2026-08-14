/*
  Q Light Controller Plus
  vcxypad.h

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

#ifndef VCXYPAD_H
#define VCXYPAD_H

#include <QVector3D>
#include <QHash>

#include "vcwidget.h"
#include "dmxsource.h"
#include "grouphead.h"

#define KXMLQLCVCXYPad  QStringLiteral("XYPad")

class ListModel;
class TreeModel;

class VCXYPad : public VCWidget, public DMXSource
{
    Q_OBJECT

    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance NOTIFY invertedAppearanceChanged FINAL)
    Q_PROPERTY(DisplayMode displayMode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged FINAL)

    Q_PROPERTY(QPointF currentPosition READ currentPosition WRITE setCurrentPosition NOTIFY currentPositionChanged FINAL)
    Q_PROPERTY(QPointF horizontalRange READ horizontalRange WRITE setHorizontalRange NOTIFY horizontalRangeChanged FINAL)
    Q_PROPERTY(QPointF verticalRange READ verticalRange WRITE setVerticalRange NOTIFY verticalRangeChanged FINAL)

    Q_PROPERTY(bool floorControl READ floorControl WRITE setFloorControl NOTIFY floorControlChanged FINAL)
    Q_PROPERTY(QVector3D floorPosition READ floorPosition WRITE setFloorPosition NOTIFY floorPositionChanged FINAL)
    Q_PROPERTY(QVector3D floorSize READ floorSize NOTIFY floorSizeChanged FINAL)
    Q_PROPERTY(QRectF floorRangeArea READ floorRangeArea NOTIFY floorRangeAreaChanged FINAL)
    Q_PROPERTY(qreal floorHeightMax READ floorHeightMax CONSTANT)
    Q_PROPERTY(qreal floorHeightStep READ floorHeightStep CONSTANT)

    Q_PROPERTY(QVariant fixtureList READ fixtureList NOTIFY fixtureListChanged)
    Q_PROPERTY(QVariantList fixturePositions READ fixturePositions NOTIFY fixturePositionsChanged)
    Q_PROPERTY(QVariant groupsTreeModel READ groupsTreeModel NOTIFY groupsTreeModelChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)
    Q_PROPERTY(QVariantList presetsList READ presetsList NOTIFY presetsListChanged)
    Q_PROPERTY(int activePresetId READ activePresetId NOTIFY activePresetIdChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCXYPad(Doc* doc = nullptr, QObject *parent = nullptr);
    virtual ~VCXYPad();

    /** @reimp */
    QString defaultCaption() const override;

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page) override;

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent) override;

    /** @reimp */
    QString propertiesResource() const override;
    QString presetsResource() const override;
    bool supportsPresets() const override;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent) const override;

    /** @reimp */
    void remapChannels(const QMap<SceneValue, SceneValue> &remapMap) override;

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget) override;

private:
    FunctionParent functionParent() const;

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    bool invertedAppearance() const;
    void setInvertedAppearance(bool newInvertedAppearance);

    enum DisplayMode
    {
        Percentage = 0,
        Degrees,
        DMX
    };
    Q_ENUM(DisplayMode)

    /** Get/Set the display mode of fixtures Pan anf Tilt range */
    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;

    /** Get/Set the current cursor position in the XY area */
    QPointF currentPosition() const;
    void setCurrentPosition(QPointF newCurrentPosition);

    /** Get/Set the range window horizontal range */
    QPointF horizontalRange() const;
    void setHorizontalRange(QPointF newHorizontalRange);

    /** Get/Set the range window vertical range */
    QPointF verticalRange() const;
    void setVerticalRange(QPointF newVerticalRange);

    /** Get/Set the "floor control" mode. When enabled, the pad area no longer
     *  represents the fixtures Pan/Tilt degrees, but the stage floor on the
     *  X/Z plane. Moving the cursor makes the enabled fixtures point at the
     *  matching floor coordinate, and a side fader raises the target height
     *  on the Y axis. */
    bool floorControl() const;
    void setFloorControl(bool enable);

    /** Get/Set the currently targeted 3D point, in metres.
     *  X/Z are relative to the environment origin (0,0 = stage front-left
     *  corner), Y is the height above the floor. */
    QVector3D floorPosition() const;
    void setFloorPosition(QVector3D newFloorPosition);

    /** Return the environment size in metres, used to map the pad area
     *  to the stage floor in floor control mode */
    QVector3D floorSize() const;

    /** Return the reachable portion of the stage floor in metres, obtained
     *  by mapping the range window onto the environment size. X/width follow
     *  the horizontal range, Y/height the vertical one. */
    QRectF floorRangeArea() const;

    /** Boundaries of the floor control height fader, in metres */
    qreal floorHeightMax() const;
    qreal floorHeightStep() const;

signals:
    void invertedAppearanceChanged();
    void displayModeChanged();
    void currentPositionChanged();
    void horizontalRangeChanged();
    void verticalRangeChanged();
    void floorControlChanged();
    void floorPositionChanged();
    void floorSizeChanged();
    void floorRangeAreaChanged();

private:
    bool m_invertedAppearance;
    DisplayMode m_displayMode;

    QPointF m_currentPosition;
    QPointF m_horizontalRange;
    QPointF m_verticalRange;
    bool m_positionChanged;

    bool m_floorControl;
    QVector3D m_floorPosition;

    /** Flag raised while a position change is driven by external input, so
     *  that setCurrentPosition doesn't echo a feedback straight back to the
     *  controller. Any other position change (UI drag, preset, undo) will
     *  re-sync the controllers instead. */
    bool m_handlingExternalInput = false;

    /** Cached MSB/LSB values for an
     *  efficient DMX computation */
    quint16 m_x16 = 0;
    quint16 m_y16 = 0;
    quint16 m_lastX16 = 0xFFFF;
    quint16 m_lastY16 = 0xFFFF;

    /*************************************************************************
     * Fixtures
     *************************************************************************/
public:
    typedef struct
    {
        /** X-Axis */
        qreal m_xMin; //!< start of pan range; 0.0 <= m_xMin <= 1.0; default: 0.0
        qreal m_xMax; //!< end of pan range; 0.0 <= m_xMax <= 1.0; default: 1.0
        bool m_xReverse; //!< pan reverse; default: false

        quint32 m_xLSB; //!< fine pan channel (relative address)
        quint32 m_xMSB; //!< coarse pan channel (relative address)
        qreal m_xOffset; //!< precomputed value for writeDMX/readDMX
        qreal m_xRange; //!< precomputed value for writeDMX/readDMX

        /** Y-Axis */
        qreal m_yMin; //!< start of tilt range; 0.0 <= m_yMin <= 1.0; default: 0.0
        qreal m_yMax; //!< end of tilt range; 0.0 <= m_yMax <= 1.0; default: 1.0
        bool m_yReverse; //!< tilt reverse; default: false

        quint32 m_yLSB; //!< fine tilt channel (relative address)
        quint32 m_yMSB; //!< coarse tilt channel (relative address)
        qreal m_yOffset; //!< precomputed value for writeDMX/readDMX
        qreal m_yRange; //!< precomputed value for writeDMX/readDMX

        /** Flag to enable/disable this fixture at runtime */
        bool m_enabled;
        GroupHead m_head;
        quint32 m_universe;
        quint32 m_fixtureAddress;

        /** ID of the FixtureGroup this entry represents. When valid, the
         *  entry is a whole group rather than a single head, and m_head is
         *  unused: the group is resolved to its heads at DMX write time, so
         *  that adding/removing fixtures from the group is picked up. */
        quint32 m_groupID;
    } XYPadFixture;

    /** Add a Fixture Group or a Universe to this XY Pad.
     *  A Universe is expanded to its fixtures, while a Fixture Group is kept
     *  as a single entry and also gets a matching FixtureGroup preset. */
    Q_INVOKABLE void addGroup(QVariant reference);

    /** Add a Fixture to this XY Pad */
    Q_INVOKABLE void addFixture(QVariant reference);

    /** Add a single head to this XY Pad */
    Q_INVOKABLE void addHead(int fixtureID, int headIndex);

    /** Remove a Fixture from this XY Pad */
    Q_INVOKABLE void removeHeads(QVariantList heads);

    /** Return a map with the current Pan/Tilt range of the given $heads.
     *  The displayed min/max values follow the current display mode and,
     *  for a mix of fixtures, the maximum allowed value is the smallest
     *  among the selection. */
    Q_INVOKABLE QVariantMap headsRangeInfo(QVariantList heads);

    /** Apply the given Pan/Tilt range (expressed in the current display
     *  mode units) and reverse flags to the given $heads */
    Q_INVOKABLE void setHeadsRange(QVariantList heads, int xMin, int xMax, bool xReverse,
                                   int yMin, int yMax, bool yReverse);

    /** Add presets */
    Q_INVOKABLE int addPositionPreset();
    Q_INVOKABLE int addFunctionPreset(quint32 functionID);
    Q_INVOKABLE int addFixtureGroupPreset(QVariant reference);
    Q_INVOKABLE int addFixtureGroupHeadPreset(int fixtureID, int headIndex);

    /** Remove/reorder/edit presets */
    Q_INVOKABLE void removePreset(quint8 presetId);
    Q_INVOKABLE int movePresetUp(quint8 presetId);
    Q_INVOKABLE int movePresetDown(quint8 presetId);
    Q_INVOKABLE void setPresetName(quint8 presetId, QString name);
    Q_INVOKABLE void applyPreset(quint8 presetId);

    /** Get the fixture list for the UI */
    QVariant fixtureList() const;

    /** Returns the data model to display a tree of FixtureGroups/Fixtures */
    QVariant groupsTreeModel();
    QVariantList fixturePositions() const;

    QVariantList presetsList() const;
    int activePresetId() const;

    /** Get/Set a string to filter Group/Fixture/Channel names */
    QString searchFilter() const;
    void setSearchFilter(QString searchFilter);

protected:
    void initXYFixtureItem(XYPadFixture &fixture);
    void computeRange(XYPadFixture &fixture);
    void updateFixtureList();

    /** Returns true if a group with the given $groupID is already in the pad */
    bool hasGroup(quint32 groupID) const;

    /** Resolve an entry to the heads it drives: a single head for a fixture
     *  entry, the current member heads for a group entry */
    QList<GroupHead> entryHeads(const XYPadFixture &fixture) const;

signals:
    /** Notify the listeners that the fixture list model has changed */
    void fixtureListChanged();
    /** Notify listeners that fixture preview positions changed */
    void fixturePositionsChanged();
    /** Notify the listeners that the fixture tree model has changed */
    void groupsTreeModelChanged();
    /** Notify the listeners that the search filter has changed */
    void searchFilterChanged();
    /** Notify listeners that presets data changed */
    void presetsListChanged();
    /** Notify listeners that the active preset changed */
    void activePresetIdChanged();

private:
    QList <XYPadFixture> m_fixtures;
    QVariantList m_fixturePositions;

    /** Reference to a ListModel representing the fixtures list for the QML UI */
    ListModel *m_fixtureList;
    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_fixtureTree;
    /** A string to filter the displayed tree items */
    QString m_searchFilter;

    /*********************************************************************
     * Presets
     *********************************************************************/
private:
    QList<class VCXYPadPreset*> presets() const;
    class VCXYPadPreset *findPreset(quint8 presetId) const;
    void refreshPresetExternalControls();
    void clearPresets();
    void addPresetInternal(class VCXYPadPreset *preset);
    bool hasHead(const GroupHead &head) const;
    QList<GroupHead> uniqueHeadsInPad(const QList<GroupHead> &heads) const;

    /** Resolve the heads selected by a Fixture Group preset. A preset that
     *  references a group resolves it every time, so that changes to the
     *  group membership are picked up; otherwise the stored head list is
     *  returned as-is. */
    QList<GroupHead> presetHeads(const class VCXYPadPreset *preset) const;
    bool sceneHasPanTilt(quint32 functionID) const;
    bool activatePreset(VCXYPadPreset *preset);
    void deactivatePreset(VCXYPadPreset *preset);
    void setActivePresetId(int presetId);

private:
    quint8 m_lastAssignedPresetId;
    QList<class VCXYPadPreset*> m_presets;
    int m_activePresetId;

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimp */
    void writeDMX(MasterTimer* timer, QList<Universe*> universes) override;

private:
    void updateChannel(FadeChannel *fc, uchar value);

    /** Write the Pan/Tilt values that make the enabled fixtures point at
     *  the current floor position. Used when floor control is enabled. */
    void writeDMXFloor(QList<Universe *> universes);

    /** On fixtures with more than 360° of Pan travel, the same direction can
     *  be reached at several Pan angles (e.g. 30° and 390° on a 540° head).
     *  Pick the one closest to where the fixture is already pointing, so that
     *  dragging across the stage keeps moving the head the short way instead
     *  of sweeping it back through the centre. Tilt is never altered. */
    qreal resolvePanDegrees(const Fixture *fixture, qreal panDeg);

    /** Last Pan angle (degrees) commanded per fixture in floor mode, used to
     *  resolve the wrap-around ambiguity above */
    QHash<quint32, qreal> m_lastFloorPan;

public slots:
    void slotUniverseWritten(quint32 idx, const QByteArray& universeData);

private:
    /** Map used to lookup a GenericFader instance for a Universe ID */
    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** @reimp */
    void updateFeedback() override;

public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value) override;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXMLFixture(QXmlStreamReader &root);
    bool loadXMLGroup(QXmlStreamReader &root);

    /** @reimp */
    bool loadXML(QXmlStreamReader &root) override;

    bool saveXMLFixture(QXmlStreamWriter *doc, const XYPadFixture &fxItem) const;

private:
    /** Read the Axis children of a Fixture/Group node into $fxItem */
    void loadXMLAxes(QXmlStreamReader &root, XYPadFixture &fxItem);

    /** Write an Axis element, but only if the axis differs from the
     *  default full range (0.0 - 1.0, not reversed) */
    void saveXMLAxis(QXmlStreamWriter *doc, const QString &axisID,
                     qreal min, qreal max, bool reverse) const;

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc) const override;
};

#endif
