/*
  Q Light Controller Plus
  headeritems.cpp

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

#include <QPainter>

#include "headeritems.h"

/****************************************************************************
 *
 * Header item
 *
 ****************************************************************************/
ShowHeaderItem::ShowHeaderItem(int width)
    : m_width(width)
    , m_height(HEADER_HEIGHT)
    , m_timeStep(HALF_SECOND_WIDTH)
    , m_timeHit(2)
    , m_timeScale(3)
    , m_BPMValue(120)
    , m_type(Show::Time)
{
}

void ShowHeaderItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    emit itemClicked(event);
}

QRectF ShowHeaderItem::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

void ShowHeaderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // draw base background
    painter->setPen(QPen(QColor(100, 100, 100, 255), 1));
    painter->setBrush(QBrush(QColor(150, 150, 150, 255)));
    painter->drawRect(0, 0, m_width, 35);

    if (m_type > Show::Time)
        m_timeStep = ((float)(120 * HALF_SECOND_WIDTH) / (float)m_BPMValue) / (float)m_timeScale;

    // draw vertical timing lines and time labels
    int tmpSec = 0;
    for (int i = 0; i < m_width / m_timeStep; i++)
    {
        float xpos = ((float)i * m_timeStep) + 1;
        painter->setPen(QPen(QColor(250, 250, 250, 255), 1));
        if (i%m_timeHit == 0)
        {
            painter->drawLine(xpos, 20, xpos, 34);
            if (m_height > HEADER_HEIGHT)
            {
                painter->setPen(QPen(QColor(105, 105, 105, 255), 1));
                painter->drawLine(xpos, HEADER_HEIGHT, xpos, m_height);
            }
            painter->setPen(QPen(Qt::black, 1));
            if (m_type == Show::Time)
            {
                tmpSec = (i/2) * m_timeScale;
                if (tmpSec < 60)
                    painter->drawText(xpos - 4, 15, QString("%1s").arg(tmpSec));
                else
                {
                    int tmpMin = tmpSec / 60;
                    tmpSec = tmpSec - (tmpMin * 60);
                    painter->drawText(xpos - 4, 15, QString("%1m%2s").arg(tmpMin).arg(tmpSec));
                }
            }
            else
            {
                tmpSec++;
                painter->drawText(xpos - 4, 15, QString("%1").arg(tmpSec));
            }
        }
        else
        {
            if (m_timeStep > 5)
            {
                painter->drawLine(xpos, 25, xpos, 34);
                if (m_height > HEADER_HEIGHT)
                {
                    painter->setPen(QPen(QColor(105, 105, 105, 255), 1));
                    painter->drawLine(xpos, HEADER_HEIGHT, xpos, m_height);
                }
            }
        }
    }

}

void ShowHeaderItem::setTimeScale(int val)
{
    m_timeScale = val;
    update();
}

int ShowHeaderItem::getTimeScale()
{
    return m_timeScale;
}

void ShowHeaderItem::setTimeDivisionType(Show::TimeDivision type)
{
    if (type >= Show::Invalid)
        return;

    m_type = type;
    if (m_type == Show::Time)
    {
        m_timeStep = HALF_SECOND_WIDTH;
        m_timeHit = 2;
    }
    else
    {
        if (m_type == Show::BPM_4_4)
            m_timeHit = 4;
        else if (m_type == Show::BPM_3_4)
            m_timeHit = 3;
        else if (m_type == Show::BPM_2_4)
            m_timeHit = 2;
    }
    update();
}

Show::TimeDivision ShowHeaderItem::getTimeDivisionType()
{
    return m_type;
}

void ShowHeaderItem::setBPMValue(int value)
{
    if (value > 1)
    {
        m_BPMValue = value;
    }
    update();
}

int ShowHeaderItem::getHalfSecondWidth()
{
    return HALF_SECOND_WIDTH;
}

float ShowHeaderItem::getTimeDivisionStep()
{
    if (m_type > Show::Time && m_timeStep <= 5)
        return m_timeStep * m_timeHit;
    return m_timeStep;
}

void ShowHeaderItem::setWidth(int w)
{
    prepareGeometryChange();
    m_width = w;
}

void ShowHeaderItem::setHeight(int h)
{
    prepareGeometryChange();
    m_height = h;
}


/****************************************************************************
 *
 * Cursor item
 *
 ****************************************************************************/
ShowCursorItem::ShowCursorItem(int h)
    : m_height(h)
    , m_time(0)
{
}

void ShowCursorItem::setHeight(int height)
{
    prepareGeometryChange();
    m_height = height;
}

void ShowCursorItem::setTime(quint32 t)
{
    m_time = t;
}

quint32 ShowCursorItem::getTime()
{
    return m_time;
}

QRectF ShowCursorItem::boundingRect() const
{
    return QRectF(-5, 0, 10, m_height);
}

void ShowCursorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(QBrush(Qt::yellow, Qt::SolidPattern));
    painter->setPen(QPen(Qt::yellow, 1));
    QPolygonF CursorHead;
    CursorHead.append(QPointF(-5.0, 22.0));
    CursorHead.append(QPointF(5.0, 22.0));
    CursorHead.append(QPointF(5.0, 25.0));
    CursorHead.append(QPointF(0.0, 35.0));
    CursorHead.append(QPointF(-5.0, 25.0));
    CursorHead.append(QPointF(-5.0, 22.0));
    painter->drawPolygon(CursorHead);
    painter->setPen(Qt::NoPen);
    painter->drawRect(0, 35, 1, m_height - 35);
}
