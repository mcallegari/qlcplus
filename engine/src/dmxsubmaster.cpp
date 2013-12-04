/*
  Q Light Controller
  dmxsource.cpp

  Copyright (c) Stefan Riemens

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

#include <qglobal.h> // Q_UNUSED

#include "dmxsubmaster.h"
#include "channelsgroup.h"
#include "doc.h"

DMXSubmaster::DMXSubmaster(Doc* doc, quint32 channelGroup) :
      m_doc(doc)
    , m_channelGroup(channelGroup)
{
    m_doc->masterTimer()->registerDMXSubmaster(this);
}

DMXSubmaster::~DMXSubmaster()
{
    m_doc->masterTimer()->unregisterDMXSubmaster(this);
}

void DMXSubmaster::perform(MasterTimer *timer, UniverseArray *universes) const
{
    Q_UNUSED(timer);

    ChannelsGroup* channelGroup = m_doc->channelsGroup(m_channelGroup);
    quint32 submasterValue = m_value;

    if (channelGroup != 0)
    {
        QByteArray values = universes->preGMValues();

        foreach (SceneValue sv, channelGroup->getChannels())
        {
            Fixture* fixture = m_doc->fixture(sv.fxi);
            if (fixture != 0)
            {
                int channel = fixture->channelAddress(sv.channel);
                QLCChannel::Group group = fixture->channel(sv.channel)->group();
                if (group == QLCChannel::Intensity && (channel >= universes->size()) == false)
                {
                    quint32 currentValue = values.at(channel);
                    quint32 targetValue = quint32((submasterValue / double(UCHAR_MAX)) * currentValue);

                    // NoGroup is LTP, forcing the value down
                    universes->write(channel, targetValue, QLCChannel::NoGroup);

                    // Restore the group attribute, this allows Grand Master to affect it again
                    universes->write(channel, targetValue, group);
                }
            }
        }
    }
}

void DMXSubmaster::setValue(uchar value)
{
    m_value = value;
}

uchar DMXSubmaster::value() const
{
    return m_value;
}

quint32 DMXSubmaster::channelGroup() const
{
    return m_channelGroup;
}
