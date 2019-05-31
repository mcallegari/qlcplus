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
    : m_flags(0)
    , m_fixture(Fixture::invalidId())
    , m_universe(Universe::invalid())
    , m_channel(QLCChannel::invalid())
    , m_address(QLCChannel::invalid())
    , m_start(0)
    , m_target(0)
    , m_current(0)
    , m_ready(false)
    , m_fadeTime(0)
    , m_elapsed(0)
{
}

FadeChannel::FadeChannel(const FadeChannel& ch)
    : m_flags(ch.m_flags)
    , m_fixture(ch.m_fixture)
    , m_universe(ch.m_universe)
    , m_channel(ch.m_channel)
    , m_address(ch.m_address)
    , m_start(ch.m_start)
    , m_target(ch.m_target)
    , m_current(ch.m_current)
    , m_ready(ch.m_ready)
    , m_fadeTime(ch.m_fadeTime)
    , m_elapsed(ch.m_elapsed)
{
    //qDebug() << Q_FUNC_INFO;
}

FadeChannel::FadeChannel(const Doc *doc, quint32 fxi, quint32 channel)
    : m_flags(0)
    , m_fixture(fxi)
    , m_channel(channel)
    , m_start(0)
    , m_target(0)
    , m_current(0)
    , m_ready(false)
    , m_fadeTime(0)
    , m_elapsed(0)
{
    autoDetect(doc);
}

FadeChannel::~FadeChannel()
{
}

bool FadeChannel::operator==(const FadeChannel& ch) const
{
    return (m_fixture == ch.m_fixture && m_channel == ch.m_channel);
}

int FadeChannel::flags() const
{
    return m_flags;
}

void FadeChannel::setFlags(int flags)
{
    m_flags = flags;
}

void FadeChannel::addFlag(int flag)
{
    m_flags |= flag;
}

void FadeChannel::removeFlag(int flag)
{
    m_flags &= (~flag);
}

void FadeChannel::autoDetect(const Doc *doc)
{
    bool fixtureWasInvalid = false;
    // reset before autodetecting
    setFlags(0);

    /* on invalid fixture, channel number is most likely
     * absolute (SimpleDesk/CueStack do it this way), so attempt
     * a reverse lookup to try and find the Fixture ID */
    if (m_fixture == Fixture::invalidId())
    {
        fixtureWasInvalid = true;
        m_fixture = doc->fixtureForAddress(channel());
    }

    Fixture *fixture = doc->fixture(m_fixture);
    if (fixture == NULL)
    {
        m_universe = Universe::invalid();
        m_address = QLCChannel::invalid();
        addFlag(FadeChannel::HTP | FadeChannel::Intensity | FadeChannel::CanFade);
    }
    else
    {
        m_universe = fixture->universe();
        m_address = fixture->address();

        // if the fixture was invalid at the beginning of this method
        // it means channel was an absolute address, so, fix it
        if (fixtureWasInvalid)
            m_channel -= fixture->address();

        const QLCChannel *channel = fixture->channel(m_channel);

        // non existing channel within fixture
        if (channel == NULL)
        {
            addFlag(FadeChannel::HTP | FadeChannel::Intensity | FadeChannel::CanFade);
            return;
        }

        // autodetect the channel type
        if (fixture->channelCanFade(m_channel))
            addFlag(FadeChannel::CanFade);

        if (channel != NULL && channel->group() == QLCChannel::Intensity)
            addFlag(FadeChannel::HTP | FadeChannel::Intensity);
        else
            addFlag(FadeChannel::LTP);

        if (fixture->forcedHTPChannels().contains(m_channel))
        {
            removeFlag(FadeChannel::LTP);
            addFlag(FadeChannel::HTP);
        }
        else if (fixture->forcedLTPChannels().contains(m_channel))
        {
            removeFlag(FadeChannel::HTP);
            addFlag(FadeChannel::LTP);
        }
    }
}

void FadeChannel::setFixture(const Doc *doc, quint32 id)
{
    m_fixture = id;
    autoDetect(doc);
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
    autoDetect(doc);
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

void FadeChannel::setStart(uchar value)
{
    m_start = value;
}

uchar FadeChannel::start() const
{
    return uchar(m_start);
}

void FadeChannel::setTarget(uchar value)
{
    m_target = value;
}

uchar FadeChannel::target() const
{
    return uchar(m_target);
}

void FadeChannel::setCurrent(uchar value)
{
    m_current = value;
}

uchar FadeChannel::current() const
{
    return uchar(m_current);
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

bool FadeChannel::canFade() const
{
    return (m_flags & CanFade) ? true : false;
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
        setReady(true);
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

    return uchar(m_current);
}

