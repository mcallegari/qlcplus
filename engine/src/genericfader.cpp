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
#include "universe.h"
#include "doc.h"

GenericFader::GenericFader(Doc* doc)
    : m_intensity(1)
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

void GenericFader::write(QList<Universe*> ua)
{
    QMutableHashIterator <FadeChannel,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        QLCChannel::Group grp = fc.group(m_doc);
        quint32 addr = fc.address();
        quint32 universe = fc.universe();
        bool canFade = fc.canFade(m_doc);

        // Calculate the next step
        uchar value = fc.nextStep(MasterTimer::tick());

        // Apply intensity to HTP channels
        if (grp == QLCChannel::Intensity && canFade == true)
            value = fc.current(intensity());

        if (universe != Universe::invalid())
            ua[universe]->write(addr, value);

        if (grp == QLCChannel::Intensity)
        {
            // Remove all HTP channels that reach their target _zero_ value.
            // They have no effect either way so removing them saves CPU a bit.
            if (fc.current() == 0 && fc.target() == 0)
                remove(fc);
        }
        else
        {
            // Remove all LTP channels after their time is up
            if (fc.elapsed() >= fc.fadeTime())
                remove(fc);
        }
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
