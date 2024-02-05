/*
  Q Light Controller Plus
  monitorproperties.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QFont>

#include "monitorproperties.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "doc.h"

#define KXMLQLCMonitorDisplay       QString("DisplayMode")
#define KXMLQLCMonitorChannels      QString("ChannelStyle")
#define KXMLQLCMonitorValues        QString("ValueStyle")
#define KXMLQLCMonitorFont          QString("Font")
#define KXMLQLCMonitorGrid          QString("Grid")
#define KXMLQLCMonitorGridWidth     QString("Width")
#define KXMLQLCMonitorGridHeight    QString("Height")
#define KXMLQLCMonitorGridDepth     QString("Depth")
#define KXMLQLCMonitorGridUnits     QString("Units")
#define KXMLQLCMonitorPointOfView   QString("POV")
#define KXMLQLCMonitorItemID        QString("ID")
#define KXMLQLCMonitorShowLabels    QString("ShowLabels")

#define KXMLQLCMonitorCommonBackground  QString("Background")
#define KXMLQLCMonitorCustomBgItem      QString("BackgroundItem")

#define KXMLQLCMonitorFixtureItem   QString("FxItem")
#define KXMLQLCMonitorStageItem     QString("StageItem")
#define KXMLQLCMonitorMeshItem      QString("MeshItem")
#define KXMLQLCMonitorItemName      QString("Name")
#define KXMLQLCMonitorItemRes       QString("Res")

#define KXMLQLCMonitorItemXPosition     QString("XPos")
#define KXMLQLCMonitorItemYPosition     QString("YPos")
#define KXMLQLCMonitorItemZPosition     QString("ZPos")
#define KXMLQLCMonitorItemXRotation     QString("XRot")
#define KXMLQLCMonitorItemYRotation     QString("YRot")
#define KXMLQLCMonitorItemZRotation     QString("ZRot")
#define KXMLQLCMonitorFixtureRotation   QString("Rotation") // LEGACY
#define KXMLQLCMonitorItemXScale        QString("XScale")
#define KXMLQLCMonitorItemYScale        QString("YScale")
#define KXMLQLCMonitorItemZScale        QString("ZScale")

#define KXMLQLCMonitorFixtureHeadIndex      QString("Head")
#define KXMLQLCMonitorFixtureLinkedIndex    QString("Linked")

#define KXMLQLCMonitorFixtureGelColor QString("GelColor")

#define KXMLQLCMonitorFixtureHiddenFlag     QString("Hidden")
#define KXMLQLCMonitorFixtureInvPanFlag     QString("InvertedPan")
#define KXMLQLCMonitorFixtureInvTiltFlag    QString("InvertedTilt")

#define GRID_DEFAULT_WIDTH  5
#define GRID_DEFAULT_HEIGHT 3
#define GRID_DEFAULT_DEPTH  5

MonitorProperties::MonitorProperties()
    : m_displayMode(DMX)
    , m_channelStyle(DMXChannels)
    , m_valueStyle(DMXValues)
    , m_gridSize(QVector3D(GRID_DEFAULT_WIDTH, GRID_DEFAULT_HEIGHT, GRID_DEFAULT_DEPTH))
    , m_gridUnits(Meters)
    , m_pointOfView(Undefined)
    , m_stageType(StageSimple)
    , m_showLabels(false)
{
    m_font = QFont("Arial", 12);
}

void MonitorProperties::reset()
{
    m_gridSize = QVector3D(GRID_DEFAULT_WIDTH, GRID_DEFAULT_HEIGHT, GRID_DEFAULT_DEPTH);
    m_gridUnits = Meters;
    m_pointOfView = Undefined;
    m_stageType = StageSimple;
    m_showLabels = false;
    m_fixtureItems.clear();
    m_genericItems.clear();
    m_commonBackgroundImage = QString();
}

/********************************************************************
 * Environment
 ********************************************************************/

