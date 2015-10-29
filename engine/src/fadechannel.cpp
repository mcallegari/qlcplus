/*
  Q Light Controller
  fadechannel.cpp

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

#include <QDebug>
#include <cmath>

#include "fadechannel.h"
#include "qlcchannel.h"
#include "qlcmacros.h"
#include "universe.h"
#include "fixture.h"

FadeChannel::FadeChannel()
    : m_fixture(Fixture::invalidId())
    , m_universe(Universe::invalid())
    , m_channel(QLCChannel::invalid())
    , m_address(QLCChannel::invalid())
    , m_group(QLCChannel::NoGroup)
    , m_start(0)
    , m_target(0)
    , m_current(0)
    , m_ready(false)
    , m_flashing(false)
    , m_fadeTime(0)
    , m_elapsed(0)
{
}

FadeChannel::FadeChannel(const FadeChannel& ch)
    : m_fixture(ch.m_fixture)
    , m_universe(ch.m_universe)
    , m_channel(ch.m_channel)
    , m_address(ch.m_address)
    , m_group(ch.m_group)
    , m_start(ch.m_start)
    , m_target(ch.m_target)
    , m_current(ch.m_current)
    , m_ready(ch.m_ready)
    , m_flashing(ch.m_flashing)
    , m_fadeTime(ch.m_fadeTime)
    , m_elapsed(ch.m_elapsed)
{
    //qDebug() << Q_FUNC_INFO;
}

FadeChannel::FadeChannel(const Doc *doc, quint32 fxi, quint32 channel)
    : m_fixture(fxi)
    , m_channel(channel)
    , m_group(QLCChannel::NoGroup)
    , m_start(0)
    , m_target(0)
    , m_current(0)
    , m_ready(false)
    , m_flashing(false)
    , m_fadeTime(0)
    , m_elapsed(0)
{
    Fixture* fixture = doc->fixture(fxi);
    if (fixture == NULL)
    {
        m_universe = Universe::invalid();
        m_address = QLCChannel::invalid();
    }
    else
    {
        m_universe = fixture->universe();
        m_address = fixture->address();
    }
    // cache the channel group just once,
    // since we (hopefully) won't change the
    // channel properties during the FadeChannel lifetime
    m_group = group(doc);
}

FadeChannel::~FadeChannel()
{
}

bool FadeChannel::operator==(const FadeChannel& ch) const
{
    return (m_fixture == ch.m_fixture && m_channel == ch.m_channel);
}

void FadeChannel::setFixture(const Doc *doc, quint32 id)
{
    m_fixture = id;
    Fixture* fixture = doc->fixture(id);
    if (fixture == NULL)
    {
        m_universe = Universe::invalid();
        m_address = QLCChannel::invalid();
    }
    else
    {
        m_universe = fixture->universe();
        m_address = fixture->address();
    }
}

quint32 FadeChannel::fixture() const
{
    return m_fixture;
}

quint32 FadeChannel::universe()
{
    if (m_universe == Universe::invalid())
        return address() / UNIVERSE_SIZE;
    return m_universe;
}

void FadeChannel::setChannel(const Doc *doc, quint32 num)
{
    m_channel = num;
    // on channel change, invalidate the current
    // cached group and retrieve the correct one
    m_group = QLCChannel::NoGroup;
    m_group = group(doc);
}

quint32 FadeChannel::channel() const
{
    return m_channel;
}

quint32 FadeChannel::address() const
{
    if (m_address == QLCChannel::invalid())
        return channel();

    return (m_address + channel());
}

quint32 FadeChannel::addressInUniverse() const
{
    return address() % UNIVERSE_SIZE;
}

QLCChannel::Group FadeChannel::group(const Doc* doc) const
{
    // if the channel group has been cached, then return it
    // right away, instead of doing a complex lookup every time
    if (m_group != QLCChannel::NoGroup)
        return m_group;

    uint chnum = QLCChannel::invalid();
    Fixture* fxi = NULL;

    if (fixture() == Fixture::invalidId())
    {
        // Do a reverse lookup; which fixture occupies channel()
        // which is now treated as an absolute DMX address.
        quint32 id = doc->fixtureForAddress(channel());
        if (id == Fixture::invalidId())
            return QLCChannel::Intensity;

        fxi = doc->fixture(id);
        if (fxi == NULL)
            return QLCChannel::Intensity;

        // Convert channel() to a relative channel number
        chnum = channel() - fxi->universeAddress();
    }
    else
    {
        // This FadeChannel contains a valid fixture ID
        fxi = doc->fixture(fixture());
        if (fxi == NULL)
            return QLCChannel::Intensity;

        // channel() is already a relative channel number
        chnum = channel();
        // this is a filthy workaround to trick
        // the write() method
        if (fxi->forcedLTPChannels().contains(chnum))
            return QLCChannel::Effect;
        if (fxi->forcedHTPChannels().contains(chnum))
            return QLCChannel::Intensity;
    }

    const QLCChannel* ch = fxi->channel(chnum);
    if (ch == NULL)
        return QLCChannel::Intensity;
    else
        return ch->group();
}

void FadeChannel::setStart(uchar value)
{
    m_start = value;
}

uchar FadeChannel::start() const
{
    return m_start;
}

void FadeChannel::setTarget(uchar value)
{
    m_target = value;
}

uchar FadeChannel::target() const
{
    return m_target;
}

void FadeChannel::setCurrent(uchar value)
{
    m_current = value;
}

uchar FadeChannel::current() const
{
    return m_current;
}

uchar FadeChannel::current(qreal intensity) const
{
    return uchar(floor((qreal(m_current) * intensity) + 0.5));
}

void FadeChannel::setReady(bool rdy)
{
    m_ready = rdy;
}

bool FadeChannel::isReady() const
{
    return m_ready;
}

void FadeChannel::setFlashing(bool flashing)
{
    m_flashing = flashing;
}

bool FadeChannel::isFlashing() const
{
    return m_flashing;
}

bool FadeChannel::canFade(const Doc* doc) const
{
    bool cFade = true;

    if (fixture() != Fixture::invalidId())
    {
        Fixture* fxi = doc->fixture(fixture());
        if (fxi != NULL)
            cFade = fxi->channelCanFade(channel());
    }
    return cFade;
}

void FadeChannel::setFadeTime(uint ms)
{
    m_fadeTime = ms;
}

uint FadeChannel::fadeTime() const
{
    return m_fadeTime;
}

void FadeChannel::setElapsed(uint time)
{
    m_elapsed = time;
}

uint FadeChannel::elapsed() const
{
    return m_elapsed;
}

uchar FadeChannel::nextStep(uint ms)
{
    if (elapsed() < UINT_MAX)
        setElapsed(elapsed() + ms);
    return calculateCurrent(fadeTime(), elapsed());
}

uchar FadeChannel::calculateCurrent(uint fadeTime, uint elapsedTime)
{
    if (elapsedTime >= fadeTime || m_ready == true)
    {
        // Return the target value if all time has been consumed
        // or if the channel has been marked ready.
        m_current = m_target;
    }
    else if (elapsedTime == 0)
    {
        m_current = m_start;
    }
    else
    {
        m_current  = m_target - m_start;
        m_current  = m_current * (qreal(elapsedTime) / qreal(fadeTime));
        m_current += m_start;
    }

    return current();
}

uint qHash(const FadeChannel& key)
{
    uint hash = key.fixture() << 16;
    hash = hash | (key.channel() & 0xFFFF);
    hash = hash & (~0U);
    return hash;
}
