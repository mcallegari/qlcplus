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

signals:
    void invertedAppearanceChanged();
    void displayModeChanged();
    void currentPositionChanged();
    void horizontalRangeChanged();
    void verticalRangeChanged();

private:
    bool m_invertedAppearance;
    DisplayMode m_displayMode;

    QPointF m_currentPosition;
    QPointF m_horizontalRange;
    QPointF m_verticalRange;
    bool m_positionChanged;

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
    } XYPadFixture;

    /** Add a Fixture Group or a Universe to this XY Pad */
    Q_INVOKABLE void addGroup(QVariant reference);

    /** Add a Fixture to this XY Pad */
    Q_INVOKABLE void addFixture(QVariant reference);

    /** Add a single head to this XY Pad */
    Q_INVOKABLE void addHead(int fixtureID, int headIndex);

    /** Remove a Fixture from this XY Pad */
    Q_INVOKABLE void removeHeads(QVariantList heads);

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

    /** @reimp */
    bool loadXML(QXmlStreamReader &root) override;

    bool saveXMLFixture(QXmlStreamWriter *doc, const XYPadFixture &fxItem) const;

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc) const override;
};

#endif
