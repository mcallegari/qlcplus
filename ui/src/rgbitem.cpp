/*
  Q Light Controller
  rgbitem.cpp

  Copyright (c) Heikki Junnila

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
#include <QBrush>
#include <QPen>

#include "mastertimer.h"
#include "qlcmacros.h"
#include "rgbitem.h"

static QColor getColor(QColor oldColor, QColor color, uint ms, uint elapsed)
{
    if (ms == 0)
    {
        return color;
    }
    else if (elapsed <= ms)
    {
        int red, green, blue;
        if (oldColor.red() < color.red())
            red = SCALE(qreal(elapsed), qreal(0), qreal(ms), qreal(oldColor.red()), qreal(color.red()));
        else
            red = SCALE(qreal(elapsed), qreal(ms), qreal(0), qreal(color.red()), qreal(oldColor.red()));
        red = CLAMP(red, 0, 255);

        if (oldColor.green() < color.green())
            green = SCALE(qreal(elapsed), qreal(0), qreal(ms), qreal(oldColor.green()), qreal(color.green()));
        else
            green = SCALE(qreal(elapsed), qreal(ms), qreal(0), qreal(color.green()), qreal(oldColor.green()));
        green = CLAMP(green, 0, 255);

        if (oldColor.blue() < color.blue())
            blue = SCALE(qreal(elapsed), qreal(0), qreal(ms), qreal(oldColor.blue()), qreal(color.blue()));
        else
            blue = SCALE(qreal(elapsed), qreal(ms), qreal(0), qreal(color.blue()), qreal(oldColor.blue()));
        blue = CLAMP(blue, 0, 255);

        return QColor(red, green, blue);
    }
    return QColor();
}

/************************************************************************
 * RGB Circle Item
 ************************************************************************/

RGBCircleItem::RGBCircleItem(QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent)
    , m_elapsed(0)
{
}

void RGBCircleItem::setColor(QRgb rgb)
{
    m_oldColor = brush().color();
    m_color = QColor(rgb);
    m_elapsed = 0;
}

QRgb RGBCircleItem::color() const
{
    return m_color.rgb();
}

void RGBCircleItem::draw(uint ms)
{
    setBrush(getColor(m_oldColor, m_color, ms, m_elapsed));
    m_elapsed += MasterTimer::tick();
}

/************************************************************************
 * RGB Rect Item
 ************************************************************************/

RGBRectItem::RGBRectItem(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
    , m_elapsed(0)
{
}

void RGBRectItem::setColor(QRgb rgb)
{
    m_oldColor = brush().color();
    m_color = QColor(rgb);
    m_elapsed = 0;
}

QRgb RGBRectItem::color() const
{
    return m_color.rgb();
}

void RGBRectItem::draw(uint ms)
{
    setBrush(getColor(m_oldColor, m_color, ms, m_elapsed));
    m_elapsed += MasterTimer::tick();
}

