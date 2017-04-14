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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMediaPlayer>
#include <QDebug>
#include <QFile>

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
  : Function(doc, Function::VideoType)
  , m_doc(doc)
  , m_startTime(UINT_MAX)
  , m_color(147, 140, 20)
  , m_locked(false)
  , m_sourceUrl("")
  , m_videoDuration(0)
  , m_resolution(QSize(0,0))
  , m_screen(0)
  , m_fullscreen(false)
{
    setName(tr("New Video"));
    setRunOrder(Video::SingleShot);

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Video::~Video()
{
}

QIcon Video::getIcon() const
{
    return QIcon(":/video.png");
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

    setSourceUrl(vid->m_sourceUrl);
    m_videoDuration = vid->m_videoDuration;
    m_color = vid->m_color;

    return Function::copyFrom(function);
}

QStringList Video::getCapabilities()
{
    QStringList caps;
    QStringList mimeTypes = QMediaPlayer::supportedMimeTypes();
    qDebug() << "Supported video types:" << caps;
    if (mimeTypes.isEmpty())
        caps << "*.avi" << "*.wmv" << "*.mkv" << "*.mp4" << "*.mpg" << "*.mpeg" << "*.flv";
    else
    {
        foreach(QString mime, mimeTypes)
        {
            if (mime.startsWith("video/"))
            {
                if (mime.endsWith("/3gpp")) caps << "*.3gp";
                else if (mime.endsWith("/mp4")) caps << "*.mp4";
                else if (mime.endsWith("/avi")) caps << "*.avi";
                else if (mime.endsWith("/m2ts")) caps << "*.m2ts";
                else if (mime.endsWith("m4v")) caps << "*.m4v";
                else if (mime.endsWith("/mpeg")) caps << "*.mpeg";
                else if (mime.endsWith("/mpg")) caps << "*.mpg";
                else if (mime.endsWith("/quicktime")) caps << "*.mov";
                else if (mime.endsWith("matroska")) caps << "*.mkv";
            }
        }
    }
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

void Video::setTotalDuration(quint32 duration)
{
    m_videoDuration = (qint64)duration;
    emit totalTimeChanged(m_videoDuration);
}

quint32 Video::totalDuration()
{
    return (quint32)m_videoDuration;
}

void Video::setResolution(QSize size)
{
    m_resolution = size;
    emit metaDataChanged("Resolution", QVariant(m_resolution));
}

QSize Video::resolution()
{
    return m_resolution;
}

void Video::setAudioCodec(QString codec)
{
    m_audioCodec = codec;
    emit metaDataChanged("AudioCodec", QVariant(m_audioCodec));
}

QString Video::audioCodec()
{
    return m_audioCodec;
}

void Video::setVideoCodec(QString codec)
{
    m_videoCodec = codec;
    emit metaDataChanged("VideoCodec", QVariant(m_videoCodec));
}

QString Video::videoCodec()
{
    return m_videoCodec;
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

bool Video::setSourceUrl(QString filename)
{
    m_sourceUrl = filename;
    qDebug() << Q_FUNC_INFO << "Source name set:" << m_sourceUrl;

    if (m_sourceUrl.contains("://"))
    {
        QUrl url(m_sourceUrl);
        setName(url.fileName());
    }
    else
    {
        if (QFile(m_sourceUrl).exists())
            setName(QFileInfo(m_sourceUrl).fileName());
        else
            setName(tr("File not found"));
    }

    emit sourceChanged(m_sourceUrl);

    return true;
}

QString Video::sourceUrl()
{
    return m_sourceUrl;
}

void Video::setScreen(int index)
{
    m_screen = index;
    emit changed(id());
}

int Video::screen()
{
    return m_screen;
}

void Video::setFullscreen(bool enable)
{
    m_fullscreen = enable;
    emit changed(id());
}

bool Video::fullscreen()
{
    return m_fullscreen;
}

void Video::adjustAttribute(qreal fraction, int attributeIndex)
{
    if (attributeIndex == Function::Intensity)
    {
        int b = -100 - (int)((qreal)-100.0 * fraction);
        emit requestBrightnessAdjust(b);
    }
    Function::adjustAttribute(fraction, attributeIndex);
}

void Video::slotFunctionRemoved(quint32 fid)
{
    Q_UNUSED(fid)
}

/*********************************************************************
 * Save & Load
 *********************************************************************/

bool Video::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    /* Speed */
    saveXMLSpeed(doc);

    /* Playback mode */
    saveXMLRunOrder(doc);

    doc->writeStartElement(KXMLQLCVideoSource);
    if (m_screen > 0)
        doc->writeAttribute(KXMLQLCVideoScreen, QString::number(m_screen));
    if (m_fullscreen == true)
        doc->writeAttribute(KXMLQLCVideoFullscreen, "1");

    if (m_sourceUrl.contains("://"))
        doc->writeCharacters(m_sourceUrl);
    else
        doc->writeCharacters(m_doc->normalizeComponentPath(m_sourceUrl));

    doc->writeEndElement();

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

bool Video::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::VideoType))
    {
        qWarning() << Q_FUNC_INFO << root.attributes().value(KXMLQLCFunctionType).toString()
                   << "is not Video";
        return false;
    }

    QString fname = name();

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVideoSource)
        {
            QXmlStreamAttributes attrs = root.attributes();
            if (attrs.hasAttribute(KXMLQLCVideoStartTime))
                setStartTime(attrs.value(KXMLQLCVideoStartTime).toString().toUInt());
            if (attrs.hasAttribute(KXMLQLCVideoColor))
                setColor(QColor(attrs.value(KXMLQLCVideoColor).toString()));
            if (attrs.hasAttribute(KXMLQLCVideoLocked))
                setLocked(true);
            if (attrs.hasAttribute(KXMLQLCVideoScreen))
                setScreen(attrs.value(KXMLQLCVideoScreen).toString().toInt());
            if (attrs.hasAttribute(KXMLQLCVideoFullscreen))
            {
                if (attrs.value(KXMLQLCVideoFullscreen).toString() == "1")
                    setFullscreen(true);
                else
                    setFullscreen(false);
            }
            QString path = root.readElementText();
            if (path.contains("://") == true)
                setSourceUrl(path);
            else
                setSourceUrl(m_doc->denormalizeComponentPath(path));
        }
        else if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(root);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Video tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    setName(fname);

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
    emit requestPlayback();
    Function::preRun(timer);
}

void Video::setPause(bool enable)
{
    if (isRunning())
    {
        emit requestPause(enable);
        Function::setPause(enable);
    }
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
    emit requestStop();
    Function::postRun(timer, universes);
}
