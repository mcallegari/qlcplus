/*
  Q Light Controller Plus
  audiodecoder.h

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

// Copyright (c) 2000-2001 Brad Hughes <bhughes@trolltech.com>
//
// Use, modification and distribution is allowed without limitation,
// warranty, or liability of any kind.
//

#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QtPlugin>
#include <QStringList>

#include "audioparameters.h"

/** @addtogroup engine_audio Audio
 * @{
 */

/*! @brief The AudioDecoder class provides the base interface class of audio decoders.
 * @author Brad Hughes <bhughes@trolltech.com>
 * @author Ilya Kotov <forkotov@hotmail.ru>
 */
class AudioDecoder: public QObject
{
    Q_OBJECT

public:
    /*!
     * Destructor.
     */
    virtual ~AudioDecoder() { /* NOP */ }

    virtual AudioDecoder *createCopy() = 0;

    /*!
     * Returns the plugin priority
     * Subclass should reimplement this function.
     */
    virtual int priority() const = 0;
    /*!
     * Returns a list of extensions for the supported file formats
     * Subclass should reimplement this function.
     */
    virtual QStringList supportedFormats() = 0;
    /*!
     * Prepares decoder for usage.
     * Subclass should reimplement this function.
     */
    virtual bool initialize(const QString &path) = 0;
    /*!
     * Returns the total time in milliseconds.
     * Subclass should reimplement this function.
     */
    virtual qint64 totalTime() = 0;
    /*!
     * Requests a seek to the time \b time indicated, specified in milliseconds.
     */
    virtual void seek(qint64 time) = 0;
    /*!
     * Reads up to \b maxSize bytes of decoded audio to \b data
     * Returns the number of bytes read, or -1 if an error occurred.
     * In most cases subclass should reimplement this function.
     */
    virtual qint64 read(char *data, qint64 maxSize) = 0;
    /*!
     * Returns current bitrate (in kbps).
     * Subclass should reimplement this function.
     */
    virtual int bitrate() = 0;

    /*!
     * Returns detected audio parameters.
     */
    AudioParameters audioParameters() const;

protected:
    /*!
     * Use this function inside initialize() reimplementation to tell other plugins about audio parameters.
     * @param srate Sample rate.
     * @param chan Number of channels.
     * @param f Audio format.
     */
    void configure(quint32 srate = 44100, int chan = 2, AudioFormat f = PCM_S16LE);

private:
    AudioParameters m_parameters;
};

#define QLCPlusAudioPlugin_iid "org.qlcplus.AudioPlugin"

Q_DECLARE_INTERFACE(AudioDecoder, QLCPlusAudioPlugin_iid)

/** @} */

#endif // AUDIODECODER_H