void MonitorProperties::setPointOfView(MonitorProperties::PointOfView pov)
{
    if (pov == m_pointOfView)
        return;

    if (m_pointOfView == Undefined)
    {
        QVector3D gSize = gridSize();
        float units = gridUnits() == MonitorProperties::Meters ? 1000.0 : 304.8;

        if (gSize.z() == 0)
        {
            // convert the grid size first
            switch (pov)
            {
                case TopView:
                    setGridSize(QVector3D(gSize.x(), GRID_DEFAULT_HEIGHT, gSize.y()));
                break;
                case RightSideView:
                case LeftSideView:
                    setGridSize(QVector3D(GRID_DEFAULT_WIDTH, gSize.x(), gSize.x()));
                break;
                default:
                break;
            }
        }

        foreach (quint32 fid, fixtureItemsID())
        {
            foreach (quint32 subID, fixtureIDList(fid))
            {
                QVector3D pos = fixturePosition(fid, fixtureHeadIndex(subID), fixtureLinkedIndex(subID));
                QVector3D newPos;

                switch (pov)
                {
                    case TopView:
                    {
                        newPos = QVector3D(pos.x(), 1000, pos.y());
                    }
                    break;
                    case RightSideView:
                    {
                        newPos = QVector3D(0, pos.y(), (gridSize().z() * units) - pos.x());
                    }
                    break;
                    case LeftSideView:
                    {
                        newPos = QVector3D(0, pos.y(), pos.x());
                    }
                    break;
                    default:
                        newPos = QVector3D(pos.x(), (gridSize().y() * units) - pos.y(), 1000);
                    break;
                }
                setFixturePosition(fid, fixtureHeadIndex(subID), fixtureLinkedIndex(subID), newPos);
            }
        }
    }
    m_pointOfView = pov;
}

/********************************************************************
 * Fixture items
 ********************************************************************/

void MonitorProperties::removeFixture(quint32 fid)
{
    if (m_fixtureItems.contains(fid))
        m_fixtureItems.take(fid);
}

void MonitorProperties::removeFixture(quint32 fid, quint16 head, quint16 linked)
{
    if (m_fixtureItems.contains(fid) == false)
        return;

    // if no sub items are present,
    // the fixture can be removed completely
    if (m_fixtureItems[fid].m_subItems.count() == 0)
    {
        m_fixtureItems.take(fid);
        return;
    }

    quint32 subID = fixtureSubID(head, linked);
    m_fixtureItems[fid].m_subItems.remove(subID);
}

quint32 MonitorProperties::fixtureSubID(quint32 headIndex, quint32 linkedIndex) const
{
    return ((headIndex << 16) | linkedIndex);
}

quint16 MonitorProperties::fixtureHeadIndex(quint32 mapID) const
{
    return (quint16)(mapID >> 16);
}

quint16 MonitorProperties::fixtureLinkedIndex(quint32 mapID) const
{
    return (quint16)(mapID & 0x0000FFFF);
}

bool MonitorProperties::containsItem(quint32 fid, quint16 head, quint16 linked)
{
    if (m_fixtureItems.contains(fid) == false)
        return false;

    if (head == 0 && linked == 0)
        return true;

    quint32 subID = fixtureSubID(head, linked);
    return m_fixtureItems[fid].m_subItems.contains(subID);
}

void MonitorProperties::setFixturePosition(quint32 fid, quint16 head, quint16 linked, QVector3D pos)
{
    //qDebug() << Q_FUNC_INFO << "X:" << pos.x() << "Y:" << pos.y();
    if (head == 0 && linked == 0)
    {
        m_fixtureItems[fid].m_baseItem.m_position = pos;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        m_fixtureItems[fid].m_subItems[subID].m_position = pos;
    }
}

QVector3D MonitorProperties::fixturePosition(quint32 fid, quint16 head, quint16 linked) const
{
    if (head == 0 && linked == 0)
    {
        return m_fixtureItems[fid].m_baseItem.m_position;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        return m_fixtureItems[fid].m_subItems[subID].m_position;
    }
}

void MonitorProperties::setFixtureRotation(quint32 fid, quint16 head, quint16 linked, QVector3D degrees)
{
    if (head == 0 && linked == 0)
    {
        m_fixtureItems[fid].m_baseItem.m_rotation = degrees;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        m_fixtureItems[fid].m_subItems[subID].m_rotation = degrees;
    }
}

QVector3D MonitorProperties::fixtureRotation(quint32 fid, quint16 head, quint16 linked) const
{
    if (head == 0 && linked == 0)
    {
        return m_fixtureItems[fid].m_baseItem.m_rotation;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        return m_fixtureItems[fid].m_subItems[subID].m_rotation;
    }
}

