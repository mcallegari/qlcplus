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

#include "qlcfixturemode.h"
#include "fadechannel.h"
#include "qlcchannel.h"
#include "universe.h"
#include "fixture.h"

FadeChannel::FadeChannel()
    : m_flags(0)
    , m_fixture(Fixture::invalidId())
    , m_universe(Universe::invalid())
    , m_primaryChannel(QLCChannel::invalid())
    , m_address(QLCChannel::invalid())
    , m_channelRef(NULL)
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
    , m_primaryChannel(ch.m_primaryChannel)
    , m_channels(ch.m_channels)
    , m_address(ch.m_address)
    , m_channelRef(ch.m_channelRef)
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
    , m_channelRef(NULL)
    , m_start(0)
    , m_target(0)
    , m_current(0)
    , m_ready(false)
    , m_fadeTime(0)
    , m_elapsed(0)
{
    m_channels.append(channel);
    autoDetect(doc);
}

FadeChannel::~FadeChannel()
{
}

FadeChannel &FadeChannel::operator=(const FadeChannel &fc)
{
    if (this != &fc)
    {
        m_flags = fc.m_flags;
        m_fixture = fc.m_fixture;
        m_universe = fc.m_universe;
        m_primaryChannel = fc.m_primaryChannel;
        m_channels = fc.m_channels;
        m_channelRef = fc.m_channelRef;
        m_address = fc.m_address;
        m_start = fc.m_start;
        m_target = fc.m_target;
        m_current = fc.m_current;
        m_ready = fc.m_ready;
        m_fadeTime = fc.m_fadeTime;
        m_elapsed = fc.m_elapsed;
    }

    return *this;
}

bool FadeChannel::operator==(const FadeChannel& ch) const
{
    return (m_fixture == ch.m_fixture && channel() == ch.channel());
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
        QLCFixtureMode *mode = fixture->fixtureMode();
        m_universe = fixture->universe();
        m_address = fixture->address();

        // if the fixture was invalid at the beginning of this method
        // it means channel was an absolute address, so, fix it
        if (fixtureWasInvalid)
            m_channels[0] -= fixture->address();

        quint32 chIndex = channel();
        m_primaryChannel = mode ? mode->primaryChannel(chIndex) : QLCChannel::invalid();
        m_channelRef = fixture->channel(chIndex);

        // non existing channel within fixture
        if (m_channelRef == NULL)
        {
            addFlag(FadeChannel::HTP | FadeChannel::Intensity | FadeChannel::CanFade);
            return;
        }

        // autodetect the channel type
        if (fixture->channelCanFade(chIndex))
            addFlag(FadeChannel::CanFade);

        if (m_channelRef != NULL && m_channelRef->group() == QLCChannel::Intensity)
            addFlag(FadeChannel::HTP | FadeChannel::Intensity);
        else
            addFlag(FadeChannel::LTP);

        if (fixture->forcedHTPChannels().contains(int(chIndex)))
        {
            removeFlag(FadeChannel::LTP);
            addFlag(FadeChannel::HTP);
        }
        else if (fixture->forcedLTPChannels().contains(int(chIndex)))
        {
            removeFlag(FadeChannel::HTP);
            addFlag(FadeChannel::LTP);
        }
    }
}

quint32 FadeChannel::fixture() const
{
    return m_fixture;
}

quint32 FadeChannel::universe() const
{
    if (m_universe == Universe::invalid())
        return address() / UNIVERSE_SIZE;
    return m_universe;
}

void FadeChannel::addChannel(quint32 num)
{
    m_channels.append(num);
    qDebug() << "[FadeChannel] ADD channel" << num << "count:" << m_channels.count();

    // on secondary channel, shift values 8bits up
    if (m_channels.count() > 1)
    {
        m_start = m_start << 8;
        m_target = m_target << 8;
        m_current = m_current << 8;
    }
}

int FadeChannel::channelCount() const
{
    if (m_channels.isEmpty())
        return 1;

    return m_channels.count();
}

quint32 FadeChannel::channel() const
{
    return m_channels.isEmpty() ? QLCChannel::invalid() : m_channels.first();
}

int FadeChannel::channelIndex(quint32 channel)
{
    int idx = m_channels.indexOf(channel);
    return idx < 0 ? 0 : idx;
}

quint32 FadeChannel::primaryChannel() const
{
    return m_primaryChannel;
}

quint32 FadeChannel::address() const
{
    if (m_address == QLCChannel::invalid())
        return channel();

    return (m_address + channel());
}

quint32 FadeChannel::addressInUniverse() const
{
    quint32 addr = address();
    if (addr == QLCChannel::invalid())
        return QLCChannel::invalid();

    return addr % UNIVERSE_SIZE;
}

/************************************************************************
 * Values
 ************************************************************************/

void FadeChannel::setStart(uchar value, int index)
{
    ((uchar *)&m_start)[channelCount() - 1 - index] = value;
}

void FadeChannel::setStart(quint32 value)
{
    m_start = value;
}

uchar FadeChannel::start(int index) const
{
    return ((uchar *)&m_start)[channelCount() - 1 - index];
}

quint32 FadeChannel::start() const
{
    return m_start;
}

void FadeChannel::setTarget(uchar value, int index)
{
    ((uchar *)&m_target)[channelCount() - 1 - index] = value;
}

void FadeChannel::setTarget(quint32 value)
{
    m_target = value;
}

uchar FadeChannel::target(int index) const
{
    return ((uchar *)&m_target)[channelCount() - 1 - index];
}

quint32 FadeChannel::target() const
{
    return m_target;
}

void FadeChannel::setCurrent(uchar value, int index)
{
    ((uchar *)&m_current)[channelCount() - 1 - index] = value;
}

void FadeChannel::setCurrent(quint32 value)
{
    m_current = value;
}

uchar FadeChannel::current(int index) const
{
    return ((uchar *)&m_current)[channelCount() - 1 - index];
}

quint32 FadeChannel::current() const
{
    return m_current;
}

uchar FadeChannel::current(qreal intensity, int index) const
{
    return uchar(floor((qreal(current(index)) * intensity) + 0.5));
}

quint32 FadeChannel::current(qreal intensity) const
{
    return quint32(floor((qreal(m_current) * intensity) + 0.5));
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
        bool rampUp = m_target > m_start ? true : false;
        m_current = rampUp ? m_target - m_start : m_start - m_target;
        m_current = m_current * (qreal(elapsedTime) / qreal(fadeTime));
        m_current = rampUp ? m_start + m_current : m_start - m_current;
        //qDebug() << "channel" << channel() << "start" << m_start << "target" << m_target << "current" << m_current << "fade" << fadeTime << "elapsed" << elapsedTime ;
    }

    return uchar(m_current);
}

