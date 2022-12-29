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
#include <QMap>

class QXmlStreamReader;
class QXmlStreamWriter;

class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCMonitorProperties QString("Monitor")

typedef struct
{
    QVector3D m_position;       ///< 3D item position
    QVector3D m_rotation;       ///< 3D item rotation
    QVector3D m_scale;          ///< 3D item scale
    QString m_name;             ///< Fixture/Item Custom name
    QString m_resource;         ///< Generic: source file
    QColor m_color;             ///< Generic: item color, Fixture: gel color
    quint32 m_flags;            ///< Item flags as specified in the ItemsFlags enum
} PreviewItem;

typedef struct
{
    PreviewItem m_baseItem;                 ///< Base fixture item properties
    QMap<quint32, PreviewItem> m_subItems;  ///< Map of the heads/linked fixtures
} FixturePreviewItem;

class MonitorProperties : public QObject
{
    Q_OBJECT

public:
    MonitorProperties();
    ~MonitorProperties() {}

    enum DisplayMode { DMX, Graphics };
    enum ChannelStyle { DMXChannels, RelativeChannels };
    enum ValueStyle { DMXValues, PercentageValues };

    /** Get/Set the font used by the Monitor dialog UI */
    inline void setFont(QFont font) { m_font = font; }
    inline QFont font() const { return m_font; }

    /** Get/Set how to display the monitor */
    inline void setDisplayMode(DisplayMode mode) { m_displayMode = mode; }
    inline DisplayMode displayMode() const { return m_displayMode; }

    /** Get/Set how to show DMX channel indices in DMX display mode */
    inline void setChannelStyle(ChannelStyle style) { m_channelStyle = style; }
    inline ChannelStyle channelStyle() const { return m_channelStyle; }

    /** Get/Set how to show DMX channel values in DMX display mode */
    inline void setValueStyle(ValueStyle style) { m_valueStyle = style; }
    inline ValueStyle valueStyle() const { return m_valueStyle; }

    /** Reset all the Monitor properties */
    void reset();

private:
    QFont m_font;
    DisplayMode m_displayMode;
    ChannelStyle m_channelStyle;
    ValueStyle m_valueStyle;

    /********************************************************************
     * Environment
     ********************************************************************/
public:
    enum GridUnits { Meters, Feet };
#if QT_VERSION >= 0x050500
    Q_ENUM(GridUnits)
#endif
    enum PointOfView { Undefined, TopView, FrontView, RightSideView, LeftSideView };
#if QT_VERSION >= 0x050500
    Q_ENUM(PointOfView)
#endif
    enum StageType { StageSimple, StageBox, StageRock, StageTheatre };

    /** Get/Set the size of the grid in 2D display mode */
    inline void setGridSize(QVector3D size) { m_gridSize = size; }
    inline QVector3D gridSize() const { return m_gridSize; }

    /** Get/Set the grid measurement units to use in 2D display mode */
    inline void setGridUnits(GridUnits units) { m_gridUnits = units; }
    inline GridUnits gridUnits() const { return m_gridUnits; }

    /** Get/Set the point of view to render the 2D preview */
    void setPointOfView(PointOfView pov);
    inline PointOfView pointOfView() const { return m_pointOfView; }

    /** Get/Set the type of stage to render in the 3D preview */
    inline void setStageType(StageType type) { m_stageType = type; }
    inline StageType stageType() const { return m_stageType; }

private:
    QVector3D m_gridSize;
    GridUnits m_gridUnits;
    PointOfView m_pointOfView;
    StageType m_stageType;

    /********************************************************************
     * Items flags
     ********************************************************************/
public:
    enum ItemFlags
    {
        HiddenFlag          = (1 << 0),
        InvertedPanFlag     = (1 << 1),
        InvertedTiltFlag    = (1 << 2),
        MeshZUpFlag         = (1 << 3)
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(ItemFlags)
#endif

    /********************************************************************
     * Fixture items
     ********************************************************************/
public:
    /** Get/Set the Fixture labels visibility status */
    inline void setLabelsVisible(bool visible) { m_showLabels = visible; }
    inline bool labelsVisible() const { return m_showLabels; }

    /** Remove a Fixture entry from the Monitor map by Fixture ID */
    void removeFixture(quint32 fid);

    /** Remove a Fixture entry from the Monitor map by item ID */
    void removeFixture(quint32 fid, quint16 head, quint16 linked);

    quint32 fixtureSubID(quint32 headIndex, quint32 linkedIndex) const;
    quint16 fixtureHeadIndex(quint32 mapID) const;
    quint16 fixtureLinkedIndex(quint32 mapID) const;

    /** Returns true if the Fixture with ID $fid is present in the monitor map */
    inline bool containsFixture(quint32 fid) { return m_fixtureItems.contains(fid); }

    /** Returns true if the provided Fixture ID, head index and linked index are in the map */
    bool containsItem(quint32 fid, quint16 head, quint16 linked);

