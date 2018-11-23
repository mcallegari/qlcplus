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
    Q_PROPERTY(QStringList videoExtensions READ videoExtensions CONSTANT)
    Q_PROPERTY(QStringList pictureExtensions READ pictureExtensions CONSTANT)
    Q_PROPERTY(QVariant mediaInfo READ mediaInfo NOTIFY mediaInfoChanged)
    Q_PROPERTY(QStringList screenList READ screenList CONSTANT)
    Q_PROPERTY(int screenIndex READ screenIndex WRITE setScreenIndex NOTIFY screenIndexChanged)
    Q_PROPERTY(bool fullscreen READ isFullscreen WRITE setFullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(bool looped READ isLooped WRITE setLooped NOTIFY loopedChanged)
    Q_PROPERTY(bool hasCustomGeometry READ hasCustomGeometry CONSTANT)
    Q_PROPERTY(QRect customGeometry READ customGeometry WRITE setCustomGeometry NOTIFY customGeometryChanged)
    Q_PROPERTY(QVector3D rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(int layer READ layer WRITE setLayer NOTIFY layerChanged)

public:
    VideoEditor(QQuickView *view, Doc *doc, QObject *parent = nullptr);
    ~VideoEditor();

    /** @reimp */
    void setFunctionID(quint32 ID);

    /** Get/Set the source file name for this Video function */
    QString sourceFileName() const;
    void setSourceFileName(QString sourceFileName);

    /** Get the supported video file types that can be decoded */
    QStringList videoExtensions() const;

    /** Get the supported picture file types that can be rendered */
    QStringList pictureExtensions() const;

    /** Get the information of the currently loaded media source */
    QVariant mediaInfo() const;

    QStringList screenList() const;

    /** Get/Set the screen index of this Video function */
    int screenIndex() const;
    void setScreenIndex(int screenIndex);

    /** Get/Set the fullscreen flag of this Video function */
    bool isFullscreen() const;
    void setFullscreen(bool fullscreen);

    /** Get/Set looped attribute for this Video function */
    bool isLooped();
    void setLooped(bool looped);

    bool hasCustomGeometry() const;

    /** Get/Set the custom geometry for this Video function */
    QRect customGeometry() const;
    void setCustomGeometry(QRect customGeometry);

    /** Get/Set a rotation transformation */
    QVector3D rotation() const;
    void setRotation(QVector3D rotation);

    /** Get/Set the layer of this Video function */
    int layer() const;
    void setLayer(int index);

protected slots:
    void slotDurationChanged(qint64 duration);
    void slotMetaDataChanged(QString key, QVariant data);

signals:
    void sourceFileNameChanged(QString sourceFileName);
    void mediaInfoChanged();
    void screenIndexChanged(int screenIndex);
    void fullscreenChanged(bool fullscreen);
    void loopedChanged();
    void customGeometryChanged(QRect customGeometry);
    void rotationChanged(QVector3D rotation);
    void layerChanged(int index);

private:
    /** Reference of the Video currently being edited */
    Video *m_video;

    /** A map representing the Video metadata */
    QVariantMap infoMap;

    /** temporary player to retrieve metadata information */
    QMediaPlayer *m_mediaPlayer;
};

#endif // VIDEOEDITOR_H
