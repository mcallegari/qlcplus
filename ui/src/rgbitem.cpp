/*
  Q Light Controller
  rgbitem.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QDebug>
#include <QBrush>
#include <QPen>

#include "mastertimer.h"
#include "qlcmacros.h"
#include "rgbitem.h"

RGBItem::RGBItem(QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent)
    , m_elapsed(0)
{
}

RGBItem::~RGBItem()
{
}

void RGBItem::setColor(QRgb rgb)
{
    m_oldColor = brush().color();
    m_color = QColor(rgb);
    m_elapsed = 0;
}

QRgb RGBItem::color() const
{
    return m_color.rgb();
}

void RGBItem::draw(uint ms)
{
    if (ms == 0)
    {
        setBrush(m_color);
    }
    else if (m_elapsed <= ms)
    {
        int red, green, blue;
        if (m_oldColor.red() < m_color.red())
            red = SCALE(qreal(m_elapsed), qreal(0), qreal(ms), qreal(m_oldColor.red()), qreal(m_color.red()));
        else
            red = SCALE(qreal(m_elapsed), qreal(ms), qreal(0), qreal(m_color.red()), qreal(m_oldColor.red()));
        red = CLAMP(red, 0, 255);

        if (m_oldColor.green() < m_color.green())
            green = SCALE(qreal(m_elapsed), qreal(0), qreal(ms), qreal(m_oldColor.green()), qreal(m_color.green()));
        else
            green = SCALE(qreal(m_elapsed), qreal(ms), qreal(0), qreal(m_color.green()), qreal(m_oldColor.green()));
        green = CLAMP(green, 0, 255);

        if (m_oldColor.blue() < m_color.blue())
            blue = SCALE(qreal(m_elapsed), qreal(0), qreal(ms), qreal(m_oldColor.blue()), qreal(m_color.blue()));
        else
            blue = SCALE(qreal(m_elapsed), qreal(ms), qreal(0), qreal(m_color.blue()), qreal(m_oldColor.blue()));
        blue = CLAMP(blue, 0, 255);

        setBrush(QColor(red, green, blue));
    }

    m_elapsed += MasterTimer::tick();
}
