/*
  Q Light Controller Plus
  audiorenderer_qt5.cpp

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
#include <QString>
#include <QDebug>

#include "doc.h"
#include "audiorenderer_qt5.h"
#include "audioplugincache.h"

AudioRendererQt5::AudioRendererQt5(QString device, Doc *doc, QObject *parent)
    : AudioRenderer(parent)
    , m_audioOutput(NULL)
    , m_output(NULL)
    , m_device(device)
{
    QSettings settings;
    QString devName = "";

    QVariant var;
    if (m_device.isEmpty())
        var = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE);
    else
        var = QVariant(m_device);

    if (var.isValid() == true)
        devName = var.toString();

    m_deviceInfo = doc->audioPluginCache()->getOutputDeviceInfo(devName);
}

AudioRendererQt5::~AudioRendererQt5()
{
    if (m_audioOutput == NULL)
        return;

    m_audioOutput->stop();
    delete m_audioOutput;
    m_audioOutput = NULL;
}

bool AudioRendererQt5::initialize(quint32 freq, int chan, AudioFormat format)
{
    m_format.setChannelCount(chan);
    m_format.setSampleRate(freq);
    m_format.setCodec("audio/pcm");

    switch (format)
    {
        case PCM_S8:
            m_format.setSampleSize(8);
            m_format.setSampleType(QAudioFormat::SignedInt);
        break;
        case PCM_S16LE:
            m_format.setSampleSize(16);
            m_format.setSampleType(QAudioFormat::SignedInt);
            m_format.setByteOrder(QAudioFormat::LittleEndian);
        break;
        case PCM_S24LE:
            m_format.setSampleSize(16);
            m_format.setSampleType(QAudioFormat::SignedInt);
            m_format.setByteOrder(QAudioFormat::LittleEndian);
        break;
        case PCM_S32LE:
            m_format.setSampleSize(16);
            m_format.setSampleType(QAudioFormat::SignedInt);
            m_format.setByteOrder(QAudioFormat::LittleEndian);
        break;
        default:
            qWarning("AudioRendererQt5: unsupported format detected");
            return false;
    }

    if (!m_deviceInfo.isFormatSupported(m_format))
    {
        m_format = m_deviceInfo.nearestFormat(m_format);
        qWarning() << "Default format not supported - trying to use nearest" << m_format.sampleRate();
    }

    return true;
}

qint64 AudioRendererQt5::latency()
{
    return 0;
}

QList<AudioDeviceInfo> AudioRendererQt5::getDevicesInfo()
{
    QList<AudioDeviceInfo> devList;
    QStringList outDevs, inDevs;

    // create a preliminary list of input devices only
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
        inDevs.append(deviceInfo.deviceName());

    // loop through output devices and check if they're input devices too
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        outDevs.append(deviceInfo.deviceName());
        AudioDeviceInfo info;
        info.deviceName = deviceInfo.deviceName();
        info.privateName = deviceInfo.deviceName();
        info.capabilities = 0;
        info.capabilities |= AUDIO_CAP_OUTPUT;
        if (inDevs.contains(deviceInfo.deviceName()))
        {
            info.capabilities |= AUDIO_CAP_INPUT;
            inDevs.removeOne(deviceInfo.deviceName());
        }
        devList.append(info);
    }

    // add the devices left in the input list. These don't have output capabilities
    foreach (QString dev, inDevs)
    {
        AudioDeviceInfo info;
        info.deviceName = dev;
        info.privateName = dev;
        info.capabilities = 0;
        info.capabilities |= AUDIO_CAP_INPUT;
        devList.append(info);
    }

    return devList;
}

qint64 AudioRendererQt5::writeAudio(unsigned char *data, qint64 maxSize)
{
    if (m_audioOutput == NULL || m_audioOutput->bytesFree() < maxSize)
        return 0;

    //qDebug() << "writeAudio called !! - " << maxSize;
    qint64 written = m_output->write((const char *)data, maxSize);

    if (written != maxSize)
        qDebug() << "[writeAudio] expexcted to write" << maxSize << "but wrote" << written;

    return written;
}

void AudioRendererQt5::drain()
{
    m_audioOutput->reset();
}

void AudioRendererQt5::reset()
{
    m_audioOutput->reset();
}

void AudioRendererQt5::suspend()
{
    m_audioOutput->suspend();
}

void AudioRendererQt5::resume()
{
    m_audioOutput->resume();
}

void AudioRendererQt5::run()
{
    if (m_audioOutput == NULL)
    {
        m_audioOutput = new QAudioOutput(m_deviceInfo, m_format);

        if (m_audioOutput == NULL)
        {
            qWarning() << "Cannot open audio output stream from device" << m_deviceInfo.deviceName();
            return;
        }

        m_audioOutput->setBufferSize(8192 * 8);
        m_output = m_audioOutput->start();

        if (m_audioOutput->error() != QAudio::NoError)
        {
            qWarning() << "Cannot start audio output stream. Error:" << m_audioOutput->error();
            return;
        }
    }
    AudioRenderer::run();
    m_audioOutput->stop();
}
