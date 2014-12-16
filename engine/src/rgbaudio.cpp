/*
  Q Light Controller Plus
  rgbaudio.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

#include "rgbaudio.h"
#include "audiocapture.h"
#include "doc.h"

RGBAudio::RGBAudio(const Doc * doc)
    : RGBAlgorithm(doc)
    , m_audioInput(NULL)
{
    m_bandsNumber = -1;
}

RGBAudio::RGBAudio(const RGBAudio& a, QObject *parent)
    : QObject(parent)
    , RGBAlgorithm(a.doc())
{
}

RGBAudio::~RGBAudio()
{
    if (m_audioInput != NULL && m_bandsNumber > 0)
    {
        m_audioInput->unregisterBandsNumber(m_bandsNumber);
        if (m_audioInput->isRunning())
            m_audioInput->stop();
    }
}

RGBAlgorithm* RGBAudio::clone() const
{
    RGBAudio* audio = new RGBAudio(*this);
    return static_cast<RGBAlgorithm*> (audio);
}

void RGBAudio::setAudioCapture(AudioCapture *cap)
{
    qDebug() << Q_FUNC_INFO << "Audio capture set";
    if (cap == NULL)
        return;

    m_audioInput = cap;
    connect(m_audioInput, SIGNAL(dataProcessed(double*,int,double,quint32)),
            this, SLOT(slotAudioBarsChanged(double*,int,double,quint32)));
    m_bandsNumber = -1;
}

void RGBAudio::slotAudioBarsChanged(double *spectrumBands, int size,
                                    double maxMagnitude, quint32 power)
{
    if (size != m_bandsNumber)
        return;

    m_mutex.lock();
    m_spectrumValues.clear();
    for (int i = 0; i < m_bandsNumber; i++)
        m_spectrumValues.append(spectrumBands[i]);
    m_maxMagnitude = maxMagnitude;
    m_volumePower = power;
    m_mutex.unlock();
}

void RGBAudio::calculateColors(int barsHeight)
{
    if (barsHeight > 0)
    {
        m_barColors.clear();
        if (endColor() == QColor() 
            || barsHeight == 1) // to avoid division by 0 below
        {
            for (int i = 0; i < barsHeight; i++)
                m_barColors.append(startColor().rgb());
        }
        else
        {
            int crDelta = (endColor().red() - startColor().red()) / (barsHeight - 1);
            int cgDelta = (endColor().green() - startColor().green()) / (barsHeight - 1);
            int cbDelta = (endColor().blue() - startColor().blue()) / (barsHeight - 1);
            QColor pixelColor = startColor();

            for (int i = 0; i < barsHeight; i++)
            {
                m_barColors.append(pixelColor.rgb());
                pixelColor = QColor(pixelColor.red() + crDelta,
                                    pixelColor.green() + cgDelta,
                                    pixelColor.blue() + cbDelta);
            }
        }
    }
}

/****************************************************************************
 * RGBAlgorithm
 ****************************************************************************/

int RGBAudio::rgbMapStepCount(const QSize& size)
{
    Q_UNUSED(size);
    return 1;
}

RGBMap RGBAudio::rgbMap(const QSize& size, uint rgb, int step)
{
    Q_UNUSED(step)

    m_mutex.lock();
    RGBMap map(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        map[y].fill(0);
    }

    // on the first round, just set the proper number of
    // spectrum bands to receive
    if (m_bandsNumber == -1)
    {
        m_bandsNumber = size.width();
        qDebug() << "[RGBAudio] set" << m_bandsNumber << "bars";
        if (m_audioInput->isInitialized() == false)
            m_audioInput->initialize(44100, 1, 2048);
        m_audioInput->registerBandsNumber(m_bandsNumber);
        m_audioInput->start();
        m_mutex.unlock();
        return map;
    }
    if (m_barColors.count() == 0)
        calculateColors(size.height());

    double volHeight = (m_volumePower * size.height()) / 0x7FFF;
    for (int x = 0; x < m_spectrumValues.count(); x++)
    {
        int barHeight =  (volHeight * m_spectrumValues[x]) / m_maxMagnitude;
        for (int y = size.height() - barHeight; y < size.height(); y++)
        {
            if (m_barColors.count() == 0)
                map[y][x] = rgb;
            else
                map[y][x] = m_barColors.at(y);
        }
    }

    m_mutex.unlock();
    return map;
}

QString RGBAudio::name() const
{
    return QString("Audio Spectrum");
}

QString RGBAudio::author() const
{
    return QString("Massimo Callegari");
}

int RGBAudio::apiVersion() const
{
    return 1;
}

void RGBAudio::setColors(QColor start, QColor end)
{
    RGBAlgorithm::setColors(start, end);

    // invalidate bars colors so the next time a rendering is
    // required, it will be filled with the right values
    m_barColors.clear();
}

RGBAlgorithm::Type RGBAudio::type() const
{
    return RGBAlgorithm::Audio;
}

int RGBAudio::acceptColors() const
{
    return 2; // start and end colors accepted
}

bool RGBAudio::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return false;
    }

    if (root.attribute(KXMLQLCRGBAlgorithmType) != KXMLQLCRGBAudio)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm is not Audio";
        return false;
    }

    return true;
}

bool RGBAudio::saveXML(QDomDocument* doc, QDomElement* mtx_root) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCRGBAlgorithm);
    root.setAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBAudio);
    mtx_root->appendChild(root);

    return true;
}
