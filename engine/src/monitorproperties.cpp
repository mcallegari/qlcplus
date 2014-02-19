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

#include <QDebug>
#include <QFont>

#include "monitorproperties.h"
#include <QDomDocument>
#include <QDomElement>

#define KXMLQLCMonitorDisplay "DisplayMode"
#define KXMLQLCMonitorChannels "ChannelStyle"
#define KXMLQLCMonitorValues "ValueStyle"
#define KXMLQLCMonitorFont "Font"
#define KXMLQLCMonitorGrid "Grid"
#define KXMLQLCMonitorGridWidth "Width"
#define KXMLQLCMonitorGridHeight "Height"
#define KXMLQLCMonitorGridUnits "Units"

#define KXMLQLCMonitorFixtureItem "FxItem"
#define KXMLQLCMonitorFixtureID "ID"
#define KXMLQLCMonitorFixtureXPos "XPos"
#define KXMLQLCMonitorFixtureYPos "YPos"

MonitorProperties::MonitorProperties()
{
    m_font = QFont("Arial", 12);
    m_displayMode = DMX;
    m_channelStyle = DMXChannels;
    m_valueStyle = DMXValues;
    m_gridSize = QSize(5, 5);
    m_gridUnits = Meters;
}

void MonitorProperties::setFixturePosition(quint32 fid, QPointF pos)
{
    qDebug() << Q_FUNC_INFO << "X:" << pos.x() << "Y:" << pos.y();
    m_fixtureItems[fid] = pos;
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool MonitorProperties::loadXML(const QDomElement &root)
{
    if (root.tagName() != KXMLQLCMonitorProperties)
    {
        qWarning() << Q_FUNC_INFO << "Monitor node not found";
        return false;
    }

    if (root.hasAttribute(KXMLQLCMonitorDisplay) == false)
    {
        qWarning() << Q_FUNC_INFO << "Cannot determine Monitor display mode !";
        return false;
    }

    setDisplayMode(DisplayMode(root.attribute(KXMLQLCMonitorDisplay).toInt()));

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCMonitorFont)
        {
            QFont fn;
            fn.fromString(tag.text());
            setFont(fn);
        }
        else if (tag.tagName() == KXMLQLCMonitorChannels)
            setChannelStyle(ChannelStyle(tag.text().toInt()));
        else if (tag.tagName() == KXMLQLCMonitorValues)
            setValueStyle(ValueStyle(tag.text().toInt()));
        else if (tag.tagName() == KXMLQLCMonitorGrid)
        {
            int w = 5, h = 5;
            if (tag.hasAttribute(KXMLQLCMonitorGridWidth))
                w = tag.attribute(KXMLQLCMonitorGridWidth).toInt();
            if (tag.hasAttribute(KXMLQLCMonitorGridHeight))
                h = tag.attribute(KXMLQLCMonitorGridHeight).toInt();
            if (tag.hasAttribute(KXMLQLCMonitorGridUnits))
                setGridUnits(GridUnits(tag.attribute(KXMLQLCMonitorGridUnits).toInt()));

            setGridSize(QSize(w, h));
        }
        else if (tag.tagName() == KXMLQLCMonitorFixtureItem)
        {
            if (tag.hasAttribute(KXMLQLCMonitorFixtureID))
            {
                quint32 fid = tag.attribute(KXMLQLCMonitorFixtureID).toUInt();
                QPointF pos(0, 0);
                if (tag.hasAttribute(KXMLQLCMonitorFixtureXPos))
                    pos.setX(tag.attribute(KXMLQLCMonitorFixtureXPos).toDouble());
                if (tag.hasAttribute(KXMLQLCMonitorFixtureYPos))
                    pos.setY(tag.attribute(KXMLQLCMonitorFixtureYPos).toDouble());
                setFixturePosition(fid, pos);
            }
        }

        node = node.nextSibling();
    }
    return true;
}

bool MonitorProperties::saveXML(QDomDocument *doc, QDomElement *wksp_root) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    QDomElement root;
    QDomElement tag;
    QDomText text;

    /* Create the master Monitor node */
    root = doc->createElement(KXMLQLCMonitorProperties);
    root.setAttribute(KXMLQLCMonitorDisplay, displayMode());
    wksp_root->appendChild(root);

    if (displayMode() == DMX)
    {
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
    }
    else if (displayMode() == Graphics)
    {
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
            root.appendChild(tag);
        }
    }

    return true;
}
