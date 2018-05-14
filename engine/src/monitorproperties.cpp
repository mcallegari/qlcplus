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
#include "qlcfile.h"
#include "doc.h"

#define KXMLQLCMonitorDisplay "DisplayMode"
#define KXMLQLCMonitorChannels "ChannelStyle"
#define KXMLQLCMonitorValues "ValueStyle"
#define KXMLQLCMonitorFont "Font"
#define KXMLQLCMonitorGrid "Grid"
#define KXMLQLCMonitorGridWidth "Width"
#define KXMLQLCMonitorGridHeight "Height"
#define KXMLQLCMonitorGridDepth "Depth"
#define KXMLQLCMonitorGridUnits "Units"
#define KXMLQLCMonitorPointOfView "POV"
#define KXMLQLCMonitorShowLabels "ShowLabels"
#define KXMLQLCMonitorCommonBackground "Background"

#define KXMLQLCMonitorCustomBgItem "BackgroundItem"
#define KXMLQLCMonitorCustomBgFuncID "ID"

#define KXMLQLCMonitorFixtureItem "FxItem"
#define KXMLQLCMonitorFixtureID "ID"
#define KXMLQLCMonitorFixtureHeadIndex "Head"
#define KXMLQLCMonitorFixtureLinkedIndex "Linked"
#define KXMLQLCMonitorFixtureLinkedName "Name"
#define KXMLQLCMonitorFixtureXPos "XPos"
#define KXMLQLCMonitorFixtureYPos "YPos"
#define KXMLQLCMonitorFixtureZPos "ZPos"
#define KXMLQLCMonitorFixtureRotation "Rotation"
#define KXMLQLCMonitorFixtureXRotation "XRot"
#define KXMLQLCMonitorFixtureYRotation "YRot"
#define KXMLQLCMonitorFixtureZRotation "ZRot"
#define KXMLQLCMonitorFixtureGelColor "GelColor"

#define KXMLQLCMonitorFixtureHiddenFlag "Hidden"
#define KXMLQLCMonitorFixtureInvPanFlag "InvertedPan"
#define KXMLQLCMonitorFixtureInvTiltFlag "InvertedTilt"

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
    m_showLabels = false;
    m_fixtureItems.clear();
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

    FixturePreviewItem item = m_fixtureItems[fid];
    // if no sub items are present,
    // the fixture can be removed completely
    if (item.m_subItems.count() == 0)
    {
        m_fixtureItems.take(fid);
        return;
    }

    quint32 subID = fixtureSubID(head, linked);
    item.m_subItems.take(subID);
}

