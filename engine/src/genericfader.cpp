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

#include <cmath>
#include <QDebug>

#include "genericfader.h"
#include "fadechannel.h"
#include "doc.h"

GenericFader::GenericFader(QObject *parent)
    : QObject(parent)
    , m_priority(Universe::Auto)
    , m_intensity(1.0)
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

int GenericFader::priority() const
{
    return m_priority;
}

void GenericFader::setPriority(int priority)
{
    m_priority = priority;
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

bool GenericFader::deleteRequest()
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
    quint32 hash = channelHash(fc.fixture(), fc.channel());
    QHash<quint32,FadeChannel>::iterator channelIterator = m_channels.find(hash);
    if (channelIterator != m_channels.end())
        return &channelIterator.value();

    fc.setCurrent(universe->preGMValue(fc.address()));

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

    QMutableHashIterator <quint32,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        int channelType = fc.type();
        int address = int(fc.addressInUniverse());
        uchar value;
        Universe::BlendMode blendMode = m_blendMode;

        // Calculate the next step
        if (m_paused)
            value = fc.current();
        else
            value = fc.nextStep(MasterTimer::tick());

        // Apply intensity to channels that can fade
        if (fc.canFade())
        {
            if (channelType & FadeChannel::Intensity)
            {
                value = fc.current(intensity());
            }
            else if (blendMode != Universe::NormalBlend &&
                     fc.fadeTime() == 0 && (channelType & FadeChannel::LTP))
            {
                // this translates into: LTP + crossfade.
                // Value is proportional between start and target, depending on intensity
                value = uchar((qreal(fc.target() - fc.start()) * intensity()) + fc.start());
            }
        }

        // LTP non intensity channels must use normal blending, otherwise they
        // will be added up in case of additive blending
        if ((channelType & FadeChannel::LTP) && (channelType & FadeChannel::Intensity) == 0)
            blendMode = Universe::NormalBlend;

        //qDebug() << "[GenericFader] >>> uni:" << universe->id() << ", address:" << address << ", value:" << value;
        if (channelType & FadeChannel::Override)
            universe->write(address, value, true);
        else if (channelType & FadeChannel::Relative)
            universe->writeRelative(address, value);
        else
            universe->writeBlended(address, value, blendMode);

        if (((channelType & FadeChannel::Intensity) &&
            (channelType & FadeChannel::HTP) &&
            blendMode == Universe::NormalBlend) || m_fadeOut)
        {
            // Remove all channels that reach their target _zero_ value.
            // They have no effect either way so removing them saves a bit of CPU.
            if (fc.current() == 0 && fc.target() == 0 && fc.isReady())
            {
                it.remove();
                continue;
            }
        }
    }

    if (m_fadeOut && channelsCount() == 0)
        requestDelete();
}

qreal GenericFader::intensity() const
{
    return m_intensity;
}

void GenericFader::adjustIntensity(qreal fraction)
{
    m_intensity = fraction;
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

    if (fadeTime)
    {
        QMutableHashIterator <quint32,FadeChannel> it(m_channels);
        while (it.hasNext() == true)
        {
            FadeChannel& fc(it.next().value());

            if ((fc.type() & FadeChannel::Intensity) == 0)
                continue;

            fc.setStart(fc.current());
            fc.setTarget(0);
            fc.setElapsed(0);
            fc.setReady(false);
            fc.setFadeTime(fc.canFade() ? fadeTime : 0);
        }
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
