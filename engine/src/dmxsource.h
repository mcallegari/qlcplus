/*
  Q Light Controller
  dmxsource.h

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

#ifndef DMXSOURCE_H
#define DMXSOURCE_H

class MasterTimer;
class Universe;

/** @addtogroup engine Engine
 * @{
 */

/**
 * DMXSource should be inherited/implemented by such object that wish to
 * write DMX data to QLC's DMX universes. Each DMXSource is polled periodically
 * by MasterTimer for new/changed DMX data.
 */
class DMXSource
{
public:
    DMXSource() { m_changed = false; }
    virtual ~DMXSource() {}

    /**
     * Write the source's current values to the given universe buffer.
     *
     * @param timer The calling MasterTimer instance
     * @param universes Universe buffer to write to
     */
    virtual void writeDMX(MasterTimer* timer, QList<Universe*> universes) = 0;

    /** Get/Set if the DMX source has changed */
    bool hasChanged() { return m_changed; }
    void setChanged(bool changed) { m_changed = changed; }

protected:
    bool m_changed;
};

/** @} */

#endif
