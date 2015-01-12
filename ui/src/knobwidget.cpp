/*
  Q Light Controller Plus
  knobwidget.cpp

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

#include <QDebug>
#include <QPainter>
#include <QTransform>

#include "qlcmacros.h"
#include "knobwidget.h"

KnobWidget::KnobWidget(QWidget *parent) : QDial(parent)
{
    m_background = new QPixmap();
    m_cursor = new QPixmap();
    setWrapping(false);
    setMinimum(0);
    setMaximum(UCHAR_MAX);
    m_gradStartColor = Qt::darkGray;
    m_gradEndColor = Qt::gray;
}

KnobWidget::~KnobWidget()
{
    delete m_background;
    delete m_cursor;
}

void KnobWidget::setEnabled(bool status)
{
    QDial::setEnabled(status);
    prepareCursor();
}

void KnobWidget::setColor(QColor color)
{
    m_gradStartColor = color;
    m_gradEndColor = color.lighter(150);
    prepareBody();
    update();
}

void KnobWidget::prepareCursor()
{
    int shortSide = height();
    if (width() < shortSide)
        shortSide = width();
    float arcWidth = shortSide / 15;
    float dialSize = shortSide - (arcWidth * 2);
    float cursor_radius = dialSize / 15;
    if (cursor_radius < 3)
        cursor_radius = 3;

    QPainter fgP(m_cursor);
    fgP.setRenderHints(QPainter::Antialiasing);
    //fgP.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    fgP.fillRect(m_cursor->rect(), Qt::transparent);

    //fgP.setCompositionMode(QPainter::CompositionMode_Source);
    if (isEnabled() == false)
        fgP.setBrush(Qt::gray);
    else
        fgP.setBrush(Qt::green);
    fgP.drawEllipse(QPointF(dialSize / 2 - (arcWidth * 1.5), dialSize - (arcWidth * 2.2)),
                    cursor_radius, cursor_radius);
}

void KnobWidget::prepareBody()
{
    int shortSide = height();
    if (width() < shortSide)
        shortSide = width();
    float arcWidth = shortSide / 15;
    float dialSize = shortSide - (arcWidth * 2);
    float radius = dialSize / 2;

    QLinearGradient linearGrad(QPointF(0,0), QPointF(0, dialSize));
    linearGrad.setColorAt(0, m_gradStartColor);
    linearGrad.setColorAt(1, m_gradEndColor);
    QLinearGradient linearGrad2(QPointF(0,0), QPointF(0, dialSize));
    linearGrad2.setColorAt(0, m_gradEndColor);
    linearGrad2.setColorAt(1, m_gradStartColor);

    m_background = new QPixmap(dialSize, dialSize);
    m_background->fill(Qt::transparent);
    m_cursor = new QPixmap(dialSize, dialSize);
    m_cursor->fill(Qt::transparent);

    QPainter bgP(m_background);
    bgP.setRenderHints(QPainter::Antialiasing);
    bgP.fillRect(m_background->rect(), Qt::transparent);

    bgP.setBrush(linearGrad);
    bgP.drawEllipse(QPointF(dialSize / 2, radius), radius, radius);
    bgP.setBrush(linearGrad2);
    bgP.setPen(Qt::NoPen);
    bgP.drawEllipse(QPointF(dialSize / 2, radius), radius - (arcWidth * 2), radius - (arcWidth * 2));
}

void KnobWidget::resizeEvent(QResizeEvent *e)
{
    QDial::resizeEvent(e);

    prepareBody();
    prepareCursor();
}

void KnobWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    int shortSide = height();
    if (width() < shortSide)
        shortSide = width();
    float arcWidth = shortSide / 15;
    QPointF pixPoint = QPointF(((width() - m_background->width()) / 2), arcWidth);

    QPainter painter(this);
    float degrees = 0.0;
    if (invertedAppearance())
        degrees = SCALE(value(), minimum(), maximum(), 330.0, 0.0);
    else
        degrees = SCALE(value(), minimum(), maximum(), 0.0, 330.0);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawPixmap(pixPoint, *m_background);

    QPixmap rotNeedle = rotatePix(m_cursor, degrees);
    painter.drawPixmap(pixPoint, rotNeedle);

    QRectF valRect = QRectF(pixPoint.x() - (arcWidth / 2) + 1, arcWidth / 2 + 1,
                             m_background->width() + arcWidth - 2, m_background->height() + arcWidth - 2);

    int penWidth = arcWidth;
    if (arcWidth <= 5)
        penWidth = 5;

    painter.setPen(QPen(QColor(100, 100, 100, 255), penWidth - 1));
    painter.drawArc(valRect, -105 * 16, -330 * 16);
    QColor col = isEnabled() ? Qt::green : Qt::lightGray;
    painter.setPen(QPen(col, penWidth - 3));
    if (invertedAppearance())
        painter.drawArc(valRect, -75 * 16, (330-degrees) * 16);
    else
        painter.drawArc(valRect, -105 * 16, -degrees * 16);
}

QPixmap KnobWidget::rotatePix(QPixmap *p_pix, float p_deg)
{
    // perform rotation, transforming around the center of the image
    QTransform trans;
    trans.translate(p_pix->width()/2.0 , p_pix->height()/2.0);
    trans.rotate(p_deg);
    trans.translate(-p_pix->width()/2.0 , -p_pix->height()/2.0);
    QPixmap outPix = p_pix->transformed(trans, Qt::SmoothTransformation);

    // re-crop to original size
    int xOffset = (outPix.width() - p_pix->width()) / 2;
    int yOffset = (outPix.height() - p_pix->height()) / 2;
    outPix = outPix.copy(xOffset, yOffset, p_pix->width(), p_pix->height());

    return outPix;

}

