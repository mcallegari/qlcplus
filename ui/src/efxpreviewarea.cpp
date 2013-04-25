/*
  Q Light Controller
  efxpreviewarea.cpp

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

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QPen>

#include "efxpreviewarea.h"
#include "qlcmacros.h"

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
        m_timer.stop();
    }
}
