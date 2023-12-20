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

#include "qlcmacros.h"
#include "rgbitem.h"

RGBItem::RGBItem(QAbstractGraphicsShapeItem* graphicsItem)
    : m_elapsed(0)
    , m_graphicsItem(graphicsItem)
{
}

void RGBItem::setColor(QRgb rgb)
{
    m_oldColor = m_graphicsItem->brush().color();
    m_color = QColor(rgb);
    m_elapsed = 0;
}

QRgb RGBItem::color() const
{
    return m_color.rgb();
}

void RGBItem::draw(uint elapsedMs, uint targetMs)
{
    m_elapsed += elapsedMs;

    if (targetMs == 0)
    {
        m_graphicsItem->setBrush(m_color);
    }
    else if (m_elapsed <= targetMs)
    {
        int red, green, blue;
        if (m_oldColor.red() < m_color.red())
            red = SCALE(qreal(m_elapsed), qreal(0), qreal(targetMs), qreal(m_oldColor.red()), qreal(m_color.red()));
        else
            red = SCALE(qreal(m_elapsed), qreal(targetMs), qreal(0), qreal(m_color.red()), qreal(m_oldColor.red()));
        red = CLAMP(red, 0, 255);

        if (m_oldColor.green() < m_color.green())
            green = SCALE(qreal(m_elapsed), qreal(0), qreal(targetMs), qreal(m_oldColor.green()), qreal(m_color.green()));
        else
            green = SCALE(qreal(m_elapsed), qreal(targetMs), qreal(0), qreal(m_color.green()), qreal(m_oldColor.green()));
        green = CLAMP(green, 0, 255);

        if (m_oldColor.blue() < m_color.blue())
            blue = SCALE(qreal(m_elapsed), qreal(0), qreal(targetMs), qreal(m_oldColor.blue()), qreal(m_color.blue()));
        else
            blue = SCALE(qreal(m_elapsed), qreal(targetMs), qreal(0), qreal(m_color.blue()), qreal(m_oldColor.blue()));
        blue = CLAMP(blue, 0, 255);

        m_graphicsItem->setBrush(QColor(red, green, blue));
    }
    else
        m_graphicsItem->setBrush(m_color);
}

QAbstractGraphicsShapeItem* RGBItem::graphicsItem() const
{
    return m_graphicsItem.data();
}
