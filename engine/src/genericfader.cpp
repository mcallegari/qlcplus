/*
  Q Light Controller
  genericfader.cpp

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

#include "genericfader.h"
#include "fadechannel.h"
#include "doc.h"

GenericFader::GenericFader(QObject *parent)
    : QObject(parent)
    , m_fid(Function::invalidId())
    , m_priority(Universe::Auto)
    , m_handleSecondary(false)
    , m_intensity(1.0)
    , m_parentIntensity(1.0)
    , m_paused(false)
    , m_enabled(true)
    , m_fadeOut(false)
    , m_deleteRequest(false)
    , m_blendMode(Universe::NormalBlend)
    , m_monitoring(false)
{
}

GenericFader::~GenericFader()
{
}

QString GenericFader::name() const
{
    return m_name;
}

void GenericFader::setName(QString name)
{
    m_name = name;
}

quint32 GenericFader::parentFunctionID() const
{
    return m_fid;
}

void GenericFader::setParentFunctionID(quint32 fid)
{
    m_fid = fid;
}

int GenericFader::priority() const
{
    return m_priority;
}

void GenericFader::setPriority(int priority)
{
    m_priority = priority;
}

bool GenericFader::handleSecondary()
{
    return m_handleSecondary;
}

void GenericFader::setHandleSecondary(bool enable)
{
    m_handleSecondary = enable;
}

quint32 GenericFader::channelHash(quint32 fixtureID, quint32 channel)
{
    return ((fixtureID & 0x0000FFFF) << 16) | (channel & 0x0000FFFF);
}

void GenericFader::add(const FadeChannel& ch)
{
    quint32 hash = channelHash(ch.fixture(), ch.channel());

    QHash<quint32,FadeChannel>::iterator channelIterator = m_channels.find(hash);
    if (channelIterator != m_channels.end())
    {
        // perform a HTP check
        if (channelIterator.value().current() <= ch.current())
            channelIterator.value() = ch;
    }
    else
    {
        m_channels.insert(hash, ch);
        qDebug() << "Added new fader with hash" << hash;
    }
}

void GenericFader::replace(const FadeChannel &ch)
{
    quint32 hash = channelHash(ch.fixture(), ch.channel());
    m_channels.insert(hash, ch);
}

void GenericFader::remove(FadeChannel *ch)
{
    if (ch == NULL)
        return;

    quint32 hash = channelHash(ch->fixture(), ch->channel());
    if (m_channels.remove(hash) == 0)
        qDebug() << "No FadeChannel found with hash" << hash;
}

void GenericFader::removeAll()
{
    m_channels.clear();
}

bool GenericFader::deleteRequested()
{
    return m_deleteRequest;
}

void GenericFader::requestDelete()
{
    m_deleteRequest = true;
}

FadeChannel *GenericFader::getChannelFader(const Doc *doc, Universe *universe, quint32 fixtureID, quint32 channel)
{
    FadeChannel fc(doc, fixtureID, channel);
    quint32 primary = fc.primaryChannel();
    quint32 hash;

    // calculate hash depending on primary channel presence
    if (handleSecondary() && primary != QLCChannel::invalid())
        hash = channelHash(fc.fixture(), primary);
    else
        hash = channelHash(fc.fixture(), fc.channel());

    // search for existing FadeChannel
    QHash<quint32,FadeChannel>::iterator channelIterator = m_channels.find(hash);
    if (channelIterator != m_channels.end())
    {
        FadeChannel *fcFound = &channelIterator.value();

        if (handleSecondary() &&
            fcFound->channelCount() == 1 &&
            primary != QLCChannel::invalid())
        {
            qDebug() << "Adding channel to primary" << channel;
            fcFound->addChannel(channel);
            if (universe)
                fcFound->setCurrent(universe->preGMValue(fcFound->address() + 1), 1);
        }
        return fcFound;
    }

    // set current universe value
    if (universe)
        fc.setCurrent(universe->preGMValue(fc.address()));

    // new channel. Add to GenericFader
    m_channels[hash] = fc;
    //qDebug() << "Added new fader with hash" << hash;

    return &m_channels[hash];
}

const QHash<quint32, FadeChannel> &GenericFader::channels() const
{
    return m_channels;
}

int GenericFader::channelsCount() const
{
    return m_channels.count();
}

void GenericFader::write(Universe *universe)
{
    if (m_monitoring)
        emit preWriteData(universe->id(), universe->preGMValues());

    qreal compIntensity = intensity() * parentIntensity();

    //qDebug() << "[GenericFader] writing channels: " << this << m_channels.count();

    // iterate through all the channels handled by this fader
    QMutableHashIterator <quint32,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        int flags = fc.flags();
        quint32 address = fc.addressInUniverse();
        int channelCount = fc.channelCount();

        if (address == QLCChannel::invalid())
        {
            qWarning() << "Invalid channel found";
            continue;
        }

        if (flags & FadeChannel::SetTarget)
        {
            fc.removeFlag(FadeChannel::SetTarget);
            fc.addFlag(FadeChannel::AutoRemove);
            for (int i = 0; i < channelCount; i++)
                fc.setTarget(universe->preGMValue(address + i), i);
        }

        // Calculate the next step
        if (m_paused == false)
            fc.nextStep(MasterTimer::tick());

        quint32 value = fc.current();

        // Apply intensity to channels that can fade
        if (fc.canFade())
        {
            if ((flags & FadeChannel::CrossFade) && fc.fadeTime() == 0)
            {
                // morph start <-> target depending on intensities
                bool rampUp = fc.target() > fc.start() ? true : false;
                value = rampUp ? fc.target() - fc.start() : fc.start() - fc.target();
                value = qreal(value) * intensity();
                value = qreal(rampUp ? fc.start() + value : fc.start() - value) * parentIntensity();
            }
            else if (flags & FadeChannel::Intensity)
            {
                value = fc.current(compIntensity);
            }
        }

        //qDebug() << "[GenericFader] >>> uni:" << universe->id() << ", address:" << address << ", value:" << value << "int:" << compIntensity;
        if (flags & FadeChannel::Override)
        {
            universe->write(address, value, true);
            continue;
        }
        else if (flags & FadeChannel::Relative)
        {
            universe->writeRelative(address, value, channelCount);
        }
        else if (flags & FadeChannel::Flashing)
        {
            universe->writeMultiple(address, value, channelCount);
            continue;
        }
        else
        {
            // treat value as a whole, so do this just once per FadeChannel
            universe->writeBlended(address, value, channelCount, m_blendMode);
        }

        if (((flags & FadeChannel::Intensity) &&
            (flags & FadeChannel::HTP) &&
            m_blendMode == Universe::NormalBlend) || m_fadeOut)
        {
            // Remove all channels that reach their target _zero_ value.
            // They have no effect either way so removing them saves a bit of CPU.
            if (fc.current() == 0 && fc.target() == 0 && fc.isReady())
                it.remove();
        }

        if (flags & FadeChannel::AutoRemove && value == fc.target())
            it.remove();
    }

    // self-request deletion when fadeout is complete
    if (m_fadeOut && channelsCount() == 0)
    {
        m_fadeOut = false;
        requestDelete();
    }
}

qreal GenericFader::intensity() const
{
    return m_intensity;
}

void GenericFader::adjustIntensity(qreal fraction)
{
    //qDebug() << name() << "I FADER intensity" << fraction << ", PARENT:" << m_parentIntensity;
    m_intensity = fraction;
}

qreal GenericFader::parentIntensity() const
{
    return m_parentIntensity;
}

void GenericFader::setParentIntensity(qreal fraction)
{
    //qDebug() << name() << "P FADER intensity" << m_intensity << ", PARENT:" << fraction;
    m_parentIntensity = fraction;
}

bool GenericFader::isPaused() const
{
    return m_paused;
}

void GenericFader::setPaused(bool paused)
{
    m_paused = paused;
}

bool GenericFader::isEnabled() const
{
    return m_enabled;
}

void GenericFader::setEnabled(bool enable)
{
    m_enabled = enable;
}

bool GenericFader::isFadingOut() const
{
    return m_fadeOut;
}

void GenericFader::setFadeOut(bool enable, uint fadeTime)
{
    m_fadeOut = enable;

    if (fadeTime == 0)
        return;

    QMutableHashIterator <quint32,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());

        fc.setStart(fc.current());
        // all channels should fade to the current universe value
        fc.addFlag(FadeChannel::SetTarget);
        fc.setTarget(0);
        fc.setElapsed(0);
        fc.setReady(false);
        fc.setFadeTime(fc.canFade() ? fadeTime : 0);
        // if flashing, remove the flag and treat
        // it like a regular fade out to target
        fc.removeFlag(FadeChannel::Flashing);
    }
}

void GenericFader::setBlendMode(Universe::BlendMode mode)
{
    m_blendMode = mode;
}

void GenericFader::setMonitoring(bool enable)
{
    m_monitoring = enable;
}

void GenericFader::resetCrossfade()
{
    qDebug() << name() << "resetting crossfade channels";
    QMutableHashIterator <quint32,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        fc.removeFlag(FadeChannel::CrossFade);
    }
}
