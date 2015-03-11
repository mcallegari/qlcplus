/*
  Q Light Controller Plus
  audiocapture_qt.cpp

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

#include <QSettings>
#include <QDebug>

#include "audiocapture_qt.h"

AudioCaptureQt::AudioCaptureQt(QObject * parent)
    : AudioCapture(parent)
    , m_audioInput(NULL)
    , m_input(NULL)
{

}

AudioCaptureQt::~AudioCaptureQt()
{
    if (m_audioInput == NULL)
        return;
    m_audioInput->stop();
    delete m_audioInput;
    m_audioInput = NULL;
}

bool AudioCaptureQt::initialize(unsigned int sampleRate, quint8 channels, quint16 bufferSize)
{
    QSettings settings;
    QString devName = "";
    QAudioDeviceInfo audioDevice = QAudioDeviceInfo::defaultInputDevice();

    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_DEVICE);
    if (var.isValid() == true)
    {
        devName = var.toString();
        foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
        {
            if (deviceInfo.deviceName() == devName)
            {
                audioDevice = deviceInfo;
                break;
            }
        }
    }

    m_format.setSampleRate(sampleRate);
    m_format.setChannelCount(channels);
    m_format.setSampleSize(16);
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec("audio/pcm");

    if (!audioDevice.isFormatSupported(m_format))
    {
        qWarning() << "Requested format not supported - trying to use nearest";
        m_format = audioDevice.nearestFormat(m_format);
        channels = m_format.channelCount();
        sampleRate = m_format.sampleRate();
    }

    m_audioInput = new QAudioInput(audioDevice, m_format, this);

    if (m_audioInput == NULL)
    {
        qWarning() << "Cannot open audio input stream from device" << audioDevice.deviceName();
        return false;
    }

    m_input = m_audioInput->start();

    return AudioCapture::initialize(sampleRate, channels, bufferSize);
}

qint64 AudioCaptureQt::latency()
{
    return 0; // TODO
}

void AudioCaptureQt::setVolume(qreal volume)
{
    m_volume = volume;
    if (m_audioInput != NULL)
        m_audioInput->setVolume(volume);
}

void AudioCaptureQt::suspend()
{
}

void AudioCaptureQt::resume()
{
}

bool AudioCaptureQt::readAudio(int maxSize)
{
    if (m_audioInput->bytesReady() < maxSize * 2)
        return false;

    /*qint64 l = */ m_input->read((char *)m_audioBuffer, maxSize * 2);

    //qDebug() << "[QT readAudio] " << l << "bytes read";

    return true;
}











