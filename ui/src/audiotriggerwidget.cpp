/*
  Q Light Controller Plus
  audiotriggerwidget.cpp

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

#include <QPainter>
#include <QDebug>
#include <qmath.h>

#include "audiotriggerwidget.h"
#include "qlcmacros.h"

AudioTriggerWidget::AudioTriggerWidget(QWidget *parent) :
    QWidget(parent)
  , m_spectrumBands(NULL)
  , m_volumeBarHeight(0)
  , m_barsNumber(0)
  , m_maxFrequency(0)
{
}

void AudioTriggerWidget::setBarsNumber(int num)
{
    m_barsNumber = num;
    if (m_spectrumBands)
        delete[] m_spectrumBands;
    m_spectrumBands = new double[m_barsNumber];
    for (int i = 0; i < m_barsNumber; i++)
        m_spectrumBands[i] = 0;
    m_volumeBarHeight = 0;
    m_barWidth = (width() - 10) / (m_barsNumber + 1);
    update();
}

int AudioTriggerWidget::barsNumber()
{
    return m_barsNumber;
}

void AudioTriggerWidget::setMaxFrequency(int freq)
{
    m_maxFrequency = freq;
}

uchar AudioTriggerWidget::getUcharVolume()
{
    return SCALE(float(m_volumeBarHeight), 0.0, float(m_spectrumHeight), 0.0, 255.0);
}

uchar AudioTriggerWidget::getUcharBand(int idx)
{
    if (idx >= 0 && idx < m_barsNumber)
        return SCALE(float(m_spectrumBands[idx]), 0.0, float(m_spectrumHeight), 0.0, 255.0);

    return 0;
}

void AudioTriggerWidget::displaySpectrum(double *spectrumData, double maxMagnitude, quint32 power)
{
    m_volumeBarHeight = (power * m_spectrumHeight) / 0x7FFF;
    for (int i = 0; i < m_barsNumber; i++)
        m_spectrumBands[i] =  (m_volumeBarHeight * spectrumData[i]) / maxMagnitude;

    //qDebug() << "[displaySpectrum] power: " << power << ", first bar: " << m_spectrumBands[0];
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

    m_spectrumHeight = height() - 20;

    QPainter painter(this);

    // fill spectrum background
    painter.setPen(QPen(Qt::darkGray, 2));
    if (this->isEnabled())
        painter.setBrush(QBrush(Qt::black));
    else
        painter.setBrush(QBrush(Qt::gray));
    painter.drawRect(0, 0, m_barWidth * m_barsNumber, m_spectrumHeight);

    // fill volume bar background
    painter.setBrush(QBrush(Qt::lightGray));
    painter.drawRect(width() - m_barWidth, 0, m_barWidth, m_spectrumHeight);

    // fill frequencies background
    painter.setBrush(QBrush(Qt::darkGray));
    painter.drawRect(0, m_spectrumHeight + 1, width(), 20);

    float xpos = 1;
    painter.setBrush(QBrush(Qt::yellow));

    for (int i = 0; i < m_barsNumber; i++)
    {
        painter.setPen(QPen(Qt::NoPen));
        painter.drawRect(xpos, m_spectrumHeight - m_spectrumBands[i], m_barWidth - 1, m_spectrumBands[i]);
        painter.setPen(QPen(Qt::lightGray, 1));
        painter.drawLine(xpos + m_barWidth, 0, xpos + m_barWidth, m_spectrumHeight - 2);

        xpos += m_barWidth;
    }

    // draw frequencies scale
    float freqIncr = m_maxFrequency / 10;
    if (this->isEnabled())
        painter.setPen(QPen(Qt::black, 1));
    else
        painter.setPen(QPen(Qt::gray, 1));

    for (int i = 1; i < 11; i++)
    {
        float xpos = ((m_barWidth * m_barsNumber) / 10 * i);
        if (width() >= 500)
            painter.drawText(xpos - 50, height() - 5, QString("%1Hz").arg(freqIncr * i));
        painter.drawLine(xpos - 2, m_spectrumHeight + 1, xpos - 2, height());
    }
    //painter.drawText(width() - 15, height() - 5, "V");

    painter.setPen(QPen(Qt::NoPen));
    painter.setBrush(QBrush(Qt::green));
    painter.drawRect(width() - m_barWidth + 1, m_spectrumHeight - m_volumeBarHeight, m_barWidth - 2, m_volumeBarHeight);
}