void MonitorProperties::setFixtureGelColor(quint32 fid, quint16 head, quint16 linked, QColor col)
{
    //qDebug() << Q_FUNC_INFO << "Gel color:" << col;
    if (head == 0 && linked == 0)
    {
        m_fixtureItems[fid].m_baseItem.m_color = col;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        m_fixtureItems[fid].m_subItems[subID].m_color = col;
    }
}

QColor MonitorProperties::fixtureGelColor(quint32 fid, quint16 head, quint16 linked) const
{
    if (head == 0 && linked == 0)
    {
        return m_fixtureItems[fid].m_baseItem.m_color;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        return m_fixtureItems[fid].m_subItems[subID].m_color;
    }
}

void MonitorProperties::setFixtureName(quint32 fid, quint16 head, quint16 linked, QString name)
{
    if (head == 0 && linked == 0)
    {
        m_fixtureItems[fid].m_baseItem.m_name = name;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        m_fixtureItems[fid].m_subItems[subID].m_name = name;
    }
}

QString MonitorProperties::fixtureName(quint32 fid, quint16 head, quint16 linked) const
{
    if (head == 0 && linked == 0)
    {
        return m_fixtureItems[fid].m_baseItem.m_name;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        return m_fixtureItems[fid].m_subItems[subID].m_name;
    }
}

void MonitorProperties::setFixtureFlags(quint32 fid, quint16 head, quint16 linked, quint32 flags)
{
    if (head == 0 && linked == 0)
    {
        m_fixtureItems[fid].m_baseItem.m_flags = flags;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        m_fixtureItems[fid].m_subItems[subID].m_flags = flags;
    }
}

quint32 MonitorProperties::fixtureFlags(quint32 fid, quint16 head, quint16 linked) const
{
    if (head == 0 && linked == 0)
    {
        return m_fixtureItems[fid].m_baseItem.m_flags;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        return m_fixtureItems[fid].m_subItems[subID].m_flags;
    }
}

PreviewItem MonitorProperties::fixtureItem(quint32 fid, quint16 head, quint16 linked) const
{
    if (head == 0 && linked == 0)
    {
        return m_fixtureItems[fid].m_baseItem;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        return m_fixtureItems[fid].m_subItems[subID];
    }
}

void MonitorProperties::setFixtureItem(quint32 fid, quint16 head, quint16 linked, PreviewItem props)
{
    if (head == 0 && linked == 0)
    {
        m_fixtureItems[fid].m_baseItem = props;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        m_fixtureItems[fid].m_subItems[subID] = props;
    }
}

QList<quint32> MonitorProperties::fixtureIDList(quint32 fid) const
{
    QList<quint32> list;
    if (m_fixtureItems.contains(fid) == false)
        return list;

    // add the basic fixture item ID
    list.append(0);

    FixturePreviewItem fxItem = m_fixtureItems[fid];
    list.append(fxItem.m_subItems.keys());

    return list;
}

/********************************************************************
 * Generic items
 ********************************************************************/

QList<quint32> MonitorProperties::genericItemsID()
{
    return m_genericItems.keys();
}

QString MonitorProperties::itemName(quint32 itemID)
{
    if (m_genericItems[itemID].m_name.isEmpty())
    {
        QFileInfo rName(m_genericItems[itemID].m_resource);
        return rName.baseName();
    }

    return m_genericItems[itemID].m_name;
}

void MonitorProperties::setItemName(quint32 itemID, QString name)
{
    m_genericItems[itemID].m_name = name;
}

QString MonitorProperties::itemResource(quint32 itemID)
{
    return m_genericItems[itemID].m_resource;
}

void MonitorProperties::setItemResource(quint32 itemID, QString resource)
{
    m_genericItems[itemID].m_resource = resource;
}

QVector3D MonitorProperties::itemPosition(quint32 itemID)
{
    return m_genericItems[itemID].m_position;
}

void MonitorProperties::setItemPosition(quint32 itemID, QVector3D pos)
{
    m_genericItems[itemID].m_position = pos;
}

QVector3D MonitorProperties::itemRotation(quint32 itemID)
{
    return m_genericItems[itemID].m_rotation;
}

void MonitorProperties::setItemRotation(quint32 itemID, QVector3D rot)
{
    m_genericItems[itemID].m_rotation = rot;
}

