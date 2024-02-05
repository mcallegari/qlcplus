/*
  Q Light Controller Plus
  audiorenderer_qt6.cpp

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

#include <QMediaDevices>
#include <QSettings>
#include <QString>
#include <QDebug>

#include "doc.h"
#include "audiorenderer_qt6.h"
#include "audioplugincache.h"

AudioRendererQt6::AudioRendererQt6(QString device, Doc *doc, QObject *parent)
    : AudioRenderer(parent)
    , m_audioSink(NULL)
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

AudioRendererQt6::~AudioRendererQt6()
{
    if (m_audioSink == NULL)
        return;

    m_audioSink->stop();
    delete m_audioSink;
    m_audioSink = NULL;
}

bool AudioRendererQt6::initialize(quint32 freq, int chan, AudioFormat format)
{
    m_format.setChannelCount(chan);
    m_format.setSampleRate(freq);
    //m_format.setCodec("audio/pcm");

    switch (format)
    {
        case PCM_S8:
            m_format.setSampleFormat(QAudioFormat::UInt8);
        break;
        case PCM_S16LE:
        case PCM_S24LE:
            m_format.setSampleFormat(QAudioFormat::Int16);
        break;
        case PCM_S32LE:
            m_format.setSampleFormat(QAudioFormat::Int32);
        break;
        default:
            qWarning("AudioRendererQt6: unsupported format detected");
            return false;
    }

    if (!m_deviceInfo.isFormatSupported(m_format))
    {
        m_format = m_deviceInfo.preferredFormat();
        qWarning() << "Default format not supported - trying to use nearest" << m_format.sampleRate();
    }

    return true;
}

qint64 AudioRendererQt6::latency()
{
    return 0;
}

QList<AudioDeviceInfo> AudioRendererQt6::getDevicesInfo()
{
    QList<AudioDeviceInfo> devList;
    QStringList outDevs, inDevs;

    // create a preliminary list of input devices only
    foreach (const QAudioDevice &deviceInfo, QMediaDevices::audioInputs())
        inDevs.append(deviceInfo.description());

    // loop through output devices and check if they're input devices too
    foreach (const QAudioDevice &deviceInfo, QMediaDevices::audioOutputs())
    {
        outDevs.append(deviceInfo.description());
        AudioDeviceInfo info;
        info.deviceName = deviceInfo.description();
        info.privateName = deviceInfo.description(); //QString::number(i);
        info.capabilities = 0;
        info.capabilities |= AUDIO_CAP_OUTPUT;
        if (inDevs.contains(deviceInfo.description()))
        {
            info.capabilities |= AUDIO_CAP_INPUT;
            inDevs.removeOne(deviceInfo.description());
        }
        devList.append(info);
    }

    // add the devices left in the input list. These don't have output capabilities
    foreach (QString dev, inDevs)
    {
        AudioDeviceInfo info;
        info.deviceName = dev;
        info.privateName = dev; //QString::number(i);
        info.capabilities = 0;
        info.capabilities |= AUDIO_CAP_INPUT;
        devList.append(info);
    }

    return devList;
}

qint64 AudioRendererQt6::writeAudio(unsigned char *data, qint64 maxSize)
{
    qsizetype bFree = m_audioSink->bytesFree();

    if (m_audioSink == NULL || bFree < maxSize)
        return 0;

    //qDebug() << "writeAudio called !! - " << maxSize << m_outputBuffer.length() << bFree;

    m_outputBuffer.append((char *)data, maxSize);

    if (m_outputBuffer.length() >= bFree)
    {
       qint64 written = m_output->write(m_outputBuffer.data(), bFree);

        if (written != bFree)
            qDebug() << "[writeAudio] expexcted to write" << bFree << "but wrote" << written;

        m_outputBuffer.remove(0, written);
    }
    return maxSize;
}

void AudioRendererQt6::drain()
{
    m_audioSink->reset();
}

void AudioRendererQt6::reset()
{
    m_audioSink->reset();
}

void AudioRendererQt6::suspend()
{
    m_audioSink->suspend();
}

void AudioRendererQt6::resume()
{
    m_audioSink->resume();
}

void AudioRendererQt6::run()
{
    if (m_audioSink == NULL)
    {
        qDebug() << "Creating audio sink on" << m_deviceInfo.description();

        m_audioSink = new QAudioSink(m_deviceInfo, m_format);

        if (m_audioSink == NULL)
        {
            qWarning() << "Cannot open audio output stream from device" << m_deviceInfo.description();
            return;
        }

        m_audioSink->setBufferSize(8192 * 8);
        m_output = m_audioSink->start();

        if (m_audioSink->error() != QAudio::NoError)
        {
            qWarning() << "Cannot start audio output stream. Error:" << m_audioSink->error();
            return;
        }
    }
    AudioRenderer::run();
    m_audioSink->stop();
}
