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
#include <QCoreApplication>

#include "audiocapture_qt.h"

AudioCaptureQt::AudioCaptureQt(QObject * parent)
    : AudioCapture(parent)
    , m_audioInput(NULL)
    , m_input(NULL)
{
}

AudioCaptureQt::~AudioCaptureQt()
{
    stop();
    Q_ASSERT(m_audioInput == NULL);
}

bool AudioCaptureQt::initialize()
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

    m_format.setSampleRate(m_sampleRate);
    m_format.setChannelCount(m_channels);
    m_format.setSampleSize(16);
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec("audio/pcm");

    if (!audioDevice.isFormatSupported(m_format))
    {
        qWarning() << "Requested format not supported - trying to use nearest";
        m_format = audioDevice.nearestFormat(m_format);
        m_channels = m_format.channelCount();
        m_sampleRate = m_format.sampleRate();
    }

    Q_ASSERT(m_audioInput == NULL);

    m_audioInput = new QAudioInput(audioDevice, m_format);

    if (m_audioInput == NULL)
    {
        qWarning() << "Cannot open audio input stream from device" << audioDevice.deviceName();
        return false;
    }

    m_input = m_audioInput->start();

    if (m_audioInput->state() == QAudio::StoppedState)
    {
        qWarning() << "Could not start input capture on device" << audioDevice.deviceName();
        delete m_audioInput;
        m_audioInput = NULL;
        m_input = NULL;
        return false;
    }

    m_currentBufferPosition = 0;

    return true;
}

void AudioCaptureQt::uninitialize()
{
    Q_ASSERT(m_audioInput != NULL);

    m_audioInput->stop();
    delete m_audioInput;
    m_audioInput = NULL;
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
    if (m_audioInput == NULL || m_input == NULL)
        return false;

    unsigned int bufferSize = maxSize * sizeof(*m_audioBuffer);

    int readSize = m_input->read(((char*)m_audioBuffer) + m_currentBufferPosition, bufferSize - m_currentBufferPosition);
    if (readSize < 0)
    {
        // read error
        return false;
    }

    m_currentBufferPosition += readSize;
    //qDebug() << "[QT readAudio] " << readSize << "bytes read -> (" << m_currentBufferPosition << "/" << bufferSize << ")";
    if (m_currentBufferPosition == bufferSize)
    {
        m_currentBufferPosition = 0;
        return true;
    }
    return false;
}
