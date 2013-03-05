/*
  Q Light Controller Plus
  clickandgowidget.cpp

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

#include <QPainter>
#include <QImage>

#include "clickandgowidget.h"
#include "vcslider.h"

ClickAndGoWidget::ClickAndGoWidget(QWidget *parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_type = VCSlider::None;
    m_linearColor = false;

    resize(256,50);
}

void ClickAndGoWidget::setupGradient(QColor end)
{
    QColor begin = Qt::black;

    QLinearGradient linearGrad(QPointF(10,0), QPointF(266, 0));
    linearGrad.setColorAt(0, begin);
    linearGrad.setColorAt(1, end);

    // create image and fill it with gradient
    m_image = QImage(276, 40, QImage::Format_RGB32);
    QPainter painter(&m_image);
    painter.fillRect(m_image.rect(), linearGrad);

    m_linearColor = true;
}

void ClickAndGoWidget::setType(int type)
{
    if (type == m_type)
        return;

    m_linearColor = false;
    if (type == VCSlider::None)
    {
        m_image = QImage();
    }
    else if (type == VCSlider::Red)
        setupGradient(Qt::red);
    else if (type == VCSlider::Green)
        setupGradient(Qt::green);
    else if (type == VCSlider::Blue)
        setupGradient(Qt::blue);
    else if (type == VCSlider::Cyan)
        setupGradient(Qt::cyan);
    else if (type == VCSlider::Magenta)
        setupGradient(Qt::magenta);
    else if (type == VCSlider::Yellow)
        setupGradient(Qt::yellow);
    else if (type == VCSlider::White)
        setupGradient(Qt::white);
    else if (type == VCSlider::Gobo)
    {

    }
    else if (type == VCSlider::Preset)
    {

    }

    m_type = type;
}

int ClickAndGoWidget::getType()
{
    return m_type;
}

QColor ClickAndGoWidget::getColorAt(uchar pos)
{
    if (m_linearColor == true)
    {
        QRgb col = m_image.pixel(10 + pos, 10);
        return QColor(col);
    }
    return QColor(0,0,0);
}

QSize ClickAndGoWidget::sizeHint() const
{
    if (m_linearColor == true)
        return QSize(276, 40);
    else if (m_type == VCSlider::RGB)
        return QSize(256, 256);

    return QSize(10, 10);
}

void ClickAndGoWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_linearColor == true)
    {
        if (event->x() <= 10)
            emit levelChanged(0);
        else if (event->x() > 10 && event->x() < 256)
            emit levelChanged((uchar)(event->x() - 10));
        else
            emit levelChanged(255);
    }
    QWidget::mousePressEvent(event);
}

void ClickAndGoWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ClickAndGoWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), m_image);
}



