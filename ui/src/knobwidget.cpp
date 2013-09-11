/*
  Q Light Controller Plus
  knobwidget.cpp

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

void KnobWidget::prepareCursor()
{
    int shortSide = height();
    if (width() < shortSide)
        shortSide = width();
    float arcWidth = shortSide / 15;
    float dialSize = shortSide - (arcWidth * 2);
    float cursor_radius = dialSize / 15;
    if (cursor_radius < 5)
        cursor_radius = 5;

    QPainter fgP(m_cursor);
    fgP.setRenderHints(QPainter::Antialiasing);
    fgP.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    fgP.fillRect(m_cursor->rect(), QColor(0, 0, 0, 0));

    fgP.setCompositionMode(QPainter::CompositionMode_Source);
    if (isEnabled() == false)
        fgP.setBrush(Qt::gray);
    else
        fgP.setBrush(Qt::green);
    fgP.drawEllipse(QPointF(dialSize / 2 - (arcWidth * 1.5), dialSize - (arcWidth * 2.2)), cursor_radius, cursor_radius);
}

void KnobWidget::resizeEvent(QResizeEvent *e)
{
    QDial::resizeEvent(e);

    qDebug() << "Resize event";
    int shortSide = height();
    if (width() < shortSide)
        shortSide = width();
    float arcWidth = shortSide / 15;
    float dialSize = shortSide - (arcWidth * 2);
    float radius = dialSize / 2;

    QLinearGradient linearGrad(QPointF(0,0), QPointF(0, dialSize));
    linearGrad.setColorAt(0, Qt::darkGray);
    linearGrad.setColorAt(1, Qt::gray);
    QLinearGradient linearGrad2(QPointF(0,0), QPointF(0, dialSize));
    linearGrad2.setColorAt(0, Qt::gray);
    linearGrad2.setColorAt(1, Qt::darkGray);

    m_background = new QPixmap(dialSize, dialSize);
    m_background->fill(Qt::transparent);
    m_cursor = new QPixmap(dialSize, dialSize);
    m_cursor->fill(Qt::transparent);

    QPainter bgP(m_background);
    bgP.setRenderHints(QPainter::Antialiasing);
    bgP.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    bgP.fillRect(m_background->rect(), QColor(0, 0, 0, 0));

    bgP.setCompositionMode(QPainter::CompositionMode_Source);
    bgP.setBrush(linearGrad);
    bgP.drawEllipse(QPointF(dialSize / 2, radius), radius, radius);
    bgP.setBrush(linearGrad2);
    bgP.setPen(Qt::NoPen);
    bgP.drawEllipse(QPointF(dialSize / 2, radius), radius - (arcWidth * 2), radius - (arcWidth * 2));

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
    float degrees = SCALE(value(), minimum(), maximum(),
                          0.0, 330.0);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawPixmap(pixPoint, *m_background);

    QPixmap rotNeedle = rotatePix(m_cursor, degrees);
    painter.drawPixmap(pixPoint, rotNeedle);

    QRectF valRect = QRectF(pixPoint.x() - (arcWidth / 2) + 1, arcWidth / 2 + 1,
                             m_background->width() + arcWidth - 2, m_background->height() + arcWidth - 2);

    int penWidth = arcWidth;
    if (arcWidth <= 6)
        penWidth = 6;

    painter.setPen(QPen(QColor(100, 100, 100, 255), penWidth - 1));
    painter.drawArc(valRect, -105 * 16, -330 * 16);
    QColor col = Qt::green;
    if (this->isEnabled() == false)
        col = Qt::lightGray;
    painter.setPen(QPen(col, penWidth - 3));
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

