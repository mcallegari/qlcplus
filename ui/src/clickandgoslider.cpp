/*
  Q Light Controller Plus
  clickandgoslider.cpp

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

#include "clickandgoslider.h"
#include <QStyleOptionSlider>
#include <QStyle>

ClickAndGoSlider::ClickAndGoSlider(QWidget *parent) : QSlider(parent)
{
}

void ClickAndGoSlider::setSliderStyleSheet(const QString &styleSheet)
{
    if(isVisible())
        QSlider::setStyleSheet(styleSheet);
    else
        m_styleSheet = styleSheet;
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
        int newVal = 0;
        if (orientation() == Qt::Vertical)
            newVal = minimum() + ((maximum() - minimum()) * (height() - event->y())) / height();
        else
            newVal = minimum() + ((maximum() - minimum()) * event->x()) / width();

        if (invertedAppearance() == true)
            setValue( maximum() - newVal );
        else
            setValue(newVal);

        event->accept();
    }
    QSlider::mousePressEvent(event);
}

void ClickAndGoSlider::showEvent(QShowEvent *)
{
    if (m_styleSheet.isEmpty() == false)
    {
        setSliderStyleSheet(m_styleSheet);
        m_styleSheet = "";
    }
}
