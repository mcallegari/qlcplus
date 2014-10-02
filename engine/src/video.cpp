/*
  Q Light Controller Plus
  video.cpp

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

#include <QDomDocument>
#include <QVideoWidget>
#include <QDomElement>
#include <QDesktopWidget>
#include <QApplication>

#include <QDebug>
#include <QFile>

#include <QMessageBox>

#include "video.h"
#include "doc.h"

#define KXMLQLCVideoSource "Source"
#define KXMLQLCVideoStartTime "StartTime"
#define KXMLQLCVideoColor "Color"
#define KXMLQLCVideoLocked "Locked"
#define KXMLQLCVideoScreen "Screen"
#define KXMLQLCVideoFullscreen "Fullscreen"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Video::Video(Doc* doc)
  : Function(doc, Function::Video)
  , m_doc(doc)
  , m_videoPlayer(NULL)
  , m_startTime(UINT_MAX)
  , m_color(147, 140, 20)
  , m_locked(false)
  , m_sourceFileName("")
  , m_videoDuration(0)
  , m_resolution(QSize(0,0))
  , m_screen(0)
  , m_fullscreen(false)
{
    setName(tr("New Video"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Video::~Video()
{
    if (m_videoPlayer != NULL)
    {
        m_videoPlayer->stop();
        delete m_videoPlayer;
    }
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Video::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Video(doc);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Video::copyFrom(const Function* function)
{
    const Video* vid = qobject_cast<const Video*> (function);
    if (vid == NULL)
        return false;

    setSourceFileName(vid->m_sourceFileName);
    m_videoDuration = vid->m_videoDuration;
    m_color = vid->m_color;

    return Function::copyFrom(function);
}

QStringList Video::getCapabilities()
{
    QStringList caps = QMediaPlayer::supportedMimeTypes();
    qDebug() << "Supported video types:" << caps;
    if (caps.isEmpty())
        caps << "*.avi" << "*.wmv" << "*.mkv" << "*.mp4" << "*.mpg";
    return caps;
}

int Video::getScreenCount()
{
    QDesktopWidget *desktop = qApp->desktop();
    if (desktop != NULL)
        return desktop->screenCount();

    return -1;
}

/*********************************************************************
 * Properties
 *********************************************************************/
void Video::setStartTime(quint32 time)
{
    m_startTime = time;
}

quint32 Video::getStartTime() const
{
    return m_startTime;
}

qint64 Video::totalDuration()
{
    return m_videoDuration;
}

void Video::setColor(QColor color)
{
    m_color = color;
}

QColor Video::getColor()
{
    return m_color;
}

void Video::setLocked(bool locked)
{
    m_locked = locked;
}

bool Video::isLocked()
{
    return m_locked;
}

bool Video::setSourceFileName(QString filename)
{
    m_sourceFileName = filename;
    qDebug() << Q_FUNC_INFO << "Source name set:" << m_sourceFileName;

    if (QFile(m_sourceFileName).exists())
    {
        setName(QFileInfo(m_sourceFileName).fileName());
        m_videoPlayer = new QMediaPlayer(0, QMediaPlayer::VideoSurface);
        m_videoWidget = new QVideoWidget;
        m_videoPlayer->setVideoOutput(m_videoWidget);
        m_videoPlayer->setMedia(QUrl::fromLocalFile(m_sourceFileName));
        connect(m_videoPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                this, SLOT(slotStatusChanged(QMediaPlayer::MediaStatus)));
        connect(m_videoPlayer, SIGNAL(metaDataChanged(QString,QVariant)),
                this, SIGNAL(metaDataChanged(QString,QVariant)));
        connect(m_videoPlayer, SIGNAL(durationChanged(qint64)),
                this, SLOT(slotTotalTimeChanged(qint64)));
    }
    else
    {
        setName(tr("File not found"));
        return true;
    }

    return true;
}

QString Video::getSourceFileName()
{
    return m_sourceFileName;
}

void Video::setScreen(int index)
{
    m_screen = index;
}

int Video::screen()
{
    return m_screen;
}

void Video::setFullscreen(bool enable)
{
    m_fullscreen = enable;
}