quint32 MonitorProperties::fixtureSubID(quint16 headIndex, quint16 linkedIndex) const
{
    return (((quint32)headIndex << 16) | (quint32)linkedIndex);
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

void MonitorProperties::setFixtureResource(quint32 fid, quint16 head, quint16 linked, QString resource)
{
    if (head == 0 && linked == 0)
    {
        m_fixtureItems[fid].m_baseItem.m_resource = resource;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        m_fixtureItems[fid].m_subItems[subID].m_resource = resource;
    }
}

QString MonitorProperties::fixtureResource(quint32 fid, quint16 head, quint16 linked) const
{
    if (head == 0 && linked == 0)
    {
        return m_fixtureItems[fid].m_baseItem.m_resource;
    }
    else
    {
        quint32 subID = fixtureSubID(head, linked);
        return m_fixtureItems[fid].m_subItems[subID].m_resource;
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
            if (tAttrs.hasAttribute(KXMLQLCMonitorCustomBgFuncID))
            {
                quint32 fid = tAttrs.value(KXMLQLCMonitorCustomBgFuncID).toString().toUInt();
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
        else if (root.name() == KXMLQLCMonitorFixtureItem)
        {
            // Fixture ID is mandatory. Skip the whole entry if not found.
            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureID) == false)
            {
                root.skipCurrentElement();
                continue;
            }

            PreviewItem item;
            quint32 fid = tAttrs.value(KXMLQLCMonitorFixtureID).toString().toUInt();
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

                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureLinkedName))
                    item.m_resource = tAttrs.value(KXMLQLCMonitorFixtureLinkedName).toString();
            }

            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureXPos))
                pos.setX(tAttrs.value(KXMLQLCMonitorFixtureXPos).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureYPos))
                pos.setY(tAttrs.value(KXMLQLCMonitorFixtureYPos).toString().toDouble());
            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureZPos))
                pos.setZ(tAttrs.value(KXMLQLCMonitorFixtureZPos).toString().toDouble());
            item.m_position = pos;

            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureRotation)) // check legacy first
            {
                rot.setY(tAttrs.value(KXMLQLCMonitorFixtureRotation).toString().toDouble());
            }
            else
            {
                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureXRotation))
                    rot.setX(tAttrs.value(KXMLQLCMonitorFixtureXRotation).toString().toDouble());
                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureYRotation))
                    rot.setY(tAttrs.value(KXMLQLCMonitorFixtureYRotation).toString().toDouble());
                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureZRotation))
                    rot.setZ(tAttrs.value(KXMLQLCMonitorFixtureZRotation).toString().toDouble());
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
    else if(customBackgroundList().isEmpty() == false)
    {
        QMapIterator <quint32, QString> it(customBackgroundList());
        while (it.hasNext() == true)
        {
            it.next();
            doc->writeStartElement(KXMLQLCMonitorCustomBgItem);
            quint32 fid = it.key();
            doc->writeAttribute(KXMLQLCMonitorCustomBgFuncID, QString::number(fid));
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

    foreach (quint32 fid, fixtureItemsID())
    {
        foreach (quint32 subID, fixtureIDList(fid))
        {
            quint16 headIndex = fixtureHeadIndex(subID);
            quint16 linkedIndex = fixtureLinkedIndex(subID);
            PreviewItem item = fixtureItem(fid, headIndex, linkedIndex);

            doc->writeStartElement(KXMLQLCMonitorFixtureItem);
            doc->writeAttribute(KXMLQLCMonitorFixtureID, QString::number(fid));

            if (headIndex)
                doc->writeAttribute(KXMLQLCMonitorFixtureHeadIndex, QString::number(headIndex));

            if (linkedIndex)
            {
                doc->writeAttribute(KXMLQLCMonitorFixtureLinkedIndex, QString::number(linkedIndex));
                if (item.m_resource.isEmpty() == false)
                    doc->writeAttribute(KXMLQLCMonitorFixtureLinkedName, item.m_resource);
            }

            if (item.m_flags & HiddenFlag)
                doc->writeAttribute(KXMLQLCMonitorFixtureHiddenFlag, KXMLQLCTrue);
            if (item.m_flags & InvertedPanFlag)
                doc->writeAttribute(KXMLQLCMonitorFixtureInvPanFlag, KXMLQLCTrue);
            if (item.m_flags & InvertedTiltFlag)
                doc->writeAttribute(KXMLQLCMonitorFixtureInvTiltFlag, KXMLQLCTrue);

            doc->writeAttribute(KXMLQLCMonitorFixtureXPos, QString::number(item.m_position.x()));
            doc->writeAttribute(KXMLQLCMonitorFixtureYPos, QString::number(item.m_position.y()));

#ifdef QMLUI
            doc->writeAttribute(KXMLQLCMonitorFixtureZPos, QString::number(item.m_position.z()));
            if (item.m_rotation.x() != 0)
                doc->writeAttribute(KXMLQLCMonitorFixtureXRotation, QString::number(item.m_rotation.x()));
            if (item.m_rotation.y() != 0)
                doc->writeAttribute(KXMLQLCMonitorFixtureYRotation, QString::number(item.m_rotation.y()));
            if (item.m_rotation.z() != 0)
                doc->writeAttribute(KXMLQLCMonitorFixtureZRotation, QString::number(item.m_rotation.z()));
#else
            if (item.m_rotation != QVector3D(0, 0, 0))
                doc->writeAttribute(KXMLQLCMonitorFixtureRotation, QString::number(item.m_rotation.y()));
#endif
            if (item.m_color.isValid())
                doc->writeAttribute(KXMLQLCMonitorFixtureGelColor, item.m_color.name());
            doc->writeEndElement();
        }
    }

    doc->writeEndElement();

    return true;
}
