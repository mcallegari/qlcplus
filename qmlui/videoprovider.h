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

#include <QQuickView>
#include <QQuickItem>
#include <QMediaPlayer>

#include "video.h"

class Doc;
class VideoContent;

class VideoProvider: public QObject
{
    Q_OBJECT
public:
    VideoProvider(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~VideoProvider();

    /** Get/Set the shared fullscreen context */
    QQuickView *fullscreenContext();
    void setFullscreenContext(QQuickView *context);

protected slots:
    void slotFunctionAdded(quint32 id);
    void slotFunctionRemoved(quint32 id);

    void slotRequestPlayback();
    void slotRequestPause(bool enable);
    void slotRequestStop();

private:
    /** Reference of the QML view */
    QQuickView *m_view;
    /** Reference of the project workspace */
    Doc *m_doc;
    /** Map of the currently available Video functions */
    QMap<quint32, VideoContent *> m_videoMap;
    /** A single instance for fullscreen rendering shared between videos */
    QQuickView *m_fullscreenContext;
};

class VideoContent: public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 id READ id CONSTANT)

public:
    VideoContent(Video *video, VideoProvider *parent = nullptr);

    quint32 id() const;
    Q_INVOKABLE void destroyContext();

    void playContent();
    void stopContent();

protected:
    QVariant getAttribute(quint32 id, const char *propName);
    void updateAttribute(quint32 id, const char *propName, QVariant value);

public slots:
    void slotDetectResolution();
    void slotAttributeChanged(int attrIndex, qreal value);

protected slots:
    void slotMetaDataChanged(const QString &key, const QVariant &value);
    void slotWindowClosing();

protected:
    /** Reference to the parent video provider */
    VideoProvider *m_provider;
    /** reference to the actual Video Function */
    Video *m_video;
    /** temporary media player to retrieve the video resolution */
    QMediaPlayer *m_mediaPlayer;
    /** the video position considering its resolution and the target screen */
    QRect m_geometry;
    /** Quick context for windowed video playback */
    QQuickView *m_viewContext;
};

#endif // VIDEOPROVIDER_H
