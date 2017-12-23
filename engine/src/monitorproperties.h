/*
  Q Light Controller Plus
  monitorproperties.h

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

#ifndef MONITORPROPERTIES_H
#define MONITORPROPERTIES_H

#include <QObject>
#include <QVector3D>
#include <QColor>
#include <QFont>
#include <QSize>
#include <QHash>

class QXmlStreamReader;
class QXmlStreamWriter;

class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCMonitorProperties "Monitor"

typedef struct
{
    QVector3D m_position;
    QVector3D m_rotation;
    QColor m_gelColor;
} FixtureItemProperties;

class MonitorProperties : public QObject
{
    Q_OBJECT

public:
    MonitorProperties();
    ~MonitorProperties() {}

    enum DisplayMode { DMX, Graphics };
    enum ChannelStyle { DMXChannels, RelativeChannels };
    enum ValueStyle { DMXValues, PercentageValues };
    enum GridUnits { Meters, Feet };
#if QT_VERSION >= 0x050500
    Q_ENUM(GridUnits)
#endif
    enum PointOfView { Undefined, TopView, FrontView, RightSideView, LeftSideView };
#if QT_VERSION >= 0x050500
    Q_ENUM(PointOfView)
#endif

    /** Get/Set the font used by the Monitor dialog UI */
    void setFont(QFont font) { m_font = font; }
    QFont font() const { return m_font; }

    /** Get/Set how to display the monitor */
    void setDisplayMode(DisplayMode mode) { m_displayMode = mode; }
    DisplayMode displayMode() const { return m_displayMode; }

    /** Get/Set how to show DMX channel indices in DMX display mode */
    void setChannelStyle(ChannelStyle style) { m_channelStyle = style; }
    ChannelStyle channelStyle() const { return m_channelStyle; }

    /** Get/Set how to show DMX channel values in DMX display mode */
    void setValueStyle(ValueStyle style) { m_valueStyle = style; }
    ValueStyle valueStyle() const { return m_valueStyle; }

    /** Get/Set the size of the grid in Graphics display mode */
    void setGridSize(QVector3D size) { m_gridSize = size; }
    QVector3D gridSize() const { return m_gridSize; }

    /** Get/Set the grid measurement units to use in Graphics display mode */
    void setGridUnits(GridUnits units) { m_gridUnits = units; }
    GridUnits gridUnits() const { return m_gridUnits; }

    /** Get/Set the point of view to render the Graphics view */
    void setPointOfView(PointOfView pov);
    PointOfView pointOfView() const { return m_pointOfView; }

    /** Remove a Fixture entry from the Monitor map */
    void removeFixture(quint32 fid);

    /** Returns if the Fixture with ID $fid is present in the monitor map */
    bool hasFixturePosition(quint32 fid) { return m_fixtureItems.contains(fid); }

    /** Get/Set the position of a Fixture with ID $fid */
    void setFixturePosition(quint32 fid, QVector3D pos);
    QVector3D fixturePosition(quint32 fid) const { return m_fixtureItems[fid].m_position; }

    /** Get/Set the rotation of a Fixture with ID $fid */
    void setFixtureRotation(quint32 fid, QVector3D degrees);
    QVector3D fixtureRotation(quint32 fid) const { return m_fixtureItems[fid].m_rotation; }

    /** Get/Set the color of a gel used to render a Fixture with ID $fid */
    void setFixtureGelColor(quint32 fid, QColor col);
    QColor fixtureGelColor(quint32 fid) const { return m_fixtureItems[fid].m_gelColor; }

    /** Get/Set the Fixture labels visibility status */
    void setLabelsVisible(bool visible) { m_showLabels = visible; }
    bool labelsVisible() const { return m_showLabels; }

    /** Get/Set a background image to be displayed in Graphics mode */
    void setCommonBackgroundImage(QString filename) { m_commonBackgroundImage = filename; }
    QString commonBackgroundImage() const { return m_commonBackgroundImage; }

    /** Set a picture found at $path to be displayed when $fid is started */
    void setCustomBackgroundItem(quint32 fid, QString path) { m_customBackgroundImages[fid] = path; }

    /** Helper method to set a whole list of custom pictures mapped by Function IDs */
    void setCustomBackgroundList(QHash<quint32, QString>list) { m_customBackgroundImages = list; }

    /** Reset any previously set background pictures list */
    void resetCustomBackgroundList() { m_customBackgroundImages.clear(); }

    /** Returns the map of custom background pictures organized as Function ID/Picture path */
    QHash<quint32, QString> customBackgroundList() const { return m_customBackgroundImages; }

    /** Returns the path of a custom picture set for a Function with $id */
    QString customBackground(quint32 fid);

    /** Get/Set the properties of a Monitor Fixture with ID $fid */
    FixtureItemProperties fixtureProperties(quint32 fid) const { return m_fixtureItems[fid]; }
    void setFixtureProperties(quint32 fid, FixtureItemProperties props) { m_fixtureItems[fid] = props; }

    /** Get a list of Fixture IDs currently set in the Monitor */
    QList <quint32> fixtureItemsID() const { return m_fixtureItems.keys(); }

    /** Reset all the Monitor properties */
    void reset();

private:
    QFont m_font;
    DisplayMode m_displayMode;
    ChannelStyle m_channelStyle;
    ValueStyle m_valueStyle;
    QVector3D m_gridSize;
    GridUnits m_gridUnits;
    PointOfView m_pointOfView;
    bool m_showLabels;
    QString m_commonBackgroundImage;
    QHash <quint32, QString> m_customBackgroundImages;
    QHash <quint32, FixtureItemProperties> m_fixtureItems;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Load the Monitor properties from the given XML node.
     *
     * @param root An XML subtree containing the Monitor properties
     * @return true if the properties were loaded successfully, otherwise false
     */
    bool loadXML(QXmlStreamReader &root, const Doc* mainDocument);

    /**
     * Save the Monitor properties into an XML document, under the given
     * XML element (tag).
     *
     * @param doc The master XML document to save to.
     * @param wksp_root The workspace root element
     */
    bool saveXML(QXmlStreamWriter *doc, const Doc * mainDocument) const;
};

/** @} */

#endif // MONITORPROPERTIES_H
