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
    , m_fullscreenContext(nullptr)
{
    Q_ASSERT(doc != nullptr);

    qmlRegisterUncreatableType<Video>("org.qlcplus.classes", 1, 0, "VideoFunction", "Can't create a Video!");

    for (Function *f : m_doc->functionsByType(Function::VideoType))
        slotFunctionAdded(f->id());

    connect(m_doc, SIGNAL(functionAdded(quint32)), this, SLOT(slotFunctionAdded(quint32)));
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
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
    if (context == nullptr && m_fullscreenContext)
        m_fullscreenContext->deleteLater();

    m_fullscreenContext = context;
}

void VideoProvider::slotFunctionAdded(quint32 id)
{
    Function *func = m_doc->function(id);
    if (func == nullptr || func->type() != Function::VideoType)
        return;

    Video *video = qobject_cast<Video *>(func);
    m_videoMap[id] = new VideoContent(video, this);

    connect(video, SIGNAL(requestPlayback()), this, SLOT(slotRequestPlayback()));
    connect(video,SIGNAL(requestPause(bool)), this, SLOT(slotRequestPause(bool)));
    connect(video, SIGNAL(requestStop()), this, SLOT(slotRequestStop()));
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
    if (video == nullptr)
        return;

    if (m_videoMap.contains(video->id()))
        m_videoMap[video->id()]->playContent();
}

void VideoProvider::slotRequestPause(bool enable)
{
    Q_UNUSED(enable)
}

void VideoProvider::slotRequestStop()
{
    Video *video = qobject_cast<Video *>(sender());
    if (video == nullptr)
        return;

    if (m_videoMap.contains(video->id()))
        m_videoMap[video->id()]->stopContent();
}

/*********************************************************************
 * VideoContent class implementation
 *********************************************************************/

VideoContent::VideoContent(Video *video, VideoProvider *parent)
    : m_provider(parent)
    , m_video(video)
    , m_mediaPlayer(nullptr)
    , m_viewContext(nullptr)
{
    Q_ASSERT(video != nullptr);

    if (video->fullscreen() == false)
        slotDetectResolution();

    connect(m_video, SIGNAL(sourceChanged(QString)),
            this, SLOT(slotDetectResolution()));
    connect(m_video, SIGNAL(attributeChanged(int,qreal)),
            this, SLOT(slotAttributeChanged(int,qreal)));
}

quint32 VideoContent::id() const
{
    return m_video->id();
}

void VideoContent::destroyContext()
{
    if (m_video->fullscreen())
    {
        m_provider->setFullscreenContext(nullptr);
    }
    else if (m_viewContext)
    {
        m_viewContext->deleteLater();
    }

    m_viewContext = nullptr;
}

