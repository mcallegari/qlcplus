/*
  Q Light Controller
  efxpreviewarea.cpp

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

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QPen>

#include "efxpreviewarea.h"
#include "qlcmacros.h"
#include "gradient.h"

EFXPreviewArea::EFXPreviewArea(QWidget* parent)
    : QWidget(parent)
    , m_timer(this)
    , m_iter(0)
{
    QPalette p = palette();
    p.setColor(QPalette::Window, p.color(QPalette::Base));
    setPalette(p);

    setAutoFillBackground(true);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

EFXPreviewArea::~EFXPreviewArea()
{
}

void EFXPreviewArea::setPoints(const QVector <QPoint>& points)
{
    m_original = QPolygon(points);
    m_points = scale(m_original, size());
}

void EFXPreviewArea::setFixturePoints(const QVector<QVector<QPoint> >& fixturePoints)
{
    m_originalFixturePoints.resize(fixturePoints.size());
    m_fixturePoints.resize(fixturePoints.size());

    for(int i = 0; i < m_fixturePoints.size(); ++i)
    {
        m_originalFixturePoints[i] = QPolygon(fixturePoints[i]);
        m_fixturePoints[i] = scale(m_originalFixturePoints[i], size());
    }
}

void EFXPreviewArea::draw(int timerInterval)
{
    m_timer.stop();

    m_iter = 0;
    m_timer.start(timerInterval);
}

void EFXPreviewArea::slotTimeout()
{
    repaint();
}

QPolygon EFXPreviewArea::scale(const QPolygon& poly, const QSize& target)
{
    QPolygon scaled(poly.size());
    for (int i = 0; i < poly.size(); i++)
    {
        QPoint pt = poly.point(i);
        pt.setX((int) SCALE(qreal(pt.x()), qreal(0), qreal(255), qreal(0), qreal(target.width())));
        pt.setY((int) SCALE(qreal(pt.y()), qreal(0), qreal(255), qreal(0), qreal(target.height())));
        scaled.setPoint(i, pt);
    }

    return scaled;
}

void EFXPreviewArea::resizeEvent(QResizeEvent* e)
{
    m_points = scale(m_original, e->size());

    for(int i = 0; i < m_fixturePoints.size(); ++i)
    {
        m_fixturePoints[i] = scale(m_originalFixturePoints[i], e->size());
    }
    QWidget::resizeEvent(e);
}

void EFXPreviewArea::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    QPen pen;
    QPoint point;
    QColor color;

    //painter.drawImage(QPoint(0, 0), Gradient::getRGBGradient ());
    painter.drawImage(painter.window (), Gradient::getRGBGradient ());

    /* Crosshairs */
    color = palette().color(QPalette::Mid);
    painter.setPen(color);
    // Do division by two instead with a bitshift to prevent rounding
    painter.drawLine(width() >> 1, 0, width() >> 1, height());
    painter.drawLine(0, height() >> 1, width(), height() >> 1);

    if (m_iter < m_points.size())
        m_iter++;

    /* Plain points with text color */
    color = palette().color(QPalette::Text);
    pen.setColor(color);
    painter.setPen(pen);
    painter.drawPolygon(m_points);

    // Draw the points from the point array
    if (m_iter < m_points.size() && m_iter >= 0)
    {
        /*
        // draw origin
        color = color.lighter(100 + (m_points.size() / 100));
        pen.setColor(color);
        painter.setPen(pen);
        point = m_points.point(m_iter);
        painter.drawEllipse(point.x() - 4, point.y() - 4, 8, 8);
        */

        painter.setBrush(Qt::white);
        pen.setColor(Qt::black);

        // draw fixture positions

        // drawing from the end -- so that lower numbers are on top
        for (int i = m_fixturePoints.size() - 1; i >= 0; --i)
        {
	    point = m_fixturePoints.at(i).at(m_iter);
            
            painter.drawEllipse(point, 8, 8);
	    painter.drawText(point.x() - 4, point.y() + 5, QString::number(i+1));
        }
    }
    else
    {
        //m_timer.stop();

        //Change old behaviour from stop to restart
        restart();
    }
}

void EFXPreviewArea::restart ()
{
    m_iter = 0;
}
