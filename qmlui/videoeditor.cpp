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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaMetaData>
#endif

#include "videoeditor.h"
#include "tardis.h"
#include "video.h"
#include "doc.h"

VideoEditor::VideoEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_video(nullptr)
    , m_mediaPlayer(nullptr)
{
    m_view->rootContext()->setContextProperty("videoEditor", this);
}

VideoEditor::~VideoEditor()
{
    if (m_mediaPlayer)
        delete m_mediaPlayer;
}

void VideoEditor::detectMedia()
{
    if (m_video == nullptr)
        return;

    infoMap.clear();

    if (m_video->isPicture())
    {
        infoMap.insert("Resolution", m_video->resolution());
        infoMap.insert("Duration", Function::speedToString(m_video->duration()));
    }
    else
    {
        QString sourceURL = m_video->sourceUrl();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);

        connect(m_mediaPlayer, SIGNAL(metaDataChanged(QString,QVariant)),
                this, SLOT(slotMetaDataChanged(QString,QVariant)));
#else
        m_mediaPlayer = new QMediaPlayer(this);

        connect(m_mediaPlayer, SIGNAL(metaDataChanged()),
                this, SLOT(slotMetaDataChanged()));
#endif

        connect(m_mediaPlayer, SIGNAL(durationChanged(qint64)),
                this, SLOT(slotDurationChanged(qint64)));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (sourceURL.contains("://"))
            m_mediaPlayer->setMedia(QUrl(sourceURL));
        else
            m_mediaPlayer->setMedia(QUrl::fromLocalFile(sourceURL));
#else
        if (sourceURL.contains("://"))
            m_mediaPlayer->setSource(QUrl(sourceURL));
        else
            m_mediaPlayer->setSource(QUrl::fromLocalFile(sourceURL));
#endif
    }
}

void VideoEditor::setFunctionID(quint32 ID)
{
    m_video = qobject_cast<Video *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);

    detectMedia();
}

QString VideoEditor::sourceFileName() const
{
    if (m_video == nullptr)
        return "";

    return m_video->sourceUrl();
}

void VideoEditor::setSourceFileName(QString sourceFileName)
{
    if (sourceFileName.startsWith("file:"))
        sourceFileName = QUrl(sourceFileName).toLocalFile();

    if (m_video == nullptr || m_video->sourceUrl() == sourceFileName)
        return;

    Tardis::instance()->enqueueAction(Tardis::VideoSetSource, m_video->id(), m_video->sourceUrl(), sourceFileName);
    m_video->setSourceUrl(sourceFileName);

    detectMedia();

    emit sourceFileNameChanged(sourceFileName);
    emit functionNameChanged(m_video->name());
    emit loopedChanged();
}

QStringList VideoEditor::videoExtensions() const
{
    return Video::getVideoCapabilities();
}

QStringList VideoEditor::pictureExtensions() const
{
    return Video::getPictureCapabilities();
}

QVariant VideoEditor::mediaInfo() const
{
    return QVariant::fromValue(infoMap);
}

void VideoEditor::slotDurationChanged(qint64 duration)
{
    infoMap.insert("Duration", Function::speedToString(duration));
    m_video->setTotalDuration(duration);
    emit mediaInfoChanged();
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void VideoEditor::slotMetaDataChanged(QString key, QVariant data)
{
    qDebug() << "Got meta data:" << key;
    infoMap.insert(key, data);
    emit mediaInfoChanged();
}
#else
void VideoEditor::slotMetaDataChanged()
{
    if (m_video == NULL)
        return;

    QMediaMetaData md = m_mediaPlayer->metaData();
    foreach (QMediaMetaData::Key k, md.keys())
    {
        QString mdKeyName = md.metaDataKeyToString(k);
        QVariant mdValue = md.stringValue(k);
        qDebug() << "[Metadata]" << mdKeyName << ":" << mdValue;

        switch (k)
        {
            case QMediaMetaData::Resolution:
                m_video->setResolution(md.value(k).toSize());
                mdValue = md.value(k).toSize();
            break;
            case QMediaMetaData::VideoCodec:
                m_video->setVideoCodec(md.stringValue(k));
                mdKeyName = "VideoCodec";
            break;
            case QMediaMetaData::AudioCodec:
                m_video->setAudioCodec(md.stringValue(k));
                mdKeyName = "AudioCodec";
            break;
            case QMediaMetaData::Duration:
                continue;
            break;
            default:
            break;
        }
        infoMap.insert(mdKeyName, mdValue);
    }
    emit mediaInfoChanged();
}
#endif

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
    if (m_video != nullptr)
        return m_video->screen();

    return 0;
}

