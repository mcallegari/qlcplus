/*
  Q Light Controller Plus
  videoprovider.h

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

#ifndef VIDEOPROVIDER_H
#define VIDEOPROVIDER_H

#include <QMediaPlayer>
#include <QObject>
#include <QHash>

#include "video.h"

class Doc;
class QVideoWidget;

class VideoWidget: public QObject
{
    Q_OBJECT

public:
    VideoWidget(Video *video, QObject *parent = NULL);

protected slots:
    void slotSourceUrlChanged(QString url);
    void slotTotalTimeChanged(qint64 duration);
    void slotStatusChanged(QMediaPlayer::MediaStatus status);
    void slotMetaDataChanged(QString key, QVariant data);
    void slotPlaybackVideo();
    void slotStopVideo();
    void slotBrightnessAdjust(int value);

private:
    int getScreenCount();

protected:
    /** reference to the actual Video Function */
    Video *m_video;
    /** output interface to render video data */
    QMediaPlayer *m_videoPlayer;
    /** Qt widget that actually displays the video */
    QVideoWidget *m_videoWidget;
};

class VideoProvider: public QObject
{
    Q_OBJECT
public:
    VideoProvider(Doc *doc, QObject *parent);
    ~VideoProvider();

protected slots:
    void slotFunctionAdded(quint32 id);
    void slotFunctionRemoved(quint32 id);

private:
    Doc *m_doc;
    QHash<quint32, VideoWidget *> m_videoMap;
};

#endif // VIDEOPROVIDER_H
