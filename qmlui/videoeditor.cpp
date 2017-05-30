/*
  Q Light Controller Plus
  videoeditor.cpp

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

#include <QGuiApplication>
#include <QScreen>

#include "videoeditor.h"
#include "video.h"
#include "doc.h"

VideoEditor::VideoEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_video(NULL)
{
    m_view->rootContext()->setContextProperty("videoEditor", this);
    m_mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);

    connect(m_mediaPlayer, SIGNAL(metaDataChanged(QString,QVariant)),
            this, SLOT(slotMetaDataChanged(QString,QVariant)));
    connect(m_mediaPlayer, SIGNAL(durationChanged(qint64)),
                this, SLOT(slotDurationChanged(qint64)));
}

VideoEditor::~VideoEditor()
{
    delete m_mediaPlayer;
}

void VideoEditor::setFunctionID(quint32 ID)
{
    m_video = qobject_cast<Video *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);
    if (m_video != NULL)
    {
        /*connect(m_video, SIGNAL(totalTimeChanged(qint64)),
                this, SLOT(slotDurationChanged(qint64)));
        connect(m_video, SIGNAL(metaDataChanged(QString,QVariant)),
                this, SLOT(slotMetaDataChanged(QString,QVariant)));*/

        QString sourceURL = m_video->sourceUrl();
        if (sourceURL.contains("://"))
            m_mediaPlayer->setMedia(QUrl(sourceURL));
        else
            m_mediaPlayer->setMedia(QUrl::fromLocalFile(sourceURL));
    }
}

QString VideoEditor::sourceFileName() const
{
    if (m_video == NULL)
        return "";

    return m_video->sourceUrl();
}

void VideoEditor::setSourceFileName(QString sourceFileName)
{
    if (m_video == NULL || m_video->sourceUrl() == sourceFileName)
        return;

    m_video->setSourceUrl(sourceFileName);
    if (sourceFileName.contains("://"))
        m_mediaPlayer->setMedia(QUrl(sourceFileName));
    else
        m_mediaPlayer->setMedia(QUrl::fromLocalFile(sourceFileName));

    emit sourceFileNameChanged(sourceFileName);
    emit mediaInfoChanged();
    emit functionNameChanged(m_video->name());
    emit loopedChanged();
}

QStringList VideoEditor::mimeTypes() const
{
    if (m_video == NULL)
        return QStringList();

    return m_video->getCapabilities();
}

QVariant VideoEditor::mediaInfo() const
{
    return QVariant::fromValue(infoMap);
}

void VideoEditor::slotDurationChanged(qint64 duration)
{
    infoMap.insert("Duration",Function::speedToString(duration));
    emit mediaInfoChanged();
}

void VideoEditor::slotMetaDataChanged(QString key, QVariant data)
{
    qDebug() << "Got meta data:" << key;
    infoMap.insert(key, data);
    emit mediaInfoChanged();
}

QStringList VideoEditor::screenList() const
{
    QStringList list;
    int i = 1;

    for (QScreen *screen : QGuiApplication::screens())
        list.append(QString(QString("Screen %1 - (%2)").arg(i++).arg(screen->name())));

    return list;
}

int VideoEditor::screenIndex() const
{
    if (m_video != NULL)
        return m_video->screen();

    return 0;
}

void VideoEditor::setScreenIndex(int screenIndex)
{
    if (m_video == NULL || m_video->screen() == screenIndex)
        return;

    m_video->setScreen(screenIndex);
    emit screenIndexChanged(screenIndex);
}

bool VideoEditor::isFullscreen() const
{
    if (m_video != NULL)
        return m_video->fullscreen();

    return false;
}

void VideoEditor::setFullscreen(bool fullscreen)
{
    if (m_video == NULL || m_video->fullscreen() == fullscreen)
        return;

    m_video->setFullscreen(fullscreen);
    emit fullscreenChanged(fullscreen);
}

bool VideoEditor::isLooped()
{
    if (m_video != NULL)
        return m_video->runOrder() == Video::Loop;

    return false;
}

void VideoEditor::setLooped(bool looped)
{
    if (m_video != NULL)
    {
        if (looped)
            m_video->setRunOrder(Video::Loop);
        else
            m_video->setRunOrder(Video::SingleShot);
    }
}

bool VideoEditor::hasCustomGeometry() const
{
    if (m_video != NULL && m_video->customGeometry().isNull() == false)
        return true;

    return false;
}

QRect VideoEditor::customGeometry() const
{
    if (m_video != NULL)
        return m_video->customGeometry();

    return QRect();
}

void VideoEditor::setCustomGeometry(QRect customGeometry)
{
    if (m_video == NULL || m_video->customGeometry() == customGeometry)
        return;

    m_video->setCustomGeometry(customGeometry);
    emit customGeometryChanged(customGeometry);
}
