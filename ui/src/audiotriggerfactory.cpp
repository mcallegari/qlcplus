/*
  Q Light Controller Plus
  audiotriggerfactory.cpp

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

#include "scenevalue.h"
#include "audiotriggerfactory.h"
#include "audiotriggersconfiguration.h"

#if defined(__APPLE__)
  #include "audiocapture_portaudio.h"
#elif defined(WIN32)
  #include "audiocapture_wavein.h"
#else
  #include "audiocapture_alsa.h"
#endif

AudioTriggerFactory::AudioTriggerFactory(Doc *doc, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
{
    setupUi(this);

    m_spectrum = new AudioTriggerWidget(this);
#if defined(__APPLE__)
    m_inputCapture = new AudioCapturePortAudio();
#elif defined(WIN32)
    m_inputCapture = new AudioCaptureWaveIn();
#else
    m_inputCapture = new AudioCaptureAlsa();
#endif

    // create the  AudioBar spectrum data. To be loaded
    // from the project
    m_volumeBar = new AudioBar(AudioBar::None, 0);
    for (int i = 0; i < m_inputCapture->bandsNumber(); i++)
    {
        AudioBar *asb = new AudioBar(AudioBar::None, 0);
        m_spectrumBars.append(asb);
    }

    m_spectrum->setBarsNumber(m_inputCapture->bandsNumber());
    m_spectrum->setMaxFrequency(AudioCapture::maxFrequency());

    m_gridLayout->addWidget(m_spectrum);

    connect(m_enableBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEnableCapture(bool)));
    connect(m_configButton, SIGNAL(clicked()),
            this, SLOT(slotConfiguration()));
}

AudioTriggerFactory::~AudioTriggerFactory()
{
    m_inputCapture->stop();
    delete m_inputCapture;
}

AudioBar *AudioTriggerFactory::getSpectrumBar(int index)
{
    if (index == 1000)
        return m_volumeBar;
    if (index >= 0 && index < m_spectrumBars.size())
        return m_spectrumBars.at(index);

    return NULL;
}

void AudioTriggerFactory::setSpectrumBarsNumber(int num)
{
    if (num > m_spectrumBars.count())
    {
        for (int i = 0 ; i < num - m_spectrumBars.count(); i++)
        {
            AudioBar *asb = new AudioBar(AudioBar::None, 0);
            m_spectrumBars.append(asb);
        }
    }
    else if (num < m_spectrumBars.count())
    {
        for (int i = 0 ; i < m_spectrumBars.count() - num; i++)
            m_spectrumBars.takeLast();
    }
}

void AudioTriggerFactory::setSpectrumBarType(int index, int type)
{
    if (index == 1000)
    {
        m_volumeBar->m_type = type;
        return;
    }
    if (index >= 0 && index < m_spectrumBars.size())
        m_spectrumBars[index]->m_type = type;
}

void AudioTriggerFactory::slotEnableCapture(bool enable)
{
    if (enable == true)
    {
        connect(m_inputCapture, SIGNAL(dataProcessed(double *, double, quint32)),
                m_spectrum, SLOT(displaySpectrum(double *, double, quint32)));
        m_inputCapture->initialize(44100, 1, 2048);
        m_inputCapture->start();
    }
    else
    {
        m_inputCapture->stop();
    }
    m_configButton->setEnabled(enable);
}

void AudioTriggerFactory::slotConfiguration()
{
    m_inputCapture->stop();
    AudioTriggersConfiguration atc(this, m_doc, m_inputCapture);
    if (atc.exec() == QDialog::Accepted)
    {

    }
    m_inputCapture->start();
}

/************************************************************************
 * AudioBar class methods
 ************************************************************************/

AudioBar::AudioBar(int t, uchar v)
{
    m_type = t;
    m_value = v;
    m_function = NULL;
    min_threshold = 51; // 20%
    max_threshold = 204; // 80%
}

void AudioBar::setName(QString nme)
{
    m_name = nme;
}

void AudioBar::attachDmxChannels(QList<SceneValue> list)
{
    m_dmxChannels.clear();
    m_dmxChannels = list;
}

void AudioBar::attachFunction(Function *func)
{
    if (func != NULL)
        m_function = func;
}

void AudioBar::debugInfo()
{
    qDebug() << "[AudioBar] " << m_name;
    qDebug() << "   type:" << m_type << ", value:" << m_value;

}
