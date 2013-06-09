/*
  Q Light Controller
  fadechannel.h

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

#ifndef FADECHANNEL_H
#define FADECHANNEL_H

#include <QtGlobal>

#include "qlcchannel.h"
#include "fixture.h"
#include "doc.h"

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
    /** Create a new FadeChannel with empty/invalid values */
    FadeChannel();

    /** Copy constructor */
    FadeChannel(const FadeChannel& ch);

    /** Destructor */
    virtual ~FadeChannel();

    /** Comparison operator (true if fixture & channel match) */
    bool operator==(const FadeChannel& fc) const;

    /************************************************************************
     * Values
     ************************************************************************/
public:
    /** Set the Fixture that is being controlled. */
    void setFixture(quint32 id);

    /** Get the Fixture that is being controlled. */
    quint32 fixture() const;

    /** Set channel within the Fixture. */
    void setChannel(quint32 num);

    /** Get channel within the Fixture. */
    quint32 channel() const;

    /** Get the absolute address for this channel. */
    quint32 address(const Doc* doc) const;

    /** Get the channel group. */
    QLCChannel::Group group(const Doc* doc) const;

    /** Set starting value. */
    void setStart(uchar value);

    /** Get starting value. */
    uchar start() const;

    /** Set target value. */
    void setTarget(uchar value);

    /** Get target value. */
    uchar target() const;

    /** Set the current value. */
    void setCurrent(uchar value);

    /** Get the current value. */
    uchar current() const;

    /** Get the current value, modified by $intensity. */
    uchar current(qreal intensity) const;

    /** Mark this channel as ready (useful for writing LTP values only once). */
    void setReady(bool rdy);

    /** Check if this channel is ready. Default is false. */
    bool isReady() const;

    /** Returns if a channel can be faded or not */
    bool canFade(const Doc *doc) const;

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
    quint32 m_fixture;
    quint32 m_channel;

    int m_start;
    int m_target;
    int m_current;
    bool m_ready;

    uint m_fadeTime;
    uint m_elapsed;
};

/**
 * Hash function for FadeChannel. Needs a valid .fixture() and .channel() to work
 * correctly.
 */
uint qHash(const FadeChannel& key);

#endif
