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
    , m_gradientBg(false)
    , m_bgAlpha(255)
{
    QPalette p = palette();
    p.setColor(QPalette::Window, p.color(QPalette::Base));
    setPalette(p);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

EFXPreviewArea::~EFXPreviewArea()
{
}

void EFXPreviewArea::setPolygon(const QPolygonF& polygon)
{
    m_original = polygon;
    m_scaled = scale(m_original, size());
}

int EFXPreviewArea::polygonsCount() const
{
    return m_original.size();
}

void EFXPreviewArea::setFixturePolygons(const QVector<QPolygonF> &fixturePoints)
{
    m_originalFixturePoints.resize(fixturePoints.size());
    m_fixturePoints.resize(fixturePoints.size());

    for (int i = 0; i < m_fixturePoints.size(); ++i)
    {
        m_originalFixturePoints[i] = QPolygonF(fixturePoints[i]);
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
    if (m_iter < m_scaled.size())
        m_iter++;

    repaint();
}

QPolygonF EFXPreviewArea::scale(const QPolygonF& poly, const QSize& target)
{
    QPolygonF scaled;
    for (int i = 0; i < poly.size(); i++)
    {
        QPointF pt = poly.at(i);
        pt.setX((int) SCALE(qreal(pt.x()), qreal(0), qreal(255), qreal(0), qreal(target.width())));
        pt.setY((int) SCALE(qreal(pt.y()), qreal(0), qreal(255), qreal(0), qreal(target.height())));
        scaled << pt;
    }

    return scaled;
}

void EFXPreviewArea::resizeEvent(QResizeEvent* e)
{
    m_scaled = scale(m_original, e->size());

    for (int i = 0; i < m_fixturePoints.size(); ++i)
        m_fixturePoints[i] = scale(m_originalFixturePoints[i], e->size());

    QWidget::resizeEvent(e);
}

void EFXPreviewArea::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    QPen pen;
    QPointF point;
    QColor color = palette().color(QPalette::Base);
    if (m_gradientBg)
        painter.drawImage(painter.window(), Gradient::getRGBGradient(256, 256));
    else
    {
        color.setAlpha(m_bgAlpha);
        painter.fillRect(rect(), color);
    }

    /* Crosshairs */
    color = palette().color(QPalette::Mid);
    painter.setPen(color);
    // Do division by two instead with a bitshift to prevent rounding
    painter.drawLine(width() >> 1, 0, width() >> 1, height());
    painter.drawLine(0, height() >> 1, width(), height() >> 1);

    /* Plain points with text color */
    color = palette().color(QPalette::Text);
    pen.setColor(color);
    painter.setPen(pen);
    painter.drawPolygon(m_scaled);

    // Draw the points from the point array
    if (m_iter < m_scaled.size() && m_iter >= 0)
    {
        painter.setBrush(Qt::white);
        pen.setColor(Qt::black);

        // drawing fixture positions from the end,
        // so that lower numbers are on top
        for (int i = m_fixturePoints.size() - 1; i >= 0; --i)
        {
            point = m_fixturePoints.at(i).at(m_iter);
            painter.drawEllipse(point, 8, 8);
            painter.drawText(point.x() - 4, point.y() + 5, QString::number(i+1));
        }
    }
    else
    {
        //Change old behaviour from stop to restart
        restart();
    }
}

void EFXPreviewArea::restart()
{
    m_iter = 0;
}

void EFXPreviewArea::showGradientBackground(bool enable)
{
    m_gradientBg = enable;
    repaint();
}

void EFXPreviewArea::setBackgroundAlpha(int alpha)
{
    m_bgAlpha = alpha;
}
