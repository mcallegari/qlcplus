/*
  Q Light Controller Plus
  audiodecoder.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// Copyright (c) 2000-2001 Brad Hughes <bhughes@trolltech.com>
//
// Use, modification and distribution is allowed without limitation,
// warranty, or liability of any kind.
//

#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QStringList>

#include "audioparameters.h"

/*! @brief The AudioDecoder class provides the base interface class of audio decoders.
 * @author Brad Hughes <bhughes@trolltech.com>
 * @author Ilya Kotov <forkotov@hotmail.ru>
 */
class AudioDecoder
{
public:
    /*!
     * Object constructor.
     * @param input QIODevice-based input source.
     */
    AudioDecoder();
    /*!
     * Destructor.
     */
    virtual ~AudioDecoder();
    /*!
     * Prepares decoder for usage.
     * Subclass should reimplement this function.
     */
    virtual bool initialize() = 0;
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

#endif // AUDIODECODER_H
