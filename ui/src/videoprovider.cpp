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

#include "videoprovider.h"
#include "doc.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMediaPlayer>
#include <QVideoWidget>

VideoProvider::VideoProvider(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    connect(m_doc, SIGNAL(functionAdded(quint32)),
            this, SLOT(slotFunctionAdded(quint32)));
    connect(m_doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

VideoProvider::~VideoProvider()
{
    m_videoMap.clear();
}

void VideoProvider::slotFunctionAdded(quint32 id)
{
    Function *func = m_doc->function(id);
    if (func == NULL)
        return;

    if(func->type() == Function::Video)
    {
        VideoWidget *vWidget = new VideoWidget(qobject_cast<Video *>(func));
        m_videoMap[id] = vWidget;
    }
}

void VideoProvider::slotFunctionRemoved(quint32 id)
{
    if (m_videoMap.contains(id))
    {
        VideoWidget *vw = m_videoMap.take(id);
        delete vw;
    }
}

/*********************************************************************
 * VideoWidget class implementation
 *********************************************************************/

VideoWidget::VideoWidget(Video *video, QObject *parent)
    : QObject(parent)
    , m_video(video)
    , m_videoPlayer(NULL)
    , m_videoWidget(NULL)
{
    Q_ASSERT(video != NULL);

    m_videoPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    m_videoPlayer->moveToThread(QCoreApplication::instance()->thread());

    connect(m_videoPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slotStatusChanged(QMediaPlayer::MediaStatus)));
    connect(m_videoPlayer, SIGNAL(metaDataChanged(QString,QVariant)),
            this, SLOT(slotMetaDataChanged(QString,QVariant)));
    connect(m_videoPlayer, SIGNAL(durationChanged(qint64)),
            this, SLOT(slotTotalTimeChanged(qint64)));

    connect(m_video, SIGNAL(sourceChanged(QString)),
            this, SLOT(slotSourceUrlChanged(QString)));
    connect(m_video, SIGNAL(requestPlayback()),
            this, SLOT(slotPlaybackVideo()));
    connect(m_video, SIGNAL(requestStop()),
            this, SLOT(slotStopVideo()));
    connect(m_video, SIGNAL(requestBrightnessAdjust(int)),
            this, SLOT(slotBrightnessAdjust(int)));

    QString sourceURL = m_video->sourceUrl();
    if (sourceURL.contains("://"))
        m_videoPlayer->setMedia(QUrl(sourceURL));
    else
        m_videoPlayer->setMedia(QUrl::fromLocalFile(sourceURL));

    qDebug() << "Video source URL:" << sourceURL;
}

void VideoWidget::slotSourceUrlChanged(QString url)
{
    qDebug() << "Video source URL changed:" << url;

    if (url.contains("://"))
        m_videoPlayer->setMedia(QUrl(url));
    else
        m_videoPlayer->setMedia(QUrl::fromLocalFile(url));
}

void VideoWidget::slotTotalTimeChanged(qint64 duration)
{
    qDebug() << "Video duration: " << duration;
    m_video->setTotalDuration(duration);
}

void VideoWidget::slotStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << Q_FUNC_INFO << status;
    switch (status)
    {
        case QMediaPlayer::UnknownMediaStatus:
        case QMediaPlayer::NoMedia:
        case QMediaPlayer::LoadedMedia:
        case QMediaPlayer::BufferingMedia:
        case QMediaPlayer::BufferedMedia:
            //setStatusInfo(QString());
            break;
        case QMediaPlayer::LoadingMedia:
            //setStatusInfo(tr("Loading..."));
            break;
        case QMediaPlayer::StalledMedia:
            //setStatusInfo(tr("Media Stalled"));
            break;
        case QMediaPlayer::EndOfMedia:
        {
            if (m_videoPlayer != NULL)
                m_videoPlayer->stop();

            if (m_videoWidget != NULL)
            {
                m_videoWidget->hide();
                //m_videoWidget->deleteLater();
                //m_videoWidget = NULL;
            }
            m_video->stop();
            break;
        }
        case QMediaPlayer::InvalidMedia:
            //displayErrorMessage();
            break;
    }
}

void VideoWidget::slotMetaDataChanged(QString key, QVariant data)
{
    if (m_video == NULL)
        return;

    qDebug() << Q_FUNC_INFO << "Got meta data:" << key;
    if (key == "Resolution")
        m_video->setResolution(data.toSize());
    else if (key == "VideoCodec")
        m_video->setVideoCodec(data.toString());
    else if (key == "AudioCodec")
        m_video->setAudioCodec(data.toString());
}

void VideoWidget::slotPlaybackVideo()
{
    if (m_videoWidget != NULL)
    {
        //m_videoWidget->deleteLater();
        delete m_videoWidget;
        m_videoWidget = NULL;
    }

    m_videoWidget = new QVideoWidget;
    //m_videoWidget->moveToThread(QCoreApplication::instance()->thread());
    m_videoPlayer->setVideoOutput(m_videoWidget);

    if (m_video->getStartTime() != UINT_MAX)
        m_videoPlayer->setPosition(m_video->getStartTime());
    else
        m_videoPlayer->setPosition(0);

    int screen = m_video->screen();
    QRect rect = qApp->desktop()->screenGeometry(screen);

    if (m_video->fullscreen() == false)
    {
        QSize resolution = m_video->resolution();
        if (resolution.isEmpty())
            m_videoWidget->setGeometry(0, 50, 640, 480);
        else
            m_videoWidget->setGeometry(0, 50, resolution.width(), resolution.height());
    }
    else
    {
        m_videoWidget->setGeometry(rect);
    }

    if (screen > 0 && getScreenCount() > screen)
    {
        m_videoWidget->move(rect.topLeft());
    }

    if (m_video->fullscreen() == true)
        m_videoWidget->setFullScreen(true);

    m_videoWidget->show();

    m_videoPlayer->play();
}

void VideoWidget::slotStopVideo()
{
    slotStatusChanged(QMediaPlayer::EndOfMedia);
}

void VideoWidget::slotBrightnessAdjust(int value)
{
    if (m_videoWidget != NULL)
        m_videoWidget->setBrightness(value);
}

int VideoWidget::getScreenCount()
{
    int screenCount = 0;
    QDesktopWidget *desktop = qApp->desktop();
    if (desktop != NULL)
        screenCount = desktop->screenCount();

    return screenCount;
}
