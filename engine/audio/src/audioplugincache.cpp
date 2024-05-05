/*
  Q Light Controller Plus
  audioplugincache.cpp

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

#include <QPluginLoader>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaDevices>
#endif
#include <QDebug>

#include "audioplugincache.h"
#include "audiodecoder.h"
#include "qlcfile.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
 #if defined(__APPLE__) || defined(Q_OS_MAC)
  #include "audiorenderer_portaudio.h"
 #elif defined(WIN32) || defined(Q_OS_WIN)
  #include "audiorenderer_waveout.h"
 #else
  #include "audiorenderer_alsa.h"
 #endif
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
 #include "audiorenderer_qt5.h"
#else
 #include "audiorenderer_qt6.h"
#endif

AudioPluginCache::AudioPluginCache(QObject *parent)
    : QObject(parent)
{
}

AudioPluginCache::~AudioPluginCache()
{
}

void AudioPluginCache::load(const QDir &dir)
{
    qDebug() << Q_FUNC_INFO << dir.path();

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#if defined(__APPLE__) || defined(Q_OS_MAC)
    m_audioDevicesList = AudioRendererPortAudio::getDevicesInfo();
#elif defined(WIN32) || defined(Q_OS_WIN)
    m_audioDevicesList = AudioRendererWaveOut::getDevicesInfo();
#else
    m_audioDevicesList = AudioRendererAlsa::getDevicesInfo();
#endif
#else
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_audioDevicesList = AudioRendererQt5::getDevicesInfo();
    m_outputDevicesList = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
#else
    m_audioDevicesList = AudioRendererQt6::getDevicesInfo();
    m_outputDevicesList = QMediaDevices::audioOutputs();
#endif
#endif

    /* Check that we can access the directory */
    if (dir.exists() == false || dir.isReadable() == false)
        return;

    /* Loop through all files in the directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        /* Attempt to load a plugin from the path */
        QString fileName(it.next());
        QString path = dir.absoluteFilePath(fileName);

        QPluginLoader loader(path, this);
        AudioDecoder* ptr = qobject_cast<AudioDecoder*> (loader.instance());
        if (ptr != NULL)
        {
            qDebug() << "Loaded audio decoder plugin from" << fileName;
            /* Just append the plugin path to be used at runtime
             * for dynamic creation of instances */
            ptr->initialize("");
            m_pluginsMap[ptr->priority()] = path;
            loader.unload();
        }
        else
            qDebug() << "Failed to load plugin: " << loader.errorString();
    }
}

QStringList AudioPluginCache::getSupportedFormats()
{
    QStringList caps;
    foreach (QString path, m_pluginsMap.values())
    {
        QPluginLoader loader(path, this);
        AudioDecoder* ptr = qobject_cast<AudioDecoder*> (loader.instance());
        if (ptr != NULL)
        {
            ptr->initialize("");
            caps << ptr->supportedFormats();
            loader.unload();
        }
    }

    return caps;
}

AudioDecoder *AudioPluginCache::getDecoderForFile(const QString &filename)
{
    QFile fn(filename);
    if (fn.exists() == false)
        return NULL;

    foreach (QString path, m_pluginsMap.values())
    {
        QPluginLoader loader(path, this);
        AudioDecoder* ptr = qobject_cast<AudioDecoder*> (loader.instance());
        if (ptr != NULL)
        {
            ptr->initialize("");
            AudioDecoder* copy = qobject_cast<AudioDecoder*> (ptr->createCopy());
            if (copy->initialize(filename) == false)
            {
                loader.unload();
                //delete copy;
                continue;
            }
            return copy;
        }
    }

    return NULL;
}

QList<AudioDeviceInfo> AudioPluginCache::audioDevicesList() const
{
    return m_audioDevicesList;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QAudioDeviceInfo AudioPluginCache::getOutputDeviceInfo(QString devName) const
{
    foreach (const QAudioDeviceInfo &deviceInfo, m_outputDevicesList)
    {
        if (deviceInfo.deviceName() == devName)
            return deviceInfo;
    }

    return QAudioDeviceInfo::defaultOutputDevice();
}
#else
QAudioDevice AudioPluginCache::getOutputDeviceInfo(QString devName) const
{
    foreach (const QAudioDevice &deviceInfo, m_outputDevicesList)
    {
        if (deviceInfo.description() == devName)
            return deviceInfo;
    }

    return QMediaDevices::defaultAudioOutput();
}
#endif
