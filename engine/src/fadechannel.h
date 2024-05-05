/*
  Q Light Controller
  fadechannel.h

  Copyright (c) Heikki Junnila

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

#ifndef FADECHANNEL_H
#define FADECHANNEL_H

#include <QtGlobal>

#include "doc.h"

/** @addtogroup engine Engine
 * @{
 */

/**
 * FadeChannel represents one fixture channel that is to be faded from $start to
 * $target, with X steps between, determined by $fadeTime. The actual fading process
 * is controlled by GenericFader, but the $current value is calculated each time
 * by FadeChannel.
 */
class FadeChannel
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    enum ChannelFlag
    {
        HTP         = (1 << 0),     /** Highest takes precedence */
        LTP         = (1 << 1),     /** Latest takes precedence */
        Fine        = (1 << 2),     /** LSB channel for 16bit fade */
        Intensity   = (1 << 3),     /** Intensity channel (dimmer, RGB, CMY, etc) */
        CanFade     = (1 << 4),     /** Subject to fade transitions */
        Flashing    = (1 << 5),     /** Is flashing */
        Relative    = (1 << 6),     /** Relative position */
        Override    = (1 << 7),     /** Override the current universe value */
        SetTarget   = (1 << 8),     /** Set target to current universe value */
        AutoRemove  = (1 << 9),     /** Automatically remove the channel once target is reached */
        CrossFade   = (1 << 10),    /** Channel subject to crossfade */
        ForceLTP    = (1 << 11)     /** Force LTP for flashing scenes */
    };

    /** Create a new FadeChannel with empty/invalid values */
    FadeChannel();

    /** Copy constructor */
    FadeChannel(const FadeChannel& ch);

    /** Create a new FadeChannel and set fixture ID and channel */
    FadeChannel(const Doc *doc, quint32 fxi, quint32 channel);

    /** Destructor */
    virtual ~FadeChannel();

    FadeChannel& operator=(const FadeChannel& fc);

    /** Comparison operator (true if fixture & channel match) */
    bool operator==(const FadeChannel& fc) const;

    /** Get/Set the channel flags listed in ChannelFlag */
    int flags() const;
    void setFlags(int flags);

    /** Add/Remove a single flag */
    void addFlag(int flag);
    void removeFlag(int flag);

    /** Get the Fixture that is being controlled. */
    quint32 fixture() const;

    /** Get the universe of the Fixture that is being controlled. */
    quint32 universe() const;

    /** Add another channel to be handled by this fader */
    void addChannel(quint32 num);

    /** Get the number of channels handled by this fader */
    int channelCount() const;

    /** Get the first (or master) channel handled by this fader */
    quint32 channel() const;

    /** Get the index of the provided $channel. This is useful only
     *  when multiple channels are handled and caller doesn't know
     *  if it is targeting primary or secondary */
    int channelIndex(quint32 channel);

    /** Get (if present) the index of the primary channel this fader relate to */
    quint32 primaryChannel() const;

    /** Get the absolute address for this channel. */
    quint32 address() const;

    /** Get the absolute address in its universe for this channel. */
    quint32 addressInUniverse() const;

protected:
    void autoDetect(const Doc *doc);

private:
    /** Bitmask representing all the channel specificities
      * such as fading, overriding, flashing, etc. */
    int m_flags;

    quint32 m_fixture;
    quint32 m_universe;
    quint32 m_primaryChannel;
    QVector<quint32> m_channels;
    quint32 m_address;

    /** Cache channel reference for faster lookup */
    const QLCChannel *m_channelRef;

    /************************************************************************
     * Values
     ************************************************************************/
public:

    /** Set starting value. */
    void setStart(uchar value, int index);
    void setStart(quint32 value);

    /** Get starting value. */
    uchar start(int index) const;
    quint32 start() const;

    /** Set target value. */
    void setTarget(uchar value, int index);
    void setTarget(quint32 value);

    /** Get target value. */
    uchar target(int index) const;
    quint32 target() const;

    /** Set the current value. */
    void setCurrent(uchar value, int index);
    void setCurrent(quint32 value);

    /** Get the current value. */
    uchar current(int index) const;
    quint32 current() const;

    /** Get the current value, modified by $intensity. */
    uchar current(qreal intensity, int index) const;
    quint32 current(qreal intensity) const;

    /** Mark this channel as ready (useful for writing LTP values only once). */
    void setReady(bool rdy);

    /** Check if this channel is ready. Default is false. */
    bool isReady() const;

    /** Returns if a channel can be faded or not */
    bool canFade() const;

    /** Set the fade time in milliseconds. */
    void setFadeTime(uint ms);

    /** Get the fade time in milliseconds. */
    uint fadeTime() const;

    /** Set the elapsed time for this channel. */
    void setElapsed(uint time);

    /** Get the elapsed time for this channel. */
    uint elapsed() const;

    /**
     * Increment elapsed() by $ms milliseconds, calculate the next step and
     * return the new current() value.
     */
    uchar nextStep(uint ms);

    /**
     * Calculate current value based on fadeTime and elapsedTime. Basically:
     * "what m_current should be, if you were given $fadeTime ticks to fade
     * from m_start to m_target when $elapsedTime ticks have already passed."
     *
     * Also, if a channel has been marked ready (isReady() == true), this method
     * returns the target value.
     *
     * @param fadeTime Number of ms to fade from start to target
     * @param elapsedTime Number of ms already spent
     * @return New current value
     */
    uchar calculateCurrent(uint fadeTime, uint elapsedTime);

private:
    quint32 m_start;
    quint32 m_target;
    quint32 m_current;
    bool m_ready;

    uint m_fadeTime;
    uint m_elapsed;
};

/** @} */

#endif
