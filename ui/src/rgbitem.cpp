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

template<typename T_QGraphicsItem>
RGBItem<T_QGraphicsItem>::RGBItem(QGraphicsItem* parent)
    : T_QGraphicsItem(parent)
    , m_elapsed(0)
{
}

template<typename T_QGraphicsItem>
void RGBItem<T_QGraphicsItem>::setColor(QRgb rgb)
{
    m_oldColor = this->brush().color();
    m_color = QColor(rgb);
    m_elapsed = 0;
}

template<typename T_QGraphicsItem>
QRgb RGBItem<T_QGraphicsItem>::color() const
{
    return m_color.rgb();
}

template<typename T_QGraphicsItem>
void RGBItem<T_QGraphicsItem>::draw(uint elapsedMs, uint targetMs)
{
    m_elapsed += elapsedMs;

    if (targetMs == 0)
    {
        this->setBrush(m_color);
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

        this->setBrush(QColor(red, green, blue));
    }
    else
        this->setBrush(m_color);

//    qDebug() << Q_FUNC_INFO << m_elapsed << this->brush().color().red() << this->brush().color().green() << this->brush().color().blue();
}

// Force instantiation of templates
template class RGBItem<QGraphicsEllipseItem>;
template class RGBItem<QGraphicsRectItem>;