void VideoEditor::setScreenIndex(int screenIndex)
{
    if (m_video == nullptr || m_video->screen() == screenIndex)
        return;

    Tardis::instance()->enqueueAction(Tardis::VideoSetScreenIndex, m_video->id(), m_video->screen(), screenIndex);
    m_video->setScreen(screenIndex);
    emit screenIndexChanged(screenIndex);
}

bool VideoEditor::isFullscreen() const
{
    if (m_video != nullptr)
        return m_video->fullscreen();

    return false;
}

void VideoEditor::setFullscreen(bool fullscreen)
{
    if (m_video == nullptr || m_video->fullscreen() == fullscreen)
        return;

    Tardis::instance()->enqueueAction(Tardis::VideoSetFullscreen, m_video->id(), m_video->fullscreen(), fullscreen);
    m_video->setFullscreen(fullscreen);
    emit fullscreenChanged(fullscreen);
}

bool VideoEditor::isLooped()
{
    if (m_video != nullptr)
        return m_video->runOrder() == Video::Loop;

    return false;
}

void VideoEditor::setLooped(bool looped)
{
    if (m_video != nullptr)
    {
        Tardis::instance()->enqueueAction(Tardis::FunctionSetRunOrder, m_video->id(),
                                          m_video->runOrder(), looped ? Video::Loop : Video::SingleShot);
        if (looped)
            m_video->setRunOrder(Video::Loop);
        else
            m_video->setRunOrder(Video::SingleShot);
    }
}

bool VideoEditor::hasCustomGeometry() const
{
    if (m_video != nullptr && m_video->customGeometry().isNull() == false)
        return true;

    return false;
}

QRect VideoEditor::customGeometry() const
{
    if (m_video != nullptr)
        return m_video->customGeometry();

    return QRect();
}

void VideoEditor::setCustomGeometry(QRect customGeometry)
{
    if (m_video == nullptr || m_video->customGeometry() == customGeometry)
        return;

    Tardis::instance()->enqueueAction(Tardis::VideoSetGeometry, m_video->id(), m_video->customGeometry(), customGeometry);
    m_video->setCustomGeometry(customGeometry);
    emit customGeometryChanged(customGeometry);
}

QVector3D VideoEditor::rotation() const
{
    if (m_video != nullptr)
        return m_video->rotation();

    return QVector3D();
}

void VideoEditor::setRotation(QVector3D rotation)
{
    if (m_video == nullptr || m_video->rotation() == rotation)
        return;

    Tardis::instance()->enqueueAction(Tardis::VideoSetRotation, m_video->id(), m_video->rotation(), rotation);
    m_video->setRotation(rotation);
    emit rotationChanged(rotation);
}

int VideoEditor::layer() const
{
    if (m_video != nullptr)
        return m_video->zIndex();

    return 1;
}

void VideoEditor::setLayer(int index)
{
    if (m_video == nullptr || m_video->zIndex() == index)
        return;

    Tardis::instance()->enqueueAction(Tardis::VideoSetLayer, m_video->id(), m_video->zIndex(), index);
    m_video->setZIndex(index);
    emit layerChanged(index);
}
