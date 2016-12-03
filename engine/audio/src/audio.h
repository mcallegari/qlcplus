/*
  Q Light Controller Plus
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

#include "audiorenderer.h"
#include "audiodecoder.h"
#include "function.h"

class QXmlStreamReader;

/** @addtogroup engine_functions Functions
 * @{
 */

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
    QStringList getCapabilities();

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
    quint32 totalDuration();

    /**
     * Set the playback duration of the audio file
     *
     * @param The playback total duration in milliseconds
     */
    void setTotalDuration(quint32 msec);

    /**
     * Set the color to be used by a AudioItem
     */
    void setColor(QColor color);

    /**
     * Get the color of this Audio object
     */
    QColor getColor();

    /** Set the lock state of the item */
    void setLocked(bool locked);

    /** Get the lock state of the item */
    bool isLocked();

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

    /**
     * Set a specific audio device for rendering. If empty
     * the QLC+ global device will be used
     */
    void setAudioDevice(QString dev);

    /**
     * Retrieve the audio device set for this function
     */
    QString audioDevice();

    void adjustAttribute(qreal fraction, int attributeIndex);

signals:
    void sourceFilenameChanged();

protected slots:
    void slotEndOfStream();

private:
    /** Instance of an AudioDecoder to perform actual audio decoding */
    AudioDecoder *m_decoder;
    /** output interface to render audio data got from m_decoder */
    AudioRenderer *m_audio_out;
    /** Audio device to use for rendering */
    QString m_audioDevice;
    /** Absolute start time of Audio over a timeline (in milliseconds) */
    quint32 m_startTime;
    /** Color to use when displaying the audio object in the Show manager */
    QColor m_color;
    /** Flag to indicate if a Audio item is locked in the Show Manager timeline */
    bool m_locked;
    /** Name of the source audio file */
    QString m_sourceFileName;
    /** Duration of the media object */
    qint64 m_audioDuration;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save function's contents to an XML document */
    bool saveXML(QXmlStreamWriter *doc);

    /** Load function's contents from an XML document */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    void postLoad();

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void preRun(MasterTimer*);

    /** @reimpl */
    void setPause(bool enable);

    /** @reimpl */
    void write(MasterTimer* timer, QList<Universe*> universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, QList<Universe *> universes);
};

/** @} */

#endif
