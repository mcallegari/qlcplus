/*
  Q Light Controller Plus
  audioeditor.h

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

#ifndef AUDIOEDITOR_H
#define AUDIOEDITOR_H

#include "functioneditor.h"

class Audio;
class ListModel;

class AudioEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QString sourceFileName READ sourceFileName WRITE setSourceFileName NOTIFY sourceFileNameChanged)
    Q_PROPERTY(QStringList audioExtensions READ audioExtensions CONSTANT)
    Q_PROPERTY(QVariant mediaInfo READ mediaInfo NOTIFY mediaInfoChanged)
    Q_PROPERTY(bool looped READ isLooped WRITE setLooped NOTIFY loopedChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(int cardLineIndex READ cardLineIndex WRITE setCardLineIndex NOTIFY cardLineIndexChanged)

public:
    AudioEditor(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Set the ID of the Audio being edited */
    void setFunctionID(quint32 ID);

    /** Get/Set the source file name for this Audio function */
    QString sourceFileName() const;
    void setSourceFileName(QString sourceFileName);

    /** Get the supported file types that can be decoded */
    QStringList audioExtensions() const;

    /** Get the information of the currently loaded media source */
    QVariant mediaInfo() const;

    /** Get/Set looped attribute for this Audio function */
    bool isLooped();
    void setLooped(bool looped);

    /** Get/Set the Audio function volume */
    qreal volume();
    void setVolume(qreal volume);

    /** Get/Set the audio card line used to play this Audio function */
    int cardLineIndex() const;
    void setCardLineIndex(int cardLineIndex);

signals:
    void sourceFileNameChanged(QString sourceFileName);
    void mediaInfoChanged();
    void loopedChanged();
    void volumeChanged();
    void cardLineIndexChanged(int cardLineIndex);

private:
    /** Reference of the Audio currently being edited */
    Audio *m_audio;
};

#endif // AUDIOEDITOR_H