QVector3D MonitorProperties::itemScale(quint32 itemID)
{
    if (m_genericItems[itemID].m_scale.isNull())
        return QVector3D(1.0, 1.0, 1.0);

    return m_genericItems[itemID].m_scale;
}

void MonitorProperties::setItemScale(quint32 itemID, QVector3D scale)
{
    m_genericItems[itemID].m_scale = scale;
}

quint32 MonitorProperties::itemFlags(quint32 itemID)
{
    return m_genericItems[itemID].m_flags;
}

void MonitorProperties::setItemFlags(quint32 itemID, quint32 flags)
{
    m_genericItems[itemID].m_flags = flags;
}

/********************************************************************
 * 2D view background
 ********************************************************************/

QString MonitorProperties::customBackground(quint32 fid)
{
    return m_customBackgroundImages.value(fid, QString());
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool MonitorProperties::loadXML(QXmlStreamReader &root, const Doc *mainDocument)
{
    if (root.name() != KXMLQLCMonitorProperties)
    {
        qWarning() << Q_FUNC_INFO << "Monitor node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();

    if (attrs.hasAttribute(KXMLQLCMonitorDisplay) == false)
    {
        qWarning() << Q_FUNC_INFO << "Cannot determine Monitor display mode !";
        return false;
    }

    setDisplayMode(DisplayMode(attrs.value(KXMLQLCMonitorDisplay).toString().toInt()));
    if (attrs.hasAttribute(KXMLQLCMonitorShowLabels))
    {
        if (attrs.value(KXMLQLCMonitorShowLabels).toString() == "1")
            setLabelsVisible(true);
        else
            setLabelsVisible(false);
    }

    while (root.readNextStartElement())
    {
        QXmlStreamAttributes tAttrs = root.attributes();
        if (root.name() == KXMLQLCMonitorFont)
        {
            QFont fn;
            fn.fromString(root.readElementText());
            setFont(fn);
        }
        else if (root.name() == KXMLQLCMonitorChannels)
            setChannelStyle(ChannelStyle(root.readElementText().toInt()));
        else if (root.name() == KXMLQLCMonitorValues)
            setValueStyle(ValueStyle(root.readElementText().toInt()));
        else if (root.name() == KXMLQLCMonitorCommonBackground)
            setCommonBackgroundImage(mainDocument->denormalizeComponentPath(root.readElementText()));
        else if (root.name() == KXMLQLCMonitorCustomBgItem)
        {
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemID))
            {
                quint32 fid = tAttrs.value(KXMLQLCMonitorItemID).toString().toUInt();
                setCustomBackgroundItem(fid, mainDocument->denormalizeComponentPath(root.readElementText()));
            }
        }
        else if (root.name() == KXMLQLCMonitorGrid)
        {
            int w = 5, h = 3, d = 5;
            if (tAttrs.hasAttribute(KXMLQLCMonitorGridWidth))
                w = tAttrs.value(KXMLQLCMonitorGridWidth).toString().toInt();
            if (tAttrs.hasAttribute(KXMLQLCMonitorGridHeight))
                h = tAttrs.value(KXMLQLCMonitorGridHeight).toString().toInt();
            if (tAttrs.hasAttribute(KXMLQLCMonitorGridDepth))
                d = tAttrs.value(KXMLQLCMonitorGridDepth).toString().toInt();
            else
                d = h; // backward compatibility

            if (tAttrs.hasAttribute(KXMLQLCMonitorGridUnits))
                setGridUnits(GridUnits(tAttrs.value(KXMLQLCMonitorGridUnits).toString().toInt()));

            if (tAttrs.hasAttribute(KXMLQLCMonitorPointOfView))
                setPointOfView(PointOfView(tAttrs.value(KXMLQLCMonitorPointOfView).toString().toInt()));
            else
                setPointOfView(Undefined);

            setGridSize(QVector3D(w, h, d));
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCMonitorStageItem)
        {
            setStageType(StageType(root.readElementText().toInt()));
        }
        else if (root.name() == KXMLQLCMonitorFixtureItem)
        {
            // Fixture ID is mandatory. Skip the whole entry if not found.
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemID) == false)
            {
                root.skipCurrentElement();
                continue;
            }

            PreviewItem item;
            quint32 fid = tAttrs.value(KXMLQLCMonitorItemID).toString().toUInt();
            quint16 headIndex = 0;
            quint16 linkedIndex = 0;
            QVector3D pos(0, 0, 0);
            QVector3D rot(0, 0, 0);

            item.m_flags = 0;

            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureHeadIndex))
                headIndex = tAttrs.value(KXMLQLCMonitorFixtureHeadIndex).toString().toUInt();

            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureLinkedIndex))
            {
                linkedIndex = tAttrs.value(KXMLQLCMonitorFixtureLinkedIndex).toString().toUInt();

                if (tAttrs.hasAttribute(KXMLQLCMonitorItemName))
                    item.m_name = tAttrs.value(KXMLQLCMonitorItemName).toString();
            }

            if (tAttrs.hasAttribute(KXMLQLCMonitorItemXPosition))
                pos.setX(tAttrs.value(KXMLQLCMonitorItemXPosition).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemYPosition))
                pos.setY(tAttrs.value(KXMLQLCMonitorItemYPosition).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemZPosition))
                pos.setZ(tAttrs.value(KXMLQLCMonitorItemZPosition).toString().toDouble());
            item.m_position = pos;

            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureRotation)) // check legacy first
            {
                rot.setY(tAttrs.value(KXMLQLCMonitorFixtureRotation).toString().toDouble());
            }
            else
            {
                if (tAttrs.hasAttribute(KXMLQLCMonitorItemXRotation))
                    rot.setX(tAttrs.value(KXMLQLCMonitorItemXRotation).toString().toDouble());
                if (tAttrs.hasAttribute(KXMLQLCMonitorItemYRotation))
                    rot.setY(tAttrs.value(KXMLQLCMonitorItemYRotation).toString().toDouble());
                if (tAttrs.hasAttribute(KXMLQLCMonitorItemZRotation))
                    rot.setZ(tAttrs.value(KXMLQLCMonitorItemZRotation).toString().toDouble());
            }
            item.m_rotation = rot;

            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureGelColor))
                item.m_color = QColor(tAttrs.value(KXMLQLCMonitorFixtureGelColor).toString());

            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureHiddenFlag))
                item.m_flags |= HiddenFlag;
            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureInvPanFlag))
                item.m_flags |= InvertedPanFlag;
            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureInvTiltFlag))
                item.m_flags |= InvertedTiltFlag;

            setFixtureItem(fid, headIndex, linkedIndex, item);
            root.skipCurrentElement();

        }
        else if (root.name() == KXMLQLCMonitorMeshItem)
        {
            // Item ID is mandatory. Skip the whole entry if not found.
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemID) == false)
            {
                root.skipCurrentElement();
                continue;
            }

            PreviewItem item;
            quint32 itemID = tAttrs.value(KXMLQLCMonitorItemID).toString().toUInt();
            QVector3D pos(0, 0, 0);
            QVector3D rot(0, 0, 0);
            QVector3D scale(1.0, 1.0, 1.0);

            item.m_flags = 0;

            if (tAttrs.hasAttribute(KXMLQLCMonitorItemXPosition))
                pos.setX(tAttrs.value(KXMLQLCMonitorItemXPosition).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemYPosition))
                pos.setY(tAttrs.value(KXMLQLCMonitorItemYPosition).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemZPosition))
                pos.setZ(tAttrs.value(KXMLQLCMonitorItemZPosition).toString().toDouble());
            item.m_position = pos;

            if (tAttrs.hasAttribute(KXMLQLCMonitorItemXRotation))
                rot.setX(tAttrs.value(KXMLQLCMonitorItemXRotation).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemYRotation))
                rot.setY(tAttrs.value(KXMLQLCMonitorItemYRotation).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemZRotation))
                rot.setZ(tAttrs.value(KXMLQLCMonitorItemZRotation).toString().toDouble());
            item.m_rotation = rot;

            if (tAttrs.hasAttribute(KXMLQLCMonitorItemXScale))
                scale.setX(tAttrs.value(KXMLQLCMonitorItemXScale).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemYScale))
                scale.setY(tAttrs.value(KXMLQLCMonitorItemYScale).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorItemZScale))
                scale.setZ(tAttrs.value(KXMLQLCMonitorItemZScale).toString().toDouble());
            item.m_scale = scale;

            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureHiddenFlag))
                item.m_flags |= HiddenFlag;

            if (tAttrs.hasAttribute(KXMLQLCMonitorItemRes))
                item.m_resource = tAttrs.value(KXMLQLCMonitorItemRes).toString();

            if (tAttrs.hasAttribute(KXMLQLCMonitorItemName))
                item.m_name = tAttrs.value(KXMLQLCMonitorItemName).toString();

            m_genericItems[itemID] = item;
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown MonitorProperties tag:" << root.name();
            root.skipCurrentElement();
        }
    }
    return true;
}

