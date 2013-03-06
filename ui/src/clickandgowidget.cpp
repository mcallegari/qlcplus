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

void ClickAndGoWidget::fillWithGradient(int r, int g, int b, QPainter *painter, int x)
{
    QColor top = Qt::black;
    QColor col(r, g , b);
    QColor bottom = Qt::white;

    QLinearGradient blackGrad(QPointF(0,0), QPointF(0, 127));
    blackGrad.setColorAt(0, top);
    blackGrad.setColorAt(1, col);
    QLinearGradient whiteGrad(QPointF(0,128), QPointF(0, 255));
    whiteGrad.setColorAt(0, col);
    whiteGrad.setColorAt(1, bottom);

    painter->fillRect(x, 0, x, 128, blackGrad);
    painter->fillRect(x, 128, x, 256, whiteGrad);
}

void ClickAndGoWidget::setupColorPicker()
{
    int r = 0xFF;
    int g = 0;
    int b = 0;
    int x = 30;
    int i = 0;
    int cw = 15;

    m_image = QImage(252 + 30, 256, QImage::Format_RGB32);
    QPainter painter(&m_image);

    // Draw the 20 default color squares
    painter.fillRect(0, 0, cw, 32, QColor(Qt::white));
    painter.fillRect(cw, 0, cw + cw, 32, QColor(Qt::black));
    painter.fillRect(0, 32, cw, 64, QColor(Qt::red));
    painter.fillRect(cw, 32, cw + cw, 64, QColor(Qt::darkRed));
    painter.fillRect(0, 64, cw, 96, QColor(Qt::green));
    painter.fillRect(cw, 64, cw + cw, 96, QColor(Qt::darkGreen));
    painter.fillRect(0, 96, cw, 128, QColor(Qt::blue));
    painter.fillRect(cw, 96, cw + cw, 128, QColor(Qt::darkBlue));
    painter.fillRect(0, 128, cw, 160, QColor(Qt::cyan));
    painter.fillRect(cw, 128, cw + cw, 160, QColor(Qt::darkCyan));
    painter.fillRect(0, 160, cw, 192, QColor(Qt::magenta));
    painter.fillRect(cw, 160, cw + cw, 192, QColor(Qt::darkMagenta));
    painter.fillRect(0, 192, cw, 224, QColor(Qt::yellow));
    painter.fillRect(cw, 192, cw + cw, 224, QColor(Qt::darkYellow));
    painter.fillRect(0, 224, cw, 256, QColor(Qt::gray));
    painter.fillRect(cw, 224, cw + cw, 256, QColor(Qt::darkGray));

    // R: 255  G:  0  B:   0
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        g+=6;
        if (g == 252) g = 255;
    }
    x+=42;
    // R: 255  G: 255  B:   0
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        r-=6;
        if (r < 6) r = 0;
    }
    x+=42;
    // R: 0  G: 255  B:  0
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        b+=6;
        if (b == 252) b = 255;
    }
    x+=42;
    // R: 0  G: 255  B:  255
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        g-=6;
        if (g < 6) g = 0;
    }
    x+=42;
    // R: 0  G:  0  B:  255
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        r+=6;
        if (r == 252) r = 255;
    }
    x+=42;
    // R: 255  G:  0  B:  255
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        b-=6;
        if (b < 6) b = 0;
    }
    // R: 255  G:  0  B:  0
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
    else if (type == VCSlider::RGB)
    {
        setupColorPicker();
    }
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
        return QSize(252 + 30, 256);

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
    else if (m_type == VCSlider::RGB)
    {
        emit colorChanged(m_image.pixel(event->x(), event->y()));
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



