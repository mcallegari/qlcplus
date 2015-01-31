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

#include <QMediaPlayer>
#include <QDomDocument>
#include <QDomElement>
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
  : Function(doc, Function::Video)
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

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Video::~Video()
{
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

void Video::setTotalDuration(qint64 duration)
{
    m_videoDuration = duration;
    emit totalTimeChanged(m_videoDuration);
}

qint64 Video::totalDuration()
{
    return m_videoDuration;
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

    if (m_sourceUrl.contains("://"))
        text = doc->createTextNode(m_sourceUrl);
    else
        text = doc->createTextNode(m_doc->normalizeComponentPath(m_sourceUrl));

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
            if (tag.text().contains("://") == true)
                setSourceUrl(tag.text());
            else
                setSourceUrl(m_doc->denormalizeComponentPath(tag.text()));
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
    emit requestPlayback();
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
    emit requestStop();
    Function::postRun(timer, universes);
}