    /** Get/Set the position of a Fixture with with the given $fid, $head and $linked index */
    void setFixturePosition(quint32 fid, quint16 head, quint16 linked, QVector3D pos);
    QVector3D fixturePosition(quint32 fid, quint16 head, quint16 linked) const;

    /** Get/Set the rotation of a Fixture with with the given $fid, $head and $linked index */
    void setFixtureRotation(quint32 fid, quint16 head, quint16 linked, QVector3D degrees);
    QVector3D fixtureRotation(quint32 fid, quint16 head, quint16 linked) const;

    /** Get/Set the color of a gel used to render a Fixture with with the given $fid, $head and $linked index */
    void setFixtureGelColor(quint32 fid, quint16 head, quint16 linked, QColor col);
    QColor fixtureGelColor(quint32 fid, quint16 head, quint16 linked) const;

    /** Get/Set the name of a Fixture with with the given $fid, $head and $linked index */
    void setFixtureName(quint32 fid, quint16 head, quint16 linked, QString name);
    QString fixtureName(quint32 fid, quint16 head, quint16 linked) const;

    /** Get/Set the flags of a Fixture with with the given $fid, $head and $linked index */
    void setFixtureFlags(quint32 fid, quint16 head, quint16 linked, quint32 flags);
    quint32 fixtureFlags(quint32 fid, quint16 head, quint16 linked) const;

    /** Get/Set all the Fixture item properties of a Fixture with ID $fid */
    inline FixturePreviewItem fixtureProperties(quint32 fid) const { return m_fixtureItems[fid]; }
    inline void setFixtureProperties(quint32 fid, FixturePreviewItem props) { m_fixtureItems[fid] = props; }

    /** Get/Set a single Fixture item property with the given $fid, $head and $linked index */
    PreviewItem fixtureItem(quint32 fid, quint16 head, quint16 linked) const;
    void setFixtureItem(quint32 fid, quint16 head, quint16 linked, PreviewItem props);

    /** Get a list of Fixture IDs currently set in the Monitor */
    QList <quint32> fixtureItemsID() const { return m_fixtureItems.keys(); }

    /** Return a list of the base ID and sub IDs for a fixture with the given $fid */
    QList<quint32> fixtureIDList(quint32 fid) const;

private:
    bool m_showLabels;
    QMap <quint32, FixturePreviewItem> m_fixtureItems;

    /********************************************************************
     * Generic items
     ********************************************************************/
public:
    /** Returns true if the item with ID $itemID is present in the monitor map */
    inline bool containsItem(quint32 itemID) { return m_genericItems.contains(itemID); }

    /** Remove an existing item from the generic items map */
    inline void removeItem(quint32 itemID) { m_genericItems.take(itemID); }

    /** Returns a list of all the generic item IDs */
    QList<quint32> genericItemsID();

    /** Get/Set the custom name for an item with ID $itemID */
    QString itemName(quint32 itemID);
    void setItemName(quint32 itemID, QString name);

    /** Get/Set the resource string for an item with ID $itemID */
    QString itemResource(quint32 itemID);
    void setItemResource(quint32 itemID, QString resource);

    /** Get/Set the 3D position of an item with ID $itemID */
    QVector3D itemPosition(quint32 itemID);
    void setItemPosition(quint32 itemID, QVector3D pos);

    /** Get/Set the 3D rotation of an item with ID $itemID */
    QVector3D itemRotation(quint32 itemID);
    void setItemRotation(quint32 itemID, QVector3D rot);

    /** Get/Set the 3D scale of an item with ID $itemID */
    QVector3D itemScale(quint32 itemID);
    void setItemScale(quint32 itemID, QVector3D scale);

    /** Get/Set the flags of an item with ID $itemID */
    quint32 itemFlags(quint32 itemID);
    void setItemFlags(quint32 itemID, quint32 flags);

private:
    QMap <quint32, PreviewItem> m_genericItems;

    /********************************************************************
     * 2D view background
     ********************************************************************/
public:
    /** Get/Set a background image to be displayed in 2D mode */
    inline void setCommonBackgroundImage(QString filename) { m_commonBackgroundImage = filename; }
    inline QString commonBackgroundImage() const { return m_commonBackgroundImage; }

    /** Set a picture found at $path to be displayed when $fid is started */
    void setCustomBackgroundItem(quint32 fid, QString path) { m_customBackgroundImages[fid] = path; }

    /** Helper method to set a whole list of custom pictures mapped by Function IDs */
    void setCustomBackgroundList(QMap<quint32, QString>list) { m_customBackgroundImages = list; }

    /** Reset any previously set background pictures list */
    void resetCustomBackgroundList() { m_customBackgroundImages.clear(); }

    /** Returns the map of custom background pictures organized as Function ID/Picture path */
    QMap<quint32, QString> customBackgroundList() const { return m_customBackgroundImages; }

    /** Returns the path of a custom picture set for a Function with $id */
    QString customBackground(quint32 fid);

private:
    QString m_commonBackgroundImage;
    QMap <quint32, QString> m_customBackgroundImages;

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
