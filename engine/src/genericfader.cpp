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

GenericFader::GenericFader(Doc* doc)
    : m_intensity(1)
    , m_blendMode(Universe::NormalBlend)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
}

GenericFader::~GenericFader()
{
}

void GenericFader::add(const FadeChannel& ch)
{
    QHash<FadeChannel,FadeChannel>::iterator channelIterator = m_channels.find(ch);
    if (channelIterator != m_channels.end())
    {
        // perform a HTP check
        if (channelIterator.value().current() <= ch.current())
            channelIterator.value() = ch;
    }
    else
    {
        m_channels.insert(ch, ch);
    }
}

void GenericFader::forceAdd(const FadeChannel &ch)
{
    m_channels.insert(ch, ch);
}

void GenericFader::remove(const FadeChannel& ch)
{
    m_channels.remove(ch);
}

void GenericFader::removeAll()
{
    m_channels.clear();
}

const QHash <FadeChannel,FadeChannel>& GenericFader::channels() const
{
    return m_channels;
}

void GenericFader::write(QList<Universe*> ua, bool paused)
{
    QMutableHashIterator <FadeChannel,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        QLCChannel::Group grp = fc.group(m_doc);
        quint32 addr = fc.addressInUniverse();
        quint32 universe = fc.universe();
        bool canFade = fc.canFade(m_doc);
        uchar value;

        // Calculate the next step
        if (paused)
            value = fc.current();
        else
            value = fc.nextStep(MasterTimer::tick());

        // Apply intensity to HTP channels
        if (grp == QLCChannel::Intensity && canFade == true)
            value = fc.current(intensity());

        if (universe != Universe::invalid())
        {
            //qDebug() << "[GenericFader] >>> uni:" << universe << ", address:" << addr << ", value:" << value;
            ua[universe]->writeBlended(addr, value, m_blendMode);
        }

        if (grp == QLCChannel::Intensity && m_blendMode == Universe::NormalBlend)
        {
            // Remove all HTP channels that reach their target _zero_ value.
            // They have no effect either way so removing them saves CPU a bit.
            if (fc.current() == 0 && fc.target() == 0)
            {
                it.remove();
                continue;
            }
        }
/*
        else
        {
            // Remove all LTP channels after their time is up
            if (fc.elapsed() >= fc.fadeTime())
                it.remove();
        }
*/
        if (fc.isFlashing())
            it.remove();
    }
}

void GenericFader::adjustIntensity(qreal fraction)
{
    m_intensity = fraction;
}

qreal GenericFader::intensity() const
{
    return m_intensity;
}

void GenericFader::setBlendMode(Universe::BlendMode mode)
{
    m_blendMode = mode;
}
