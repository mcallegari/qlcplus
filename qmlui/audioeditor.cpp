/*
  Q Light Controller Plus
  audioeditor.cpp

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

#include "audioplugincache.h"
#include "audioeditor.h"
#include "tardis.h"
#include "audio.h"
#include "doc.h"

AudioEditor::AudioEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_audio(nullptr)
{
    m_view->rootContext()->setContextProperty("audioEditor", this);
}

void AudioEditor::setFunctionID(quint32 ID)
{
    m_audio = qobject_cast<Audio *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);
    if (m_audio != nullptr)
        connect(m_audio, SIGNAL(totalDurationChanged()),
                this, SIGNAL(mediaInfoChanged()));
}

QString AudioEditor::sourceFileName() const
{
    if (m_audio == nullptr)
        return "";

    return m_audio->getSourceFileName();
}

void AudioEditor::setSourceFileName(QString sourceFileName)
{
    if (sourceFileName.startsWith("file:"))
        sourceFileName = QUrl(sourceFileName).toLocalFile();

    if (m_audio == nullptr || m_audio->getSourceFileName() == sourceFileName)
        return;

    Tardis::instance()->enqueueAction(Tardis::AudioSetSource, m_audio->id(), m_audio->getSourceFileName(), sourceFileName);
    m_audio->setSourceFileName(sourceFileName);
    emit sourceFileNameChanged(sourceFileName);
    emit mediaInfoChanged();
    emit functionNameChanged(m_audio->name());
    emit loopedChanged();
}

QStringList AudioEditor::audioExtensions() const
{
    if (m_audio == nullptr)
        return QStringList();

    return m_audio->getCapabilities();
}

QVariant AudioEditor::mediaInfo() const
{
    QVariantMap infoMap;

    if (m_audio == nullptr)
        return QVariant();

    AudioDecoder *adec = m_audio->getAudioDecoder();
    if (adec == nullptr)
        return QVariant();

    AudioParameters ap = adec->audioParameters();
    infoMap.insert("duration", Function::speedToString(m_audio->totalDuration()));
    infoMap.insert("sampleRate", QString("%1 Hz").arg(ap.sampleRate()));
    infoMap.insert("channels", ap.channels());
    infoMap.insert("bitrate", QString("%1 kb/s").arg(adec->bitrate()));

    return QVariant::fromValue(infoMap);
}

bool AudioEditor::isLooped()
{
    if (m_audio != nullptr)
        return m_audio->runOrder() == Audio::Loop;

    return false;
}

void AudioEditor::setLooped(bool looped)
{
    if (m_audio != nullptr)
    {
        Tardis::instance()->enqueueAction(Tardis::FunctionSetRunOrder, m_audio->id(), m_audio->runOrder(),
                                          looped ? Audio::Loop : Audio::SingleShot);

        if (looped)
            m_audio->setRunOrder(Audio::Loop);
        else
            m_audio->setRunOrder(Audio::SingleShot);

        emit loopedChanged();
    }
}

qreal AudioEditor::volume()
{
    if (m_audio != nullptr)
        return m_audio->volume() * 100;

    return 100;
}

void AudioEditor::setVolume(qreal volume)
{
    if (m_audio == nullptr)
        return;

    Tardis::instance()->enqueueAction(Tardis::AudioSetVolume, m_audio->id(), m_audio->volume(), volume / 100.0);

    m_audio->setVolume(volume / 100);

    emit volumeChanged();
}

int AudioEditor::cardLineIndex() const
{
    if (m_audio == nullptr || m_audio->audioDevice().isEmpty())
        return 0;

    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();
    int i = 1;
    QString device = m_audio->audioDevice();

    foreach (AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_OUTPUT)
        {
            if (info.privateName == device)
                return i;
            i++;
        }

    }
    return 0;
}

void AudioEditor::setCardLineIndex(int cardLineIndex)
{
    if (m_audio == nullptr)
        return;

    if (cardLineIndex == 0)
    {
        if (m_audio->audioDevice().isEmpty() == false)
            emit cardLineIndexChanged(cardLineIndex);

        m_audio->setAudioDevice("");
        return;
    }

    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();
    int i = 1;

    foreach (AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_OUTPUT)
        {
            if (i == cardLineIndex)
            {
                if (m_audio->audioDevice() != info.privateName)
                    emit cardLineIndexChanged(cardLineIndex);

                m_audio->setAudioDevice(info.privateName);
                return;
            }
            i++;
        }

    }
}