void VideoContent::playContent()
{
    QScreen *vScreen = nullptr;

    if (m_video->fullscreen())
        m_viewContext = m_provider->fullscreenContext();

    if (m_video->isPicture())
    {
        m_geometry.setSize(m_video->resolution());
    }
    else if (!m_video->customGeometry().isNull())
    {
        m_geometry = m_video->customGeometry();
    }

    qDebug() << "Video screen:" << m_video->screen() << ", geometry:" << m_geometry;

    if (m_viewContext == nullptr)
    {
        QList<QScreen *> screens = QGuiApplication::screens();

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

    m_viewContext->setFlags(m_viewContext->flags() | Qt::WindowStaysOnTopHint);

    if (vScreen && m_video->fullscreen())
    {
        m_provider->setFullscreenContext(m_viewContext);
        m_viewContext->showFullScreen();
    }
    else
        m_viewContext->show();
}

void VideoContent::stopContent()
{
    if (m_viewContext == nullptr)
        return;

    QMetaObject::invokeMethod(m_viewContext->rootObject(), "removeContent",
                              Q_ARG(QVariant, m_video->id()));
}

void VideoContent::slotDetectResolution()
{
    if (m_video->isPicture())
        return;

    m_mediaPlayer = new QMediaPlayer();

    connect(m_mediaPlayer, SIGNAL(metaDataChanged(QString,QVariant)),
                this, SLOT(slotMetaDataChanged(QString,QVariant)));

    QString sourceURL = m_video->sourceUrl();
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

QVariant VideoContent::getAttribute(quint32 id, const char *propName)
{
    QQuickItem *item = qobject_cast<QQuickItem*>(m_viewContext->findChild<QQuickItem*>(QString("media-%1").arg(id)));
    if (item)
        return item->property(propName);

    return QVariant();
}

void VideoContent::updateAttribute(quint32 id, const char *propName, QVariant value)
{
    QQuickItem *item = qobject_cast<QQuickItem*>(m_viewContext->findChild<QQuickItem*>(QString("media-%1").arg(id)));
    if (item)
        item->setProperty(propName, value);
}

void VideoContent::slotAttributeChanged(int attrIndex, qreal value)
{
    switch (attrIndex)
    {
        case Video::Volume:
        {
            updateAttribute(m_video->id(), "volume", float(value / 100.0));
        }
        break;
        case Video::XRotation:
        {
            QVector3D rot = m_video->rotation();
            rot.setX(float(value));
            updateAttribute(m_video->id(), "rotation", rot);
        }
        break;
        case Video::YRotation:
        {
            QVector3D rot = m_video->rotation();
            rot.setY(float(value));
            updateAttribute(m_video->id(), "rotation", rot);
        }
        break;
        case Video::ZRotation:
        {
            QVector3D rot = m_video->rotation();
            rot.setZ(float(value));
            updateAttribute(m_video->id(), "rotation", rot);
        }
        break;
        case Video::XPosition:
        {
            qreal xDelta = qreal(m_viewContext->width()) * (value / 100.0);
            QVariant var = getAttribute(m_video->id(), "geometry");
            QRect currGeom = var.isNull() ? m_geometry : var.toRect();
            QRect geom(m_geometry.x() + int(xDelta), currGeom.y(),
                       currGeom.width(), currGeom.height());
            updateAttribute(m_video->id(), "geometry", geom);
        }
        break;
        case Video::YPosition:
        {
            qreal yDelta = qreal(m_viewContext->height()) * (value / 100.0);
            QVariant var = getAttribute(m_video->id(), "geometry");
            QRect currGeom = var.isNull() ? m_geometry : var.toRect();
            QRect geom(currGeom.x(), m_geometry.y() + int(yDelta),
                       currGeom.width(), currGeom.height());
            updateAttribute(m_video->id(), "geometry", geom);
        }
        break;
        case Video::WidthScale:
        {
            QVariant var = getAttribute(m_video->id(), "geometry");
            QRect geom = var.isNull() ? m_geometry : var.toRect();
            qreal newWidth = qreal(m_geometry.width()) * (value / 100.0);
            geom.setWidth(int(newWidth));
            updateAttribute(m_video->id(), "geometry", geom);
        }
        break;
        case Video::HeightScale:
        {
            QVariant var = getAttribute(m_video->id(), "geometry");
            QRect geom = var.isNull() ? m_geometry : var.toRect();
            qreal newHeight = qreal(m_geometry.height()) * (value / 100.0);
            geom.setHeight(int(newHeight));
            updateAttribute(m_video->id(), "geometry", geom);
        }
        break;
        default:
        break;
    }
}

void VideoContent::slotMetaDataChanged(const QString &key, const QVariant &value)
{
    if (key == "Resolution")
    {
        m_geometry.setSize(value.toSize());

        disconnect(m_mediaPlayer, SIGNAL(metaDataChanged(QString,QVariant)),
                    this, SLOT(slotMetaDataChanged(QString,QVariant)));
        m_mediaPlayer->deleteLater();
        m_mediaPlayer = nullptr;
    }
}

void VideoContent::slotWindowClosing()
{
    stopContent();
}
