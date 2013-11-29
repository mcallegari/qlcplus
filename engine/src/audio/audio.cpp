/*
  Q Light Controller
  audio.cpp

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
#include <QDomElement>
#include <QDebug>
#include <QFile>

#include <QMessageBox>

#ifdef QT_PHONON_LIB
#include <phonon/mediaobject.h>
#include <phonon/backendcapabilities.h>
#endif

#include "audiodecoder.h"
#ifdef HAS_LIBSNDFILE
  #include "audiodecoder_sndfile.h"
#endif
#ifdef HAS_LIBMAD
  #include "audiodecoder_mad.h"
#endif

#include "audiorenderer.h"

#if defined(__APPLE__) || defined(Q_OS_MAC)
  //#include "audiorenderer_coreaudio.h"
  #include "audiorenderer_portaudio.h"
#elif defined(WIN32) || defined(Q_OS_WIN)
  #include "audiorenderer_waveout.h"
#else
  #include "audiorenderer_alsa.h"
#endif
#include "audio.h"
#include "doc.h"

#define KXMLQLCAudioSource "Source"
#define KXMLQLCAudioStartTime "StartTime"
#define KXMLQLCAudioColor "Color"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Audio::Audio(Doc* doc)
  : Function(doc, Function::Audio)
  , m_doc(doc)
#ifdef QT_PHONON_LIB
  , m_object(NULL)
#endif
  , m_decoder(NULL)
  , m_audio_out(NULL)
  , m_startTime(UINT_MAX)
  , m_color(96, 128, 83)
  , m_sourceFileName("")
  , m_audioDuration(0)
{
    setName(tr("New Audio"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Audio::~Audio()
{
    if (m_audio_out != NULL)
    {
        m_audio_out->stop();
        delete m_audio_out;
    }
    if (m_decoder != NULL)
        delete m_decoder;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Audio::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Audio(doc);
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

bool Audio::copyFrom(const Function* function)
{
    const Audio* aud = qobject_cast<const Audio*> (function);
    if (aud == NULL)
        return false;

    setSourceFileName(aud->m_sourceFileName);
    m_audioDuration = aud->m_audioDuration;
    m_color = aud->m_color;

    return Function::copyFrom(function);
}

QStringList Audio::getCapabilities()
{
    QStringList cap;
#ifdef QT_PHONON_LIB
    return Phonon::BackendCapabilities::availableMimeTypes();
#endif
#ifdef HAS_LIBSNDFILE
    cap << AudioDecoderSndFile::getSupportedFormats();
#endif
#ifdef HAS_LIBMAD
    cap << AudioDecoderMAD::getSupportedFormats();
#endif
    return cap;
}

/*********************************************************************
 * Properties
 *********************************************************************/
void Audio::setStartTime(quint32 time)
{
    m_startTime = time;
}

quint32 Audio::getStartTime() const
{
    return m_startTime;
}

qint64 Audio::getDuration()
{
    return m_audioDuration;
}

void Audio::setColor(QColor color)
{
    m_color = color;
}

QColor Audio::getColor()
{
    return m_color;
}

bool Audio::setSourceFileName(QString filename)
{
    if (m_sourceFileName.isEmpty() == false)
    {
        // unload previous source
        if (m_decoder != NULL)
        {
            delete m_decoder;
            m_decoder = NULL;
        }
    }

#ifdef QT_PHONON_LIB
    m_object = Phonon::createPlayer(Phonon::MusicCategory,
                                    Phonon::MediaSource(filename));
    if (m_object == NULL)
        return false;
#endif
    m_sourceFileName = filename;

    //QMessageBox::warning(0,"Warning", QString("File complete path: %1").arg(m_sourceFileName));

    if (QFile(m_sourceFileName).exists())
        setName(QFileInfo(m_sourceFileName).fileName());
    else
    {
        setName(tr("File not found"));
        return true;
    }

#ifdef QT_PHONON_LIB
    connect(m_object, SIGNAL(totalTimeChanged(qint64)), this, SLOT(slotTotalTimeChanged(qint64)));
#endif

#ifdef HAS_LIBSNDFILE
    m_decoder = new AudioDecoderSndFile(m_sourceFileName);
    if (m_decoder->initialize() == false)
    {
        delete m_decoder;
        m_decoder = NULL;
    }
    else
    {
        m_audioDuration = m_decoder->totalTime();
        return true;
    }
#endif
#ifdef HAS_LIBMAD
    m_decoder = new AudioDecoderMAD(m_sourceFileName);
    if (m_decoder->initialize() == false)
    {
        delete m_decoder;
        m_decoder = NULL;
    }
    else
    {
        m_audioDuration = m_decoder->totalTime();
        return true;
    }
#endif
    return false;
}

