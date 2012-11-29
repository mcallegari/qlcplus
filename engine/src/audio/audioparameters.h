/*
  Q Light Controller Plus
  audioparameters.h

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

#ifndef AUDIOPARAMETERS_H
#define AUDIOPARAMETERS_H

#include <QtGlobal>

/*!
 * Audio formats
 */
enum AudioFormat
{
    PCM_UNKNOWN = -1, /*!< Unknown format */
    PCM_S8 = 0, /*!< Signed 8 bit */
    PCM_S16LE,  /*!< Signed 16 bit Little Endian */
    PCM_S24LE,  /*!< Signed 24 bit Little Endian using low three bytes in 32-bit word */
    PCM_S32LE   /*!< Signed 32 bit Little Endian */
};

/*! @brief The AudioParameters class keeps information about audio settings.
 *  @author Ilya Kotov <forkotov02@hotmail.ru>
 */
class AudioParameters
{
public:
    /*!
     * Contsructor.
     */
    AudioParameters();
    /*!
     * Constructs audio settings with the given parameters.
     * @param srate Sampling rate.
     * @param chan Number of channels.
     * @param format PCM data format.
     */
    AudioParameters(quint32 srate, int chan, AudioFormat format);
    /*!
     * Constructs a copy of \b other.
     */
    AudioParameters(const AudioParameters &other);
    /*!
     * Assigns audio parameters \b p to this parameters.
     */
    void operator=(const AudioParameters &p);
    /*!
     * Returns \b true if parameters \b p is equal to this parameters; otherwise returns \b false.
     */
    bool operator==(const AudioParameters &p) const;
    /*!
     * Returns \b true if parameters \b p is not equal to this parameters; otherwise returns \b false.
     */
    bool operator!=(const AudioParameters &p) const;
    /*!
     * Returns sample rate in Hz.
     */
    quint32 sampleRate() const;
    /*!
     * Returns number of channels.
     */
    int channels() const;
    /*!
     * Returns pcm format.
     */
    AudioFormat format() const;
    /*!
     * Returns sample size in bytes.
     */
    int sampleSize() const;
    /*!
     * Returns sample size in bytes of the given pcm data \b format.
     */
    static int sampleSize(AudioFormat format);

private:
    quint32 m_srate;
    int m_chan;
    AudioFormat m_format;
};

#endif