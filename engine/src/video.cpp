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

#include <QDebug>
#include <QFile>

#include <QMessageBox>

#include "video.h"
#include "doc.h"

#define KXMLQLCVideoSource "Source"
#define KXMLQLCVideoStartTime "StartTime"
#define KXMLQLCVideoColor "Color"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Video::Video(Doc* doc)
  : Function(doc, Function::Video)
  , m_doc(doc)
  , m_videoPlayer(NULL)
  , m_startTime(UINT_MAX)
  , m_color(147, 140, 20)
  , m_sourceFileName("")
  , m_videoDuration(0)
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
    qDebug() << caps;
    return caps;
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

qint64 Video::getDuration()
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
    source.setAttribute(KXMLQLCVideoStartTime, m_startTime);
    source.setAttribute(KXMLQLCVideoColor, m_color.name());

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
                m_startTime = tag.attribute(KXMLQLCVideoStartTime).toUInt();
            if (tag.hasAttribute(KXMLQLCVideoColor))
                m_color = QColor(tag.attribute(KXMLQLCVideoColor));
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