bool MonitorProperties::saveXML(QXmlStreamWriter *doc, const Doc *mainDocument) const
{
    Q_ASSERT(doc != NULL);

    /* Create the master Monitor node */
    doc->writeStartElement(KXMLQLCMonitorProperties);
    doc->writeAttribute(KXMLQLCMonitorDisplay, QString::number(displayMode()));
    doc->writeAttribute(KXMLQLCMonitorShowLabels, QString::number(labelsVisible()));

    /* Font */
    doc->writeTextElement(KXMLQLCMonitorFont, font().toString());
    /* Channels style */
    doc->writeTextElement(KXMLQLCMonitorChannels, QString::number(channelStyle()));
    /* Values style */
    doc->writeTextElement(KXMLQLCMonitorValues, QString::number(valueStyle()));

    /* Background */
    if (commonBackgroundImage().isEmpty() == false)
    {
        doc->writeTextElement(KXMLQLCMonitorCommonBackground,
                              mainDocument->normalizeComponentPath(commonBackgroundImage()));
    }
    else if (customBackgroundList().isEmpty() == false)
    {
        QMapIterator <quint32, QString> it(customBackgroundList());
        while (it.hasNext() == true)
        {
            it.next();
            doc->writeStartElement(KXMLQLCMonitorCustomBgItem);
            quint32 fid = it.key();
            doc->writeAttribute(KXMLQLCMonitorItemID, QString::number(fid));
            doc->writeCharacters(mainDocument->normalizeComponentPath(it.value()));
            doc->writeEndElement();
        }
    }

    doc->writeStartElement(KXMLQLCMonitorGrid);
    doc->writeAttribute(KXMLQLCMonitorGridWidth, QString::number(gridSize().x()));
    doc->writeAttribute(KXMLQLCMonitorGridHeight, QString::number(gridSize().y()));
    doc->writeAttribute(KXMLQLCMonitorGridDepth, QString::number(gridSize().z()));
    doc->writeAttribute(KXMLQLCMonitorGridUnits, QString::number(gridUnits()));
    if (m_pointOfView != Undefined)
        doc->writeAttribute(KXMLQLCMonitorPointOfView, QString::number(pointOfView()));

    doc->writeEndElement();

#ifdef QMLUI
    doc->writeTextElement(KXMLQLCMonitorStageItem, QString::number(stageType()));
#endif

    // ***********************************************************
    // *                write fixtures information               *
    // ***********************************************************

    foreach (quint32 fid, fixtureItemsID())
    {
        foreach (quint32 subID, fixtureIDList(fid))
        {
            quint16 headIndex = fixtureHeadIndex(subID);
            quint16 linkedIndex = fixtureLinkedIndex(subID);
            PreviewItem item = fixtureItem(fid, headIndex, linkedIndex);

            doc->writeStartElement(KXMLQLCMonitorFixtureItem);
            doc->writeAttribute(KXMLQLCMonitorItemID, QString::number(fid));

            if (headIndex)
                doc->writeAttribute(KXMLQLCMonitorFixtureHeadIndex, QString::number(headIndex));

            if (linkedIndex)
            {
                doc->writeAttribute(KXMLQLCMonitorFixtureLinkedIndex, QString::number(linkedIndex));
                if (item.m_name.isEmpty() == false)
                    doc->writeAttribute(KXMLQLCMonitorItemName, item.m_name);
            }

            // write flags, if present
            if (item.m_flags & HiddenFlag)
                doc->writeAttribute(KXMLQLCMonitorFixtureHiddenFlag, KXMLQLCTrue);
            if (item.m_flags & InvertedPanFlag)
                doc->writeAttribute(KXMLQLCMonitorFixtureInvPanFlag, KXMLQLCTrue);
            if (item.m_flags & InvertedTiltFlag)
                doc->writeAttribute(KXMLQLCMonitorFixtureInvTiltFlag, KXMLQLCTrue);

            // always write position
            doc->writeAttribute(KXMLQLCMonitorItemXPosition, QString::number(item.m_position.x()));
            doc->writeAttribute(KXMLQLCMonitorItemYPosition, QString::number(item.m_position.y()));

#ifdef QMLUI
            doc->writeAttribute(KXMLQLCMonitorItemZPosition, QString::number(item.m_position.z()));

            // write rotation, if set
            if (item.m_rotation.x() != 0)
                doc->writeAttribute(KXMLQLCMonitorItemXRotation, QString::number(item.m_rotation.x()));
            if (item.m_rotation.y() != 0)
                doc->writeAttribute(KXMLQLCMonitorItemYRotation, QString::number(item.m_rotation.y()));
            if (item.m_rotation.z() != 0)
                doc->writeAttribute(KXMLQLCMonitorItemZRotation, QString::number(item.m_rotation.z()));
#else
            if (item.m_rotation != QVector3D(0, 0, 0))
                doc->writeAttribute(KXMLQLCMonitorFixtureRotation, QString::number(item.m_rotation.y()));
#endif
            if (item.m_color.isValid())
                doc->writeAttribute(KXMLQLCMonitorFixtureGelColor, item.m_color.name());

            doc->writeEndElement();
        }
    }
#ifdef QMLUI
    QDir dir = QDir::cleanPath(QLCFile::systemDirectory(MESHESDIR).path());
    QString meshDirAbsPath = dir.absolutePath() + QDir::separator();
#endif

    // ***********************************************************
    // *             write generic items information             *
    // ***********************************************************
    QMapIterator<quint32, PreviewItem> it(m_genericItems);
    while (it.hasNext())
    {
        it.next();
        quint32 itemID = it.key();
        PreviewItem item = it.value();

        doc->writeStartElement(KXMLQLCMonitorMeshItem);
        doc->writeAttribute(KXMLQLCMonitorItemID, QString::number(itemID));

        // write flags, if present
        if (item.m_flags & HiddenFlag)
            doc->writeAttribute(KXMLQLCMonitorFixtureHiddenFlag, KXMLQLCTrue);

        // always write position
        doc->writeAttribute(KXMLQLCMonitorItemXPosition, QString::number(item.m_position.x()));
        doc->writeAttribute(KXMLQLCMonitorItemYPosition, QString::number(item.m_position.y()));
        doc->writeAttribute(KXMLQLCMonitorItemZPosition, QString::number(item.m_position.z()));

        // write rotation, if set
        if (item.m_rotation.x() != 0)
            doc->writeAttribute(KXMLQLCMonitorItemXRotation, QString::number(item.m_rotation.x()));
        if (item.m_rotation.y() != 0)
            doc->writeAttribute(KXMLQLCMonitorItemYRotation, QString::number(item.m_rotation.y()));
        if (item.m_rotation.z() != 0)
            doc->writeAttribute(KXMLQLCMonitorItemZRotation, QString::number(item.m_rotation.z()));

        // write scale, if set
        if (item.m_scale.x() != 1.0)
            doc->writeAttribute(KXMLQLCMonitorItemXScale, QString::number(item.m_scale.x()));
        if (item.m_scale.y() != 1.0)
            doc->writeAttribute(KXMLQLCMonitorItemYScale, QString::number(item.m_scale.y()));
        if (item.m_scale.z() != 1.0)
            doc->writeAttribute(KXMLQLCMonitorItemZScale, QString::number(item.m_scale.z()));

        if (item.m_resource.isEmpty() == false)
        {
            // perform normalization depending on the mesh location
            // (mesh folder, project path, absolute path)
            QFileInfo res(item.m_resource);

            if (res.isRelative())
            {
                doc->writeAttribute(KXMLQLCMonitorItemRes, item.m_resource);
            }
#ifdef QMLUI
            else if (item.m_resource.startsWith(meshDirAbsPath))
            {
                item.m_resource.remove(meshDirAbsPath);
                doc->writeAttribute(KXMLQLCMonitorItemRes, item.m_resource);
            }
#endif
            else
            {
                doc->writeAttribute(KXMLQLCMonitorItemRes, mainDocument->normalizeComponentPath(item.m_resource));
            }
        }

        if (item.m_name.isEmpty() == false)
            doc->writeAttribute(KXMLQLCMonitorItemName, item.m_name);

        doc->writeEndElement();
    }

    doc->writeEndElement();

    return true;
}
