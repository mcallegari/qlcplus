/*
  Q Light Controller Plus
  videoeditor.h

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

#ifndef VIDEOEDITOR_H
#define VIDEOEDITOR_H

#include <QMediaPlayer>

#include "functioneditor.h"

class Video;

class VideoEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QString sourceFileName READ sourceFileName WRITE setSourceFileName NOTIFY sourceFileNameChanged)
    Q_PROPERTY(QStringList mimeTypes READ mimeTypes CONSTANT)
    Q_PROPERTY(QVariant mediaInfo READ mediaInfo NOTIFY mediaInfoChanged)
    Q_PROPERTY(QStringList screenList READ screenList CONSTANT)
    Q_PROPERTY(int screenIndex READ screenIndex WRITE setScreenIndex NOTIFY screenIndexChanged)
    Q_PROPERTY(bool looped READ isLooped WRITE setLooped NOTIFY loopedChanged)

public:
    VideoEditor(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~VideoEditor();

    /** @reimp */
    void setFunctionID(quint32 ID);

    /** Get/Set the source file name for this Video function */
    QString sourceFileName() const;
    void setSourceFileName(QString sourceFileName);

    /** Get the supported file types that can be decoded */
    QStringList mimeTypes() const;

    /** Get the information of the currently loaded media source */
    QVariant mediaInfo() const;

    /** Get/Set looped attribute for this Audio function */
    bool isLooped();
    void setLooped(bool looped);

    QStringList screenList() const;

    int screenIndex() const;
    void setScreenIndex(int screenIndex);

protected slots:
    void slotDurationChanged(qint64 duration);
    void slotMetaDataChanged(QString key, QVariant data);

signals:
    void sourceFileNameChanged(QString sourceFileName);
    void mediaInfoChanged();
    void loopedChanged();

    void screenIndexChanged(int screenIndex);

private:
    /** Reference of the Video currently being edited */
    Video *m_video;

    /** A map representing the Video metadata */
    QVariantMap infoMap;

    /** temporary player to retrieve metadata information */
    QMediaPlayer *m_mediaPlayer;
};

#endif // VIDEOEDITOR_H
