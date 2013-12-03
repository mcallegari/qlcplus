/*
  Q Light Controller
  dmxsource.h

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

#ifndef DMXSUBMASTER_H
#define DMXSUBMASTER_H

class MasterTimer;
class UniverseArray;
class ChannelsGroup;
class Doc;

class DMXSubmaster
{
public:
    DMXSubmaster(Doc *doc, quint32 channelGroup);
    virtual ~DMXSubmaster();

    /**
     * @brief perform Perform submaster duty for the given universearray
     * @param timer The timer sending the request
     * @param universes The universes array, on which to apply the submaster
     */
    void perform(MasterTimer* timer, UniverseArray* universes) const;

    void setValue(uchar value);
    uchar value() const;

    quint32 channelGroup() const;

private:
    Doc* m_doc;

    quint32 m_channelGroup;
    uchar m_value;
};

#endif // DMXSUBMASTER_H
