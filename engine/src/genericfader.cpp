/*
  Q Light Controller
  genericfader.cpp

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

#include <cmath>
#include <QDebug>

#include "universearray.h"
#include "genericfader.h"
#include "fadechannel.h"
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
    if (m_channels.contains(ch) == true)
    {
        if (m_channels[ch].current() <= ch.current())
            m_channels[ch] = ch;
    }
    else
    {
        m_channels[ch] = ch;
    }
}

void GenericFader::remove(const FadeChannel& ch)
{
    if (m_channels.contains(ch) == true)
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

void GenericFader::write(UniverseArray* ua)
{
    QMutableHashIterator <FadeChannel,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        QLCChannel::Group grp = fc.group(m_doc);
        quint32 addr = fc.address(m_doc);
        bool canFade = fc.canFade(m_doc);

        // Calculate the next step
        uchar value = fc.nextStep(MasterTimer::tick());

        // Apply intensity to HTP channels
        if (grp == QLCChannel::Intensity && canFade == true)
            value = fc.current(intensity());

        ua->write(addr, value, grp);

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
