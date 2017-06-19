/*
  Q Light Controller Plus
  videoprovider.cpp

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
#include <QQmlContext>
#include <QScreen>

#include "videoprovider.h"
#include "doc.h"

VideoProvider::VideoProvider(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_fullscreenContext(NULL)
{
    Q_ASSERT(doc != NULL);

    qmlRegisterUncreatableType<Video>("org.qlcplus.classes", 1, 0, "VideoFunction", "Can't create a Video !");

    for (Function *f : m_doc->functionsByType(Function::VideoType))
        slotFunctionAdded(f->id());

    connect(m_doc, &Doc::functionAdded, this, &VideoProvider::slotFunctionAdded);
    connect(m_doc, &Doc::functionRemoved, this, &VideoProvider::slotFunctionRemoved);
}

VideoProvider::~VideoProvider()
{
    m_videoMap.clear();
}

QQuickView *VideoProvider::fullscreenContext()
{
    return m_fullscreenContext;
}

void VideoProvider::setFullscreenContext(QQuickView *context)
{
    m_fullscreenContext = context;
}

void VideoProvider::slotFunctionAdded(quint32 id)
{
    Function *func = m_doc->function(id);
    if (func == NULL || func->type() != Function::VideoType)
        return;

    Video *video = qobject_cast<Video *>(func);
    m_videoMap[id] = new VideoContent(video, this);

    connect(video, &Video::requestPlayback, this, &VideoProvider::slotRequestPlayback);
    connect(video, &Video::requestPause, this, &VideoProvider::slotRequestPause);
    connect(video, &Video::requestStop, this, &VideoProvider::slotRequestStop);
    connect(video, &Video::requestBrightnessAdjust, this, &VideoProvider::slotBrightnessAdjust);
}

void VideoProvider::slotFunctionRemoved(quint32 id)
{
    if (m_videoMap.contains(id))
    {
        VideoContent *vc = m_videoMap.take(id);
        delete vc;
    }
}

void VideoProvider::slotRequestPlayback()
{
    Video *video = qobject_cast<Video *>(sender());
    if (video == NULL)
        return;

    if (m_videoMap.contains(video->id()))
        m_videoMap[video->id()]->playVideo();
}

void VideoProvider::slotRequestPause(bool enable)
{
    Q_UNUSED(enable)
}

void VideoProvider::slotRequestStop()
{
    Video *video = qobject_cast<Video *>(sender());
    if (video == NULL)
        return;

    if (m_videoMap.contains(video->id()))
        m_videoMap[video->id()]->stopVideo();
}

void VideoProvider::slotBrightnessAdjust(int value)
{
    Q_UNUSED(value)
}

/*********************************************************************
 * VideoContent class implementation
 *********************************************************************/

VideoContent::VideoContent(Video *video, VideoProvider *parent)
    : m_provider(parent)
    , m_video(video)
    , m_mediaPlayer(NULL)
    , m_viewContext(NULL)
{
    Q_ASSERT(video != NULL);

    if (video->fullscreen() == false)
        slotDetectResolution();

    connect(m_video, SIGNAL(sourceChanged(QString)),
            this, SLOT(slotDetectResolution()));
}

quint32 VideoContent::id() const
{
    return m_video->id();
}

void VideoContent::destroyContext()
{
    stopVideo();

    if (m_video->fullscreen())
        m_provider->setFullscreenContext(NULL);
}

void VideoContent::playVideo()
{
    if (m_video->fullscreen())
        m_viewContext = m_provider->fullscreenContext();

    if (m_video->isPicture())
        m_geometry.setSize(m_video->resolution());

    qDebug() << "Video screen:" << m_video->screen() << ", geometry:" << m_geometry;

    if (m_viewContext == NULL)
    {
        QList<QScreen *> screens = QGuiApplication::screens();
        QScreen *vScreen = NULL;

        if (m_video->screen() < screens.count())
            vScreen = screens.at(m_video->screen());

        m_viewContext = new QQuickView(QUrl("qrc:/VideoContext.qml"));
        m_viewContext->rootContext()->setContextProperty("videoContent", this);

        if (m_video->customGeometry().isNull())
        {
            m_viewContext->setGeometry(m_geometry);
            if (vScreen)
                m_viewContext->setPosition(vScreen->geometry().topLeft());
        }
        else
        {
            QPoint topLeft = vScreen ? vScreen->geometry().topLeft() : QPoint(0, 0);
            topLeft.setX(topLeft.x() + m_video->customGeometry().x());
            topLeft.setY(topLeft.y() + m_video->customGeometry().y());
            m_viewContext->setGeometry(m_video->customGeometry());
            m_viewContext->setPosition(topLeft);
        }

        connect(m_viewContext, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(slotWindowClosing()));
    }

    if (m_video->isPicture())
    {
        QMetaObject::invokeMethod(m_viewContext->rootObject(), "addPicture",
                                  Q_ARG(QVariant, QVariant::fromValue(m_video)));
    }
    else
    {
        QMetaObject::invokeMethod(m_viewContext->rootObject(), "addVideo",
                                  Q_ARG(QVariant, QVariant::fromValue(m_video)));
    }

    if (m_video->fullscreen())
    {
        m_provider->setFullscreenContext(m_viewContext);
        m_viewContext->showFullScreen();
    }
    else
        m_viewContext->show();
}

void VideoContent::stopVideo()
{
    if (m_viewContext)
    {
        m_viewContext->deleteLater();
        m_viewContext = NULL;
        if (m_video->fullscreen())
            m_provider->setFullscreenContext(NULL);
    }
}

void VideoContent::slotDetectResolution()
{
    if (m_video->isPicture())
        return;

    m_mediaPlayer = new QMediaPlayer();

    connect(m_mediaPlayer, SIGNAL(metaDataChanged(QString,QVariant)),
                this, SLOT(slotMetaDataChanged(QString,QVariant)));

    QString sourceURL = m_video->sourceUrl();
    if (sourceURL.contains("://"))
        m_mediaPlayer->setMedia(QUrl(sourceURL));
    else
        m_mediaPlayer->setMedia(QUrl::fromLocalFile(sourceURL));
}

void VideoContent::slotMetaDataChanged(const QString &key, const QVariant &value)
{
    if (key == "Resolution")
    {
        m_geometry.setSize(value.toSize());

        disconnect(m_mediaPlayer, SIGNAL(metaDataChanged(QString,QVariant)),
                    this, SLOT(slotMetaDataChanged(QString,QVariant)));
        m_mediaPlayer->deleteLater();
        m_mediaPlayer = NULL;
    }
}

void VideoContent::slotWindowClosing()
{
    stopVideo();
}
