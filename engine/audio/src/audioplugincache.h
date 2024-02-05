/*
  Q Light Controller Plus
  audioplugincache.h

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

#ifndef AUDIOPLUGINCACHE_H
#define AUDIOPLUGINCACHE_H

#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
 #include <QAudioDeviceInfo>
#else
 #include <QAudioDevice>
#endif
#include <QObject>
#include <QDir>

#include "audiorenderer.h"
#include "audiocapture.h"

/** @addtogroup engine Engine
 * @{
 */

class AudioDecoder;

class AudioPluginCache : public QObject
{
    Q_OBJECT

public:
    AudioPluginCache(QObject* parent);
    ~AudioPluginCache();

    /** Load plugins from the given directory. */
    void load(const QDir& dir);

    /** Get a list of the audio formats supported by the
     *  loaded plugins */
    QStringList getSupportedFormats();

    /** Get an audio decoder instance suitable for the given $filename.
     *  If $filename can't be decoded, this method returns NULL */
    AudioDecoder *getDecoderForFile(const QString& filename);

    /** Get the list of cached audio devices detected on creation */
    QList<AudioDeviceInfo> audioDevicesList() const;

    /** Return a Qt output device info match based on $devName */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QAudioDeviceInfo getOutputDeviceInfo(QString devName) const;
#else
    QAudioDevice getOutputDeviceInfo(QString devName) const;
#endif

private:
    /** a map of the vailable plugins ordered by priority */
    QMap<int, QString> m_pluginsMap;

    /** a list of all input/output audio devices arranged in
     *  AudioDeviceInfo structure */
    QList<AudioDeviceInfo> m_audioDevicesList;

    /** a list of output audio device for faster lookup */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QList<QAudioDeviceInfo> m_outputDevicesList;
#else
    QList<QAudioDevice> m_outputDevicesList;
#endif
};

/** @} */

#endif
