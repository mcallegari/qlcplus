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
#include <QPainter>
#include <QStyle>

ClickAndGoSlider::ClickAndGoSlider(QWidget *parent) : QSlider(parent)
{
    m_shadowLevel = -1;
}

void ClickAndGoSlider::setSliderStyleSheet(const QString &styleSheet)
{
    if (isVisible())
        QSlider::setStyleSheet(styleSheet);
    else
        m_styleSheet = styleSheet;
}

void ClickAndGoSlider::setShadowLevel(int level)
{
    m_shadowLevel = level;
    update();
}

void ClickAndGoSlider::mousePressEvent(QMouseEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier)
    {
        emit controlClicked();
        return;
    }

    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    if (e->button() == Qt::LeftButton && // react only to left button press
        sr.contains(e->pos()) == false) // check if the click is not over the slider's handle
    {
        int newVal = 0;
        if (orientation() == Qt::Vertical)
            newVal = minimum() + ((maximum() - minimum()) * (height() - e->pos().y())) / height();
        else
            newVal = minimum() + ((maximum() - minimum()) * e->pos().x()) / width();

        setSliderDown(true);
        if (invertedAppearance() == true)
            setValue(maximum() - newVal);
        else
            setValue(newVal);
        setSliderDown(false);

        e->accept();
    }
    QSlider::mousePressEvent(e);
}

void ClickAndGoSlider::wheelEvent(QWheelEvent *e)
{
    setSliderDown(true);
    QSlider::wheelEvent(e);
    setSliderDown(false);
}

void ClickAndGoSlider::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)
        setSliderDown(true);
    QSlider::keyPressEvent(e);
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)
        setSliderDown(false);
}

void ClickAndGoSlider::paintEvent(QPaintEvent *e)
{
    if (m_shadowLevel >= 0)
    {
        QPainter p(this);
        int levHeight = ((float)height() / 255.0) * m_shadowLevel;
        p.drawRect(width() - 6, 0, width(), height());
        p.fillRect(width() - 5, 0, width() - 1, height(), QColor(Qt::darkGray));
        if (invertedAppearance())
            p.fillRect(width() - 5, 0, width() - 1, levHeight, QColor(Qt::green));
        else
            p.fillRect(width() - 5, height() - levHeight, width() - 1, height(), QColor(Qt::green));
    }

    QSlider::paintEvent(e);
}

void ClickAndGoSlider::showEvent(QShowEvent *)
{
    if (m_styleSheet.isEmpty() == false)
    {
        setSliderStyleSheet(m_styleSheet);
        m_styleSheet = "";
    }
}
