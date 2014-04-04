/*
  Q Light Controller Plus
  monitorfixtureitem.cpp

  Copyright (C) Massimo Callegari

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

#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QCursor>
#include <QDebug>

#include "monitorfixtureitem.h"
#include "qlcfixturehead.h"
#include "fixture.h"
#include "doc.h"

MonitorFixtureItem::MonitorFixtureItem(Doc *doc, quint32 fid)
    : m_doc(doc)
    , m_fid(fid)
    , m_gelColor(QColor())
    , m_labelVisibility(false)
{
    Q_ASSERT(doc != NULL);

    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    Fixture *fxi = m_doc->fixture(fid);
    Q_ASSERT(fxi != NULL);

    m_name = fxi->name();

    setToolTip(m_name);

    m_font = qApp->font();
    m_font.setPixelSize(8);

    for (int i = 0; i < fxi->heads(); i++)
    {
        FixtureHead fxiItem;
        fxiItem.m_item = new QGraphicsEllipseItem(this);
        fxiItem.m_item->setPen(QPen(Qt::white, 1));
        fxiItem.m_item->setBrush(QBrush(Qt::black));

        QLCFixtureHead head = fxi->head(i);
        foreach (quint32 rgbComp, head.rgbChannels())
        {
            fxiItem.m_rgb.append(rgbComp + fxi->address());
            qDebug() << "Add RGB comp at address:" << rgbComp + fxi->address();
        }
        foreach (quint32 cmyComp, head.cmyChannels())
        {
            fxiItem.m_cmy.append(cmyComp + fxi->address());
            qDebug() << "Add CMY comp at address:" << cmyComp + fxi->address();
        }

        if (head.masterIntensityChannel() != QLCChannel::invalid())
        {
            fxiItem.m_masterDimmer = fxi->address() + head.masterIntensityChannel();
            qDebug() << "Set master dimmer to:" << fxiItem.m_masterDimmer;
            fxiItem.m_back = new QGraphicsEllipseItem(this);
            fxiItem.m_back->setPen(QPen(Qt::white, 1));
            fxiItem.m_back->setBrush(QBrush(Qt::black));

        }
        else
        {
            fxiItem.m_masterDimmer = QLCChannel::invalid();
            fxiItem.m_back = NULL;
        }

        m_heads.append(fxiItem);
    }
}

void MonitorFixtureItem::setSize(QSize size)
{
    prepareGeometryChange();
    m_width = size.width();
    m_height = size.height();

    if (m_width < 5 || m_height < 5)
        return;

    // calculate the diameter of every single head
    double headArea = (m_width * m_height) / m_heads.count();
    double headSide = sqrt(headArea);
    int columns = (m_width / headSide) + 0.5;
    int rows = (m_height / headSide) + 0.5;

    // dirty workaround to correctly display right columns on one row
    if (rows == 1)
        columns = m_heads.count();

    //qDebug() << "Fixture columns:" << columns;

    if (columns > m_heads.count())
        columns = m_heads.count();

    if (rows < 1)
        rows = 1;
    if (columns < 1)
        columns = 1;

    double cellWidth = m_width / columns;
    double cellHeight = m_height / rows;
    double headDiam = (cellWidth < cellHeight)?cellWidth:cellHeight;
    
    int ypos = (cellHeight - headDiam) / 2;
    for (int i = 0; i < rows; i++)
    {
        int xpos = (cellWidth - headDiam) / 2;
        for (int j = 0; j < columns; j++)
        {
            int index = i * columns + j;
            if (index < m_heads.size())
            {
                QGraphicsEllipseItem *head = m_heads.at(index).m_item;
                head->setRect(xpos, ypos, headDiam, headDiam);
                head->setZValue(2);
                QGraphicsEllipseItem *back = m_heads.at(index).m_back;
                if (back != NULL)
                {
                    back->setRect(xpos, ypos, headDiam, headDiam);
                    back->setZValue(1);
                }
            }
            xpos += cellWidth;
        }
        ypos += cellHeight;
    }

    QFontMetrics fm(m_font);
    m_labelRect = fm.boundingRect(QRect(-10, m_height + 2, m_width + 20, 30),
                                  Qt::AlignHCenter | Qt::TextWrapAnywhere, m_name);
    update();
}

void MonitorFixtureItem::updateValues(const QByteArray & ua)
{
    foreach(FixtureHead head, m_heads)
    {
        uchar alpha = 255;
        if (head.m_masterDimmer != UINT_MAX /*QLCChannel::invalid()*/)
        {
            if (head.m_masterDimmer >= (quint32)ua.size())
                continue;
            alpha = ua.at(head.m_masterDimmer);
        }

        if (head.m_rgb.count() > 0)
        {
            uchar r = 0, g = 0, b = 0;
            if (head.m_rgb.at(0) < (quint32)ua.count())
                r = ua.at(head.m_rgb.at(0));
            if (head.m_rgb.at(1) < (quint32)ua.count())
                g = ua.at(head.m_rgb.at(1));
            if (head.m_rgb.at(2) < (quint32)ua.count())
                b = ua.at(head.m_rgb.at(2));
            head.m_item->setBrush(QBrush(QColor(r, g, b, alpha)));
        }
        else if (head.m_cmy.count() > 0)
        {
            uchar c = 0, m = 0, y = 0;
            if (head.m_cmy.at(0) < (quint32)ua.count())
                c = ua.at(head.m_cmy.at(0));
            if (head.m_cmy.at(1) < (quint32)ua.count())
                m = ua.at(head.m_cmy.at(1));
            if (head.m_cmy.at(2) < (quint32)ua.count())
                y = ua.at(head.m_cmy.at(2));
            QColor col = QColor::fromCmyk(c, m, y, 0);
            col.setAlpha(alpha);
            head.m_item->setBrush(QBrush(col));
        }
        else if (m_gelColor.isValid())
        {
            QColor col = m_gelColor;
            col.setAlpha(alpha);
            head.m_item->setBrush(QBrush(col));
        }
        else
        {
            head.m_item->setBrush(QBrush(QColor(255, 255, 255, alpha)));
        }
    }
}

void MonitorFixtureItem::showLabel(bool visible)
{
    prepareGeometryChange();
    m_labelVisibility = visible;
    update();
}

QRectF MonitorFixtureItem::boundingRect() const
{
    if (m_labelVisibility)
    {
        return QRectF(-10, 0, m_width + 20, m_height + m_labelRect.height() + 2);
    }
    else
        return QRectF(0, 0, m_width, m_height);
}

void MonitorFixtureItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (this->isSelected() == true)
        painter->setPen(QPen(Qt::yellow, 1));
    else
        painter->setPen(QPen(Qt::darkGray, 1));

    // draw chaser background
    painter->setBrush(QBrush(QColor(33, 33, 33)));
    painter->drawRect(0, 0, m_width, m_height);
    foreach (FixtureHead head, m_heads)
        head.m_item->update();

    if (m_labelVisibility)
    {
        painter->setFont(m_font);
        painter->drawRoundedRect(m_labelRect, 2, 2);
        painter->setPen(QPen(Qt::white, 1));
        painter->drawText(m_labelRect, Qt::AlignHCenter | Qt::TextWrapAnywhere, m_name);
    }
}

void MonitorFixtureItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    this->setSelected(true);
}

void MonitorFixtureItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    emit itemDropped(this);
}

void MonitorFixtureItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
}
