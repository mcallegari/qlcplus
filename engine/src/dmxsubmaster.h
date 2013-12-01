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

class DMXSubmaster
{
public:
    DMXSubmaster();
    virtual ~DMXSubmaster();

    /**
     * @brief perform Perform submaster duty for the given universearray
     * @param timer The timer sending the request
     * @param universes The universes array, on which to apply the submaster
     */
    void perform(MasterTimer* timer, UniverseArray* universes);

private:
    ChannelsGroup* m_channelGroup;
};

#endif // DMXSUBMASTER_H
