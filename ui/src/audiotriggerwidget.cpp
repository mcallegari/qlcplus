/*
  Q Light Controller Plus
  audiotriggerwidget.cpp

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
#include <QDebug>

#include "audiotriggerwidget.h"

AudioTriggerWidget::AudioTriggerWidget(QWidget *parent) :
    QWidget(parent)
  , m_spectrumBands(NULL)
{
    m_barsNumber = 0;
}

void AudioTriggerWidget::setBarsNumber(int num)
{
    m_barsNumber = num;
    if (m_spectrumBands)
        delete m_spectrumBands;
    m_spectrumBands = new double[m_barsNumber];
    update();
}

void AudioTriggerWidget::displaySpectrum(double *spectrumData, quint32 power)
{
    qDebug() << "[displaySpectrum] power: " << power;
    for (int i = 0; i < m_barsNumber; i++)
        m_spectrumBands[i] = spectrumData[i];
    m_powerValue = power;
    update();
}

void AudioTriggerWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    m_barWidth = (width() - 10) / (m_barsNumber + 1);
}

void AudioTriggerWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    if (m_barsNumber == 0)
        return;

    QPainter painter(this);

    painter.setPen(QPen(Qt::darkGray, 2));
    painter.setBrush(QBrush(Qt::black));
    painter.drawRect(0, 0, m_barWidth * m_barsNumber, height());

    painter.setBrush(QBrush(Qt::lightGray));
    painter.drawRect(width() - m_barWidth, 0, m_barWidth, height());

    float xpos = 1;
    painter.setBrush(QBrush(Qt::yellow));

    for (int i = 0; i < m_barsNumber; i++)
    {
        painter.setPen(QPen(Qt::NoPen));
        painter.drawRect(xpos, height() - m_spectrumBands[i], m_barWidth - 1, m_spectrumBands[i]);
        painter.setPen(QPen(Qt::lightGray, 1));
        painter.drawLine(xpos + m_barWidth, 0, xpos + m_barWidth, height());

        xpos += m_barWidth;
    }

    painter.setPen(QPen(Qt::NoPen));
    painter.setBrush(QBrush(Qt::green));
    painter.drawRect(width() - m_barWidth, height() - m_powerValue, m_barWidth - 1, m_powerValue);
}
