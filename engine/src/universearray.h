/*
  Q Light Controller
  universearray.h

  Copyright (c) Heikki Junnila

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

#ifndef UNIVERSEARRAY_H
#define UNIVERSEARRAY_H

#include <QByteArray>
#include <QSet>

#include "qlcchannel.h"

class UniverseArray
{
public:
    /** Construct a new UniverseArray of given size */
    UniverseArray(int size);

    /** Destructor */
    virtual ~UniverseArray();

    /** Get the size of the UniverseArray */
    int size() const;

    /**
     * Unapplies Grand Master to all channels and resets their values to 0.
     */
    void reset();

    /**
     * Unapplies Grand Master to the given address range and resets values to 0.
     *
     * @param address Starting address
     * @param range Number of channels, starting from address, to reset
     */
    void reset(int address, int range);

protected:
    const int m_size;

    /************************************************************************
     * Highest Takes Precedence
     ************************************************************************/
public:
    /** Set all intensity channel values to zero */
    void zeroIntensityChannels();

    /** Check if new $value for $channel & $group pass HTP criteria. */
    bool checkHTP(int channel, uchar value, QLCChannel::Group group) const;

    /************************************************************************
     * Grand Master
     ************************************************************************/
public:
    enum GMValueMode
    {
        GMLimit, /** Limit maximum values to current GM value */
        GMReduce /** Reduce channel values by a fraction (0-100%) */
    };

    enum GMChannelMode
    {
        GMIntensity,  /** GM applied only for Intensity channels */
        GMAllChannels /** GM applied for all channels */
    };

    static GMValueMode stringToGMValueMode(const QString& str);
    static QString gMValueModeToString(GMValueMode mode);

    static GMChannelMode stringToGMChannelMode(const QString& str);
    static QString gMChannelModeToString(GMChannelMode mode);

    /**
     * Set the way how Grand Master should treat its value. @See enum
     * GMValueMode for more info on the modes.
     *
     * @param mode The mode to set
     */
    void setGMValueMode(GMValueMode mode);

    /**
     * Get the Grand Master value mode.
     * @See setGMValueMode() and enum GMValueMode.
     *
     * @return Current value mode
     */
    GMValueMode gMValueMode() const;

    /**
     * Set the way how Grand Master should treat channels. @See enum
     * GrandMasterChannelMode for more info on the modes.
     *
     * @param mode The mode to set
     */
    void setGMChannelMode(GMChannelMode mode);

    /**
     * Get the Grand Master channel mode.
     * @See setGMChannelMode() and enum GMChannelMode.
     *
     * @return Current channel mode
     */
    GMChannelMode gMChannelMode() const;

    /**
     * Set the Grand Master value as a DMX value 0-255. This value is
     * converted to a fraction according to the current mode.
     */
    void setGMValue(uchar value);

    /**
     * Get the current Grand Master value as a DMX value (0 - 255)
     *
     * @return Current Grand Master value in DMX
     */
    uchar gMValue() const;

    /**
     * Get the current Grand Master value as a fraction 0.0 - 1.0
     *
     * @return Current Grand Master value as a fraction
     */
    double gMFraction() const;

    /**
     * Get the current post-Grand-Master values (to be written to output HW)
     * Don't write to the returned array to prevent copying. Not that it would
     * do anything to UniverseArray's internal values, but it would be just
     * pointless waste of CPU time.
     *
     * @return The current values
     */
    const QByteArray* postGMValues() const;

    /**
     * Get the current pre-Grand-Master values (used by functions and everyone
     * else INSIDE QLC). Don't write to the returned array to prevent copying.
     * Not that it would do anything to UniverseArray's internal values, but it
     * would be just pointless waste of CPU time.
     *
     * @return The current values
     */
    const QByteArray preGMValues() const;

protected:
    /**
     * Apply Grand Master to the value.
     *
     * @param channel The channel to apply Grand Master to
     * @param value The value to write
     * @param group The channel's channel group
     * @return Value filtered through grand master (if applicable)
     */
    uchar applyGM(int channel, uchar value, QLCChannel::Group group);

protected:
    GMValueMode m_gMValueMode;
    GMChannelMode m_gMChannelMode;
    uchar m_gMValue;
    double m_gMFraction;
    QSet <int> m_gMIntensityChannels;
    QSet <int> m_gMNonIntensityChannels;
    QByteArray* m_preGMValues;
    QByteArray* m_postGMValues;

    /************************************************************************
     * Writing
     ************************************************************************/
public:
    /**
     * Write a value to a DMX channel, taking Grand Master and HTP into
     * account, if applicable.
     *
     * @param channel The channel number to write to
     * @param value The value to write
     * @param group The channel's channel group
     * @return true if successful, otherwise false
     */
    bool write(int channel, uchar value,
               QLCChannel::Group group = QLCChannel::NoGroup);
};

#endif
