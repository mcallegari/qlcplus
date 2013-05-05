/*
  Q Light Controller Plus
  clickandgoslider.cpp

  Copyright (c) Massimo Callegari

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

#include "clickandgoslider.h"
#include <QStyleOptionSlider>
#include <QStyle>

ClickAndGoSlider::ClickAndGoSlider(QWidget *parent) : QSlider(parent)
{
}

void ClickAndGoSlider::mousePressEvent ( QMouseEvent * event )
{
    if (event->modifiers() == Qt::ControlModifier)
    {
        emit controlClicked();
        return;
    }

    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    if (event->button() == Qt::LeftButton && // react only to left button press
        sr.contains(event->pos()) == false) // check if the click is not over the slider's handle
    {
        int newVal = minimum() + ((maximum()-minimum()) * (height()-event->y())) / height();
        if (invertedAppearance() == true)
            setValue( maximum() - newVal );
        else
            setValue(newVal);

        event->accept();
    }
    QSlider::mousePressEvent(event);
}