QString Audio::getSourceFileName()
{
    return m_sourceFileName;
}

AudioDecoder* Audio::getAudioDecoder()
{
    return m_decoder;
}

void Audio::adjustAttribute(qreal fraction, int attributeIndex)
{
    if (m_audio_out != NULL && attributeIndex == Intensity)
        m_audio_out->adjustIntensity(fraction);
    Function::adjustAttribute(fraction, attributeIndex);
}

void Audio::slotEndOfStream()
{
#ifdef QT_PHONON_LIB
    if (m_object != NULL)
        m_object->stop();
#endif
    if (m_audio_out != NULL)
    {
        m_audio_out->stop();
        delete m_audio_out;
        m_audio_out = NULL;
        m_decoder->seek(0);
    }
    Function::postRun(NULL, NULL);
}

void Audio::slotTotalTimeChanged(qint64)
{
#ifdef QT_PHONON_LIB
    m_audioDuration = m_object->totalTime();
#endif
    qDebug() << "Audio duration: " << m_audioDuration;
    emit totalTimeChanged(m_audioDuration);
}

void Audio::slotFunctionRemoved(quint32 fid)
{
    Q_UNUSED(fid)
}

/*********************************************************************
 * Save & Load
 *********************************************************************/

bool Audio::saveXML(QDomDocument* doc, QDomElement* wksp_root)
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

    QDomElement source = doc->createElement(KXMLQLCAudioSource);
    source.setAttribute(KXMLQLCAudioStartTime, m_startTime);
    source.setAttribute(KXMLQLCAudioColor, m_color.name());

    text = doc->createTextNode(m_doc->normalizeComponentPath(m_sourceFileName));

    source.appendChild(text);
    root.appendChild(source);

    return true;
}

bool Audio::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Audio))
    {
        qWarning() << Q_FUNC_INFO << root.attribute(KXMLQLCFunctionType)
                   << "is not Audio";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCAudioSource)
        {
            if (tag.hasAttribute(KXMLQLCAudioStartTime))
                m_startTime = tag.attribute(KXMLQLCAudioStartTime).toUInt();
            if (tag.hasAttribute(KXMLQLCAudioColor))
                m_color = QColor(tag.attribute(KXMLQLCAudioColor));
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

void Audio::postLoad()
{
}

/*********************************************************************
 * Running
 *********************************************************************/
void Audio::preRun(MasterTimer* timer)
{
#ifdef QT_PHONON_LIB
    if (m_object != NULL)
    {
        m_object->play();
        return;
    }
#endif
    if (m_decoder != NULL)
    {
        m_decoder->seek(elapsed());
        AudioParameters ap = m_decoder->audioParameters();
#if defined(__APPLE__) || defined(Q_OS_MAC)
        //m_audio_out = new AudioRendererCoreAudio();
        m_audio_out = new AudioRendererPortAudio();
#elif defined(WIN32) || defined(Q_OS_WIN)
        m_audio_out = new AudioRendererWaveOut();
#else
        m_audio_out = new AudioRendererAlsa();
#endif
        m_audio_out->setDecoder(m_decoder);
        m_audio_out->initialize(ap.sampleRate(), ap.channels(), ap.format());
        m_audio_out->setFadeIn(fadeInSpeed());
        m_audio_out->start();
        m_audio_out->adjustIntensity(getAttributeValue(Intensity));
        connect(m_audio_out, SIGNAL(endOfStreamReached()),
                this, SLOT(slotEndOfStream()));
    }
    Function::preRun(timer);
}

void Audio::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer)
    Q_UNUSED(universes)

    incrementElapsed();

    if (fadeOutSpeed() != 0)
    {
        if (getDuration() - elapsed() <= fadeOutSpeed())
            m_audio_out->setFadeOut(fadeOutSpeed());
    }
}

void Audio::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer)
    Q_UNUSED(universes)
    slotEndOfStream();
}