bool Video::fullscreen()
{
    return m_fullscreen;
}

void Video::adjustAttribute(qreal fraction, int attributeIndex)
{
    //if (m_video_out != NULL && attributeIndex == Intensity)
    //    m_video_out->adjustIntensity(fraction);
    Function::adjustAttribute(fraction, attributeIndex);
}

void Video::slotStatusChanged(QMediaPlayer::MediaStatus status)
{
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
            {
                m_videoPlayer->stop();
                m_videoWidget->hide();
                /*m_videoWidget->deleteLater();
                delete m_videoWidget;
                m_videoWidget = NULL;

                delete m_videoPlayer;
                m_videoPlayer = NULL;*/
            }
            Function::postRun(NULL, QList<Universe *>());
            break;
        }
        case QMediaPlayer::InvalidMedia:
            //displayErrorMessage();
            break;
    }
}

void Video::slotTotalTimeChanged(qint64 duration)
{
    m_videoDuration = duration;
    qDebug() << "Video duration: " << m_videoDuration;
    emit totalTimeChanged(m_videoDuration);
}

void Video::slotFunctionRemoved(quint32 fid)
{
    Q_UNUSED(fid)
}

/*********************************************************************
 * Save & Load
 *********************************************************************/

bool Video::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    /* Common attributes */
    saveXMLCommon(&root);

    /* Speed */
    saveXMLSpeed(doc, &root);

    QDomElement source = doc->createElement(KXMLQLCVideoSource);
    if (m_screen > 0)
        source.setAttribute(KXMLQLCVideoScreen, m_screen);
    if (m_fullscreen == true)
        source.setAttribute(KXMLQLCVideoFullscreen, true);

    text = doc->createTextNode(m_doc->normalizeComponentPath(m_sourceFileName));

    source.appendChild(text);
    root.appendChild(source);

    return true;
}

bool Video::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Video))
    {
        qWarning() << Q_FUNC_INFO << root.attribute(KXMLQLCFunctionType)
                   << "is not Video";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCVideoSource)
        {
            if (tag.hasAttribute(KXMLQLCVideoStartTime))
                setStartTime(tag.attribute(KXMLQLCVideoStartTime).toUInt());
            if (tag.hasAttribute(KXMLQLCVideoColor))
                setColor(QColor(tag.attribute(KXMLQLCVideoColor)));
            if (tag.hasAttribute(KXMLQLCVideoLocked))
                setLocked(true);
            if (tag.hasAttribute(KXMLQLCVideoScreen))
                setScreen(tag.attribute(KXMLQLCVideoScreen).toInt());
            if (tag.hasAttribute(KXMLQLCVideoFullscreen))
            {
                if (tag.attribute(KXMLQLCVideoFullscreen) == "1")
                    setFullscreen(true);
                else
                    setFullscreen(false);
            }
            setSourceFileName(m_doc->denormalizeComponentPath(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(tag);
        }
        node = node.nextSibling();
    }

    return true;
}

void Video::postLoad()
{
}

/*********************************************************************
 * Running
 *********************************************************************/
void Video::preRun(MasterTimer* timer)
{
    if (m_startTime != UINT_MAX)
        m_videoPlayer->setPosition(m_startTime);
    if (m_screen > 0 && getScreenCount() > m_screen)
    {
        QRect rect = qApp->desktop()->screenGeometry(m_screen);
        m_videoWidget->move(rect.topLeft());
    }
    if (m_fullscreen == true)
        m_videoWidget->setFullScreen(true);
    m_videoWidget->show();

    m_videoPlayer->play();
    Function::preRun(timer);
}

void Video::write(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(timer)
    Q_UNUSED(universes)

    incrementElapsed();
/*
    if (fadeOutSpeed() != 0)
    {
        if (getDuration() - elapsed() <= fadeOutSpeed())
            m_audio_out->setFadeOut(fadeOutSpeed());
    }
*/
}

void Video::postRun(MasterTimer* timer, QList<Universe*> universes)
{
    Q_UNUSED(timer)
    Q_UNUSED(universes)
    slotStatusChanged(QMediaPlayer::EndOfMedia);
}
