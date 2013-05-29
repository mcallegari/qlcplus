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

#include "audiotriggerfactory.h"
#include "ui_audiotriggerfactory.h"

#if defined(__APPLE__)
  #include "audiocapture_portaudio.h"
#elif defined(WIN32)
  #include "audiocapture_wavein.h"
#else
  #include "audiocapture_alsa.h"
#endif

AudioTriggerFactory::AudioTriggerFactory(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AudioTriggerFactory)
{
    ui->setupUi(this);

    m_spectrum = new AudioTriggerWidget(this);
#if defined(__APPLE__)
    m_inputCapture = new AudioCapturePortAudio();
#elif defined(WIN32)
    m_inputCapture = new AudioCaptureWaveIn();
#else
    m_inputCapture = new AudioCaptureAlsa();
#endif
    m_spectrum->setBarsNumber(m_inputCapture->bandsNumber());
    m_spectrum->setMaxFrequency(AudioCapture::maxFrequency());

    ui->m_gridLayout->addWidget(m_spectrum);

    connect(ui->m_enableBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEnableCapture(bool)));
    connect(ui->m_configButton, SIGNAL(clicked()),
            this, SLOT(slotConfiguration()));
}

AudioTriggerFactory::~AudioTriggerFactory()
{
    m_inputCapture->stop();
    delete m_inputCapture;
    delete ui;
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
    ui->m_configButton->setEnabled(enable);
}

void AudioTriggerFactory::slotConfiguration()
{
}
