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

#include "vcbutton.h"
#include "vcslider.h"
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
    if (m_inputCapture != NULL)
        m_inputCapture->setBandsNumber(num);
    m_spectrum->setBarsNumber(num);
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
                this, SLOT(slotDisplaySpectrum(double *, double, quint32)));
        m_inputCapture->initialize(44100, 1, 2048);
        m_inputCapture->start();
    }
    else
    {
        m_inputCapture->stop();
    }
    m_configButton->setEnabled(enable);
}

void AudioTriggerFactory::slotDisplaySpectrum(double *spectrumBands, double maxMagnitude, quint32 power)
{
    m_spectrum->displaySpectrum(spectrumBands, maxMagnitude, power);
    m_volumeBar->m_value = m_spectrum->getUcharVolume();
    if (m_volumeBar->m_type == AudioBar::FunctionBar)
        m_volumeBar->checkFunctionThresholds(m_doc);

    for (int i = 0; i < m_spectrumBars.count(); i++)
    {
        m_spectrumBars[i]->m_value = m_spectrum->getUcharBand(i);
        if (m_spectrumBars[i]->m_type == AudioBar::FunctionBar)
            m_spectrumBars[i]->checkFunctionThresholds(m_doc);
    }
}

void AudioTriggerFactory::slotConfiguration()
{
    m_inputCapture->stop();
    m_doc->masterTimer()->unregisterDMXSource(this);
    AudioTriggersConfiguration atc(this, m_doc, m_inputCapture);
    if (atc.exec() == QDialog::Accepted)
    {

    }
    if (m_volumeBar->m_type == AudioBar::DMXBar)
        m_doc->masterTimer()->registerDMXSource(this);
    else
    {
        foreach(AudioBar *bar, m_spectrumBars)
        {
            if (bar->m_type == AudioBar::DMXBar)
                m_doc->masterTimer()->registerDMXSource(this);
        }
    }
    m_inputCapture->start();
}

/*********************************************************************
 * DMXSource
 *********************************************************************/

void AudioTriggerFactory::writeDMX(MasterTimer *timer, UniverseArray *universes)
{
    Q_UNUSED(timer);
    if (m_volumeBar->m_type == AudioBar::DMXBar)
    {
        for(int i = 0; i < m_volumeBar->m_absDmxChannels.count(); i++)
            universes->write(m_volumeBar->m_absDmxChannels.at(i), m_volumeBar->m_value, QLCChannel::Intensity);
    }
    foreach(AudioBar *sb, m_spectrumBars)
    {
        if (sb->m_type == AudioBar::DMXBar)
        {
            for(int i = 0; i < sb->m_absDmxChannels.count(); i++)
                universes->write(sb->m_absDmxChannels.at(i), sb->m_value, QLCChannel::Intensity);
        }
    }
}

/************************************************************************
 * AudioBar class methods
 ************************************************************************/

AudioBar::AudioBar(int t, uchar v)
{
    m_type = t;
    m_value = v;
    m_dmxChannels.clear();
    m_absDmxChannels.clear();
    m_function = NULL;
    m_widget = NULL;
    m_minThreshold = 51; // 20%
    m_maxThreshold = 204; // 80%
}

void AudioBar::setName(QString nme)
{
    m_name = nme;
}

void AudioBar::setMinThreshold(uchar value)
{
    m_minThreshold = value;
}

void AudioBar::setMaxThreshold(uchar value)
{
    m_maxThreshold = value;
}

void AudioBar::attachDmxChannels(Doc *doc, QList<SceneValue> list)
{
    m_dmxChannels.clear();
    m_dmxChannels = list;
    m_absDmxChannels.clear();
    foreach(SceneValue scv, m_dmxChannels)
    {
        Fixture *fx = doc->fixture(scv.fxi);
        if (fx != NULL)
        {
            quint32 absAddr = fx->universeAddress() + scv.channel;
            m_absDmxChannels.append(absAddr);
        }
    }
}

void AudioBar::attachFunction(Function *func)
{
    if (func != NULL)
        m_function = func;
}

void AudioBar::attachWidget(VCWidget *widget)
{
    if (widget != NULL)
        m_widget = widget;
}

void AudioBar::checkFunctionThresholds(Doc *doc)
{
    if (m_function == NULL)
        return;
    if (m_value >= m_maxThreshold && m_function->isRunning() == false)
        m_function->start(doc->masterTimer());
    else if (m_value < m_minThreshold && m_function->isRunning() == true)
        m_function->stop();
}

void AudioBar::checkWidgetFunctionality()
{
    if (m_widget == NULL)
        return;
    if (m_widget->type() == VCWidget::ButtonWidget)
    {
        VCButton *btn = (VCButton *)m_widget;
        if (m_value >= m_maxThreshold && btn->isOn() == false)
            btn->setOn(true);
        else if (m_value < m_minThreshold && btn->isOn() == true)
            btn->setOn(false);
    }
    else if (m_widget->type() == VCWidget::SliderWidget)
    {
        VCSlider *slider = (VCSlider *)m_widget;
        slider->setSliderValue(m_value);
    }
}

void AudioBar::debugInfo()
{
    qDebug() << "[AudioBar] " << m_name;
    qDebug() << "   type:" << m_type << ", value:" << m_value;

}
