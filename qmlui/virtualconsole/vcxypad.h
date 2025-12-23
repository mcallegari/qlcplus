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
    Q_PROPERTY(QVariant groupsTreeModel READ groupsTreeModel NOTIFY groupsTreeModelChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCXYPad(Doc* doc = nullptr, QObject *parent = nullptr);
    virtual ~VCXYPad();

    /** @reimp */
    QString defaultCaption();

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page);

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent);

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget);

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
    Q_INVOKABLE void removeFixture(QVariant reference);

    /** Get the fixture list for the UI */
    QVariant fixtureList() const;

    /** Returns the data model to display a tree of FixtureGroups/Fixtures */
    QVariant groupsTreeModel();

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
    /** Notify the listeners that the fixture tree model has changed */
    void groupsTreeModelChanged();
    /** Notify the listeners that the search filter has changed */
    void searchFilterChanged();

private:
    QList <XYPadFixture> m_fixtures;

    /** Reference to a ListModel representing the fixtures list for the QML UI */
    ListModel *m_fixtureList;
    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_fixtureTree;
    /** A string to filter the displayed tree items */
    QString m_searchFilter;

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimp */
    void writeDMX(MasterTimer* timer, QList<Universe*> universes);

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
    void updateFeedback();

public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXMLFixture(QXmlStreamReader &root);

    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    bool saveXMLFixture(QXmlStreamWriter *doc, XYPadFixture &fxItem);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);
};

#endif
