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
  , m_spectrumHeight(0)
  , m_volumeBarHeight(0)
  , m_barsNumber(0)
  , m_barWidth(0)
  , m_maxFrequency(0)
  , m_spectrumPixelWidth(0)
  , m_volumeBarPixelWidth(0)
{
}

AudioTriggerWidget::~AudioTriggerWidget()
{
    delete[] m_spectrumBands;
}

void AudioTriggerWidget::setBarsNumber(int num)
{
    m_barsNumber = num;
    delete[] m_spectrumBands;
    m_spectrumBands = new double[m_barsNumber];
    for (int i = 0; i < m_barsNumber; i++)
        m_spectrumBands[i] = 0;
    m_volumeBarHeight = 0;
    recomputeBarLayout();
    update();
}

int AudioTriggerWidget::barsNumber() const
{
    return m_barsNumber;
}

void AudioTriggerWidget::setMaxFrequency(int freq)
{
    m_maxFrequency = freq;
}

void AudioTriggerWidget::setBandFrequencyEdges(const QVector<double> &edges)
{
    m_bandEdges = edges;
    recomputeBarLayout();
    update();
}

void AudioTriggerWidget::recomputeBarLayout()
{
    const int totalW = qMax(0, width() - 10);
    m_volumeBarPixelWidth = (m_barsNumber > 0) ? float(totalW) / float(m_barsNumber + 1) : 0;
    m_spectrumPixelWidth = float(totalW) - m_volumeBarPixelWidth;

    m_barXStart.resize(m_barsNumber);
    m_barPixelWidth.resize(m_barsNumber);

    if (m_barsNumber <= 0 || m_spectrumPixelWidth <= 0)
        return;

    if (m_bandEdges.size() != m_barsNumber + 1)
    {
        m_barWidth = m_spectrumPixelWidth / float(m_barsNumber);
        for (int i = 0; i < m_barsNumber; ++i)
        {
            m_barXStart[i] = 1.0f + float(i) * m_barWidth;
            m_barPixelWidth[i] = m_barWidth - 1.0f;
        }
        return;
    }

    const double f0 = qMax(1.0, m_bandEdges.first());
    const double fN = qMax(f0 + 1.0, m_bandEdges.last());
    const double log0 = qLn(f0);
    const double logN = qLn(fN);
    const double logSpan = logN - log0;

    if (logSpan <= 0.0)
    {
        m_barWidth = m_spectrumPixelWidth / float(m_barsNumber);
        for (int i = 0; i < m_barsNumber; ++i)
        {
            m_barXStart[i] = 1.0f + float(i) * m_barWidth;
            m_barPixelWidth[i] = m_barWidth - 1.0f;
        }
        return;
    }

    for (int i = 0; i < m_barsNumber; ++i)
    {
        const double logLo = qLn(qMax(1.0, m_bandEdges[i]));
        const double logHi = qLn(qMax(1.0, m_bandEdges[i + 1]));
        const float xLo = 1.0f + float((logLo - log0) / logSpan) * m_spectrumPixelWidth;
        const float xHi = 1.0f + float((logHi - log0) / logSpan) * m_spectrumPixelWidth;
        m_barXStart[i] = xLo;
        m_barPixelWidth[i] = qMax(1.0f, xHi - xLo - 1.0f);
    }
}

uchar AudioTriggerWidget::getUcharVolume() const
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
        m_spectrumBands[i] = (m_volumeBarHeight * spectrumData[i]) / maxMagnitude;

    update();
}

void AudioTriggerWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    recomputeBarLayout();
}

void AudioTriggerWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    if (m_barsNumber == 0)
        return;

    m_spectrumHeight = height() - 20;

    QPainter painter(this);

    painter.setPen(QPen(Qt::darkGray, 2));
    if (this->isEnabled())
        painter.setBrush(QBrush(Qt::black));
    else
        painter.setBrush(QBrush(Qt::gray));
    painter.drawRect(0, 0, int(m_spectrumPixelWidth), m_spectrumHeight);

    painter.setBrush(QBrush(Qt::lightGray));
    painter.drawRect(int(m_spectrumPixelWidth), 0, int(m_volumeBarPixelWidth), m_spectrumHeight);

    painter.setBrush(QBrush(Qt::darkGray));
    painter.drawRect(0, m_spectrumHeight + 1, width(), 20);

    painter.setBrush(QBrush(Qt::yellow));

    for (int i = 0; i < m_barsNumber; i++)
    {
        const float xpos = (i < m_barXStart.size()) ? m_barXStart[i] : 1.0f;
        const float barW = (i < m_barPixelWidth.size()) ? m_barPixelWidth[i] : m_barWidth;

        painter.setPen(QPen(Qt::NoPen));
        painter.drawRect(int(xpos), m_spectrumHeight - int(m_spectrumBands[i]),
                         int(barW), int(m_spectrumBands[i]));
        painter.setPen(QPen(Qt::lightGray, 1));
        painter.drawLine(int(xpos + barW + 1), 0, int(xpos + barW + 1), m_spectrumHeight - 2);
    }

    if (m_bandEdges.size() == m_barsNumber + 1 && m_barsNumber > 0)
    {
        const double f0 = qMax(1.0, m_bandEdges.first());
        const double fN = qMax(f0 + 1.0, m_bandEdges.last());
        const double log0 = qLn(f0);
        const double logN = qLn(fN);
        const double logSpan = logN - log0;

        if (this->isEnabled())
            painter.setPen(QPen(Qt::black, 1));
        else
            painter.setPen(QPen(Qt::gray, 1));

        if (logSpan > 0.0)
        {
            const int tickCount = 10;
            for (int t = 1; t <= tickCount; ++t)
            {
                const double logF = log0 + logSpan * (double(t) / double(tickCount));
                const double freq = qExp(logF);
                const float xpos = 1.0f + float((logF - log0) / logSpan) * m_spectrumPixelWidth;
                if (width() >= 500)
                    painter.drawText(int(xpos) - 25, height() - 5, QString("%1Hz").arg(int(freq)));
                painter.drawLine(int(xpos), m_spectrumHeight + 1, int(xpos), height());
            }
        }
    }
    else
    {
        float freqIncr = m_maxFrequency / 10.0f;
        if (this->isEnabled())
            painter.setPen(QPen(Qt::black, 1));
        else
            painter.setPen(QPen(Qt::gray, 1));

        for (int i = 1; i < 11; i++)
        {
            float xpos = (m_spectrumPixelWidth / 10.0f * float(i));
            if (width() >= 500)
                painter.drawText(int(xpos) - 50, height() - 5, QString("%1Hz").arg(int(freqIncr * i)));
            painter.drawLine(int(xpos) - 2, m_spectrumHeight + 1, int(xpos), height());
        }
    }

    painter.setPen(QPen(Qt::NoPen));
    painter.setBrush(QBrush(Qt::green));
    painter.drawRect(int(m_spectrumPixelWidth) + 1, m_spectrumHeight - int(m_volumeBarHeight),
                     int(m_volumeBarPixelWidth) - 2, int(m_volumeBarHeight));
}
