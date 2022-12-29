/*
  Q Light Controller Plus
  audiocapture_qt5.cpp

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

#include "audiocapture_qt5.h"

AudioCaptureQt6::AudioCaptureQt6(QObject * parent)
    : AudioCapture(parent)
    , m_audioInput(NULL)
    , m_input(NULL)
{
}

AudioCaptureQt6::~AudioCaptureQt6()
{
    stop();
    Q_ASSERT(m_audioInput == NULL);
}

bool AudioCaptureQt6::initialize()
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

    m_currentReadBuffer.clear();

    return true;
}

void AudioCaptureQt6::uninitialize()
{
    Q_ASSERT(m_audioInput != NULL);

    m_audioInput->stop();
    delete m_audioInput;
    m_audioInput = NULL;
}

qint64 AudioCaptureQt6::latency()
{
    return 0; // TODO
}

void AudioCaptureQt6::setVolume(qreal volume)
{
    m_volume = volume;
    if (m_audioInput != NULL)
        m_audioInput->setVolume(volume);
}

void AudioCaptureQt6::suspend()
{
}

void AudioCaptureQt6::resume()
{
}

bool AudioCaptureQt6::readAudio(int maxSize)
{
    if (m_audioInput == NULL || m_input == NULL)
        return false;

    int bufferSize = maxSize * sizeof(*m_audioBuffer);

    QByteArray readBuffer = m_input->readAll();
    m_currentReadBuffer += readBuffer;

    // qDebug() << "[QT readAudio] " << readBuffer.size() << "bytes read -> (" << m_currentReadBuffer.size() << "/" << bufferSize << ")";

    if (m_currentReadBuffer.size() < bufferSize)
    {
        // nothing has been read
        return false;
    }

    memcpy(m_audioBuffer, m_currentReadBuffer.data(), bufferSize);
    m_currentReadBuffer.remove(0, bufferSize);
    return true;
}
