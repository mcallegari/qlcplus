/*
  Q Light Controller
  audio.h

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

#ifndef AUDIO_H
#define AUDIO_H

#include <QColor>

#ifdef QT_PHONON_LIB
#include <phonon/mediaobject.h>
#include <phonon/backendcapabilities.h>
#endif

#include "audiorenderer.h"
#include "audiodecoder.h"
#include "function.h"

class QDomDocument;

class Audio : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Audio)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Audio(Doc* doc);
    virtual ~Audio();

private:
    Doc *m_doc;
    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

public slots:
    /** Catches Doc::functionRemoved() so that destroyed members can be
        removed immediately. */
    void slotFunctionRemoved(quint32 function);

    /*********************************************************************
     * Capabilities
     *********************************************************************/
public:
    static QStringList getCapabilities();

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /**
     * Set the time where the Audio object is placed over a timeline
     *
     * @param time The start time in milliseconds of the Audio object
     */
    void setStartTime(quint32 time);

    /**
     * Returns the time where the Audio object is placed over a timeline
     *
     * @return Start time in milliseconds of the Audio object
     */
    quint32 getStartTime() const;

    /**
     * Returns the duration of the source audio file loaded
     *
     * @return Duration in milliseconds of the source audio file
     */
    qint64 getDuration();

    /**
     * Set the color to be used by a AudioItem
     */
    void setColor(QColor color);

    /**
     * Get the color of this Audio object
     */
    QColor getColor();

    /**
     * Set the source file name used by this Audio object
     */
    bool setSourceFileName(QString filename);

    /**
     * Retrieve the source file name used by this Audio object
     */
    QString getSourceFileName();

    /**
     * Retrieve the currently associated audio decoder
     */
    AudioDecoder* getAudioDecoder();

    void adjustAttribute(qreal fraction, int attributeIndex);

protected slots:
    void slotEndOfStream();

private:
#ifdef QT_PHONON_LIB
    Phonon::MediaObject *m_object;
#endif
    /** Instance of an AudioDecoder to perform actual audio decoding */
    AudioDecoder *m_decoder;
    /** output interface to render audio data got from m_decoder */
    AudioRenderer *m_audio_out;
    /** Absolute start time of Audio over a timeline (in milliseconds) */
    quint32 m_startTime;
    /** Color to use when displaying the audio object in the Show manager */
    QColor m_color;
    /** Name of the source audio file */
    QString m_sourceFileName;
    /** Duration of the media object */
    qint64 m_audioDuration;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save function's contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement*);

    /** Load function's contents from an XML document */
    bool loadXML(const QDomElement&);

    /** @reimp */
    void postLoad();

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void preRun(MasterTimer*);

    /** @reimpl */
    void write(MasterTimer* timer, QList<Universe*> universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, QList<Universe *> universes);

protected slots:
    void slotTotalTimeChanged(qint64);

signals:
    void totalTimeChanged(qint64);
};

#endif
