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
#include "doc.h"

#define KXMLQLCMonitorDisplay "DisplayMode"
#define KXMLQLCMonitorChannels "ChannelStyle"
#define KXMLQLCMonitorValues "ValueStyle"
#define KXMLQLCMonitorFont "Font"
#define KXMLQLCMonitorGrid "Grid"
#define KXMLQLCMonitorGridWidth "Width"
#define KXMLQLCMonitorGridHeight "Height"
#define KXMLQLCMonitorGridUnits "Units"
#define KXMLQLCMonitorShowLabels "ShowLabels"
#define KXMLQLCMonitorCommonBackground "Background"

#define KXMLQLCMonitorCustomBgItem "BackgroundItem"
#define KXMLQLCMonitorCustomBgFuncID "ID"

#define KXMLQLCMonitorFixtureItem "FxItem"
#define KXMLQLCMonitorFixtureID "ID"
#define KXMLQLCMonitorFixtureXPos "XPos"
#define KXMLQLCMonitorFixtureYPos "YPos"
#define KXMLQLCMonitorFixtureRotation "Rotation"
#define KXMLQLCMonitorFixtureGelColor "GelColor"

MonitorProperties::MonitorProperties()
{
    m_font = QFont("Arial", 12);
    m_displayMode = DMX;
    m_channelStyle = DMXChannels;
    m_valueStyle = DMXValues;
    m_gridSize = QSize(5, 5);
    m_gridUnits = Meters;
    m_showLabels = false;
}

void MonitorProperties::removeFixture(quint32 fid)
{
    if (m_fixtureItems.contains(fid))
        m_fixtureItems.take(fid);
}

void MonitorProperties::setFixturePosition(quint32 fid, QPointF pos)
{
    qDebug() << Q_FUNC_INFO << "X:" << pos.x() << "Y:" << pos.y();
    m_fixtureItems[fid].m_position = pos;
}

void MonitorProperties::setFixtureRotation(quint32 fid, ushort degrees)
{
    m_fixtureItems[fid].m_rotation = degrees;
}

void MonitorProperties::setFixtureGelColor(quint32 fid, QColor col)
{
    qDebug() << Q_FUNC_INFO << "Gel color:" << col;
    m_fixtureItems[fid].m_gelColor = col;
}

QString MonitorProperties::customBackground(quint32 id)
{
    if (m_customBackgroundImages.contains(id))
        return m_customBackgroundImages[id];

    return QString();
}

void MonitorProperties::reset()
{
    m_gridSize = QSize(5, 5);
    m_gridUnits = Meters;
    m_showLabels = false;
    m_fixtureItems.clear();
    m_commonBackgroundImage = QString();
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool MonitorProperties::loadXML(QXmlStreamReader &root, const Doc * mainDocument)
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
            int w = 5, h = 5;
            if (tAttrs.hasAttribute(KXMLQLCMonitorGridWidth))
                w = tAttrs.value(KXMLQLCMonitorGridWidth).toString().toInt();
            if (tAttrs.hasAttribute(KXMLQLCMonitorGridHeight))
                h = tAttrs.value(KXMLQLCMonitorGridHeight).toString().toInt();
            if (tAttrs.hasAttribute(KXMLQLCMonitorGridUnits))
                setGridUnits(GridUnits(tAttrs.value(KXMLQLCMonitorGridUnits).toString().toInt()));

            setGridSize(QSize(w, h));
        }
        else if (root.name() == KXMLQLCMonitorFixtureItem)
        {
            if (tag.hasAttribute(KXMLQLCMonitorFixtureID))
            {
                quint32 fid = tAttrs.value(KXMLQLCMonitorFixtureID).toString().toUInt();
                QPointF pos(0, 0);
                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureXPos))
                    pos.setX(tAttrs.value(KXMLQLCMonitorFixtureXPos).toString().toDouble());
                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureYPos))
                    pos.setY(tAttrs.value(KXMLQLCMonitorFixtureYPos).toString().toDouble());
                setFixturePosition(fid, pos);

                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureRotation))
                    setFixtureRotation(fid, tAttrs.value(KXMLQLCMonitorFixtureRotation).toString().toUShort());

                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureGelColor))
                    setFixtureGelColor(fid, QColor(tAttrs.value(KXMLQLCMonitorFixtureGelColor).toString()));
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown MonitorProperties tag:" << root.name();
            root.skipCurrentElement();
        }
    }
    return true;
}

bool MonitorProperties::saveXML(QDomDocument *doc, QDomElement *wksp_root, const Doc * mainDocument) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    QDomElement root;
    QDomElement tag;
    QDomText text;

    /* Create the master Monitor node */
    root = doc->createElement(KXMLQLCMonitorProperties);
    root.setAttribute(KXMLQLCMonitorDisplay, displayMode());
    root.setAttribute(KXMLQLCMonitorShowLabels, labelsVisible());
    wksp_root->appendChild(root);


    tag = doc->createElement(KXMLQLCMonitorFont);
    root.appendChild(tag);
    text = doc->createTextNode(font().toString());
    tag.appendChild(text);

    tag = doc->createElement(KXMLQLCMonitorChannels);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(channelStyle()));
    tag.appendChild(text);

    tag = doc->createElement(KXMLQLCMonitorValues);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(valueStyle()));
    tag.appendChild(text);

    if (commonBackgroundImage().isEmpty() == false)
    {
        tag = doc->createElement(KXMLQLCMonitorCommonBackground);
        root.appendChild(tag);
        text = doc->createTextNode(mainDocument->normalizeComponentPath(commonBackgroundImage()));
        tag.appendChild(text);
    }
    else if(customBackgroundList().isEmpty() == false)
    {
        QHashIterator <quint32, QString> it(customBackgroundList());
        while (it.hasNext() == true)
        {
            it.next();
 
            tag = doc->createElement(KXMLQLCMonitorCustomBgItem);
            root.appendChild(tag);
            quint32 fid = it.key();
            tag.setAttribute(KXMLQLCMonitorCustomBgFuncID, fid);
            text = doc->createTextNode(mainDocument->normalizeComponentPath(it.value()));
            tag.appendChild(text);
        }
    }

    tag = doc->createElement(KXMLQLCMonitorGrid);
    tag.setAttribute(KXMLQLCMonitorGridWidth, gridSize().width());
    tag.setAttribute(KXMLQLCMonitorGridHeight, gridSize().height());
    tag.setAttribute(KXMLQLCMonitorGridUnits, gridUnits());
    root.appendChild(tag);

    foreach (quint32 fid, fixtureItemsID())
    {
        QPointF pos = fixturePosition(fid);
        tag = doc->createElement(KXMLQLCMonitorFixtureItem);
        tag.setAttribute(KXMLQLCMonitorFixtureID, fid);
        tag.setAttribute(KXMLQLCMonitorFixtureXPos, QString::number(pos.x()));
        tag.setAttribute(KXMLQLCMonitorFixtureYPos, QString::number(pos.y()));
        if (fixtureRotation(fid) != 0)
            tag.setAttribute(KXMLQLCMonitorFixtureRotation, QString::number(fixtureRotation(fid)));

        QColor col = fixtureGelColor(fid);
        if (col.isValid())
            tag.setAttribute(KXMLQLCMonitorFixtureGelColor, col.name());
        root.appendChild(tag);
    }

    return true;
}
