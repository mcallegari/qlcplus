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

#include "audioeditor.h"
#include "audio.h"
#include "doc.h"

AudioEditor::AudioEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_audio(NULL)
{
    m_view->rootContext()->setContextProperty("audioEditor", this);
}

void AudioEditor::setFunctionID(quint32 ID)
{
    m_audio = qobject_cast<Audio *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);
    if (m_audio != NULL)
        connect(m_audio, SIGNAL(totalDurationChanged()),
                this, SIGNAL(mediaInfoChanged()));
}

QString AudioEditor::sourceFileName() const
{
    if (m_audio == NULL)
        return "";

    return m_audio->getSourceFileName();
}

void AudioEditor::setSourceFileName(QString sourceFileName)
{
    if (m_audio == NULL || m_audio->getSourceFileName() == sourceFileName)
        return;

    m_audio->setSourceFileName(sourceFileName);
    emit sourceFileNameChanged(sourceFileName);
    emit mediaInfoChanged();
    emit functionNameChanged(m_audio->name());
    emit loopedChanged();
}

QStringList AudioEditor::mimeTypes() const
{
    if (m_audio == NULL)
        return QStringList();

    return m_audio->getCapabilities();
}

QVariant AudioEditor::mediaInfo() const
{
    QVariantMap infoMap;

    if (m_audio != NULL)
    {
        AudioDecoder *adec = m_audio->getAudioDecoder();
        if (adec != NULL)
        {
            AudioParameters ap = adec->audioParameters();

            infoMap.insert("duration", Function::speedToString(m_audio->totalDuration()));
            infoMap.insert("sampleRate", QString("%1 Hz").arg(ap.sampleRate()));
            infoMap.insert("channels", ap.channels());
            infoMap.insert("bitrate", QString("%1 kb/s").arg(adec->bitrate()));
        }
    }

    return QVariant::fromValue(infoMap);
}

bool AudioEditor::isLooped()
{
    if (m_audio != NULL)
        return m_audio->runOrder() == Audio::Loop;

    return false;
}

void AudioEditor::setLooped(bool looped)
{
    if (m_audio != NULL)
    {
        if (looped)
            m_audio->setRunOrder(Audio::Loop);
        else
            m_audio->setRunOrder(Audio::SingleShot);
    }
}
