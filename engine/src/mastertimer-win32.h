/*
  Q Light Controller Plus
  mastertimer-win32.h

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#ifndef MASTERTIMER_PRIVATE_H
#define MASTERTIMER_PRIVATE_H

#include <Windows.h>

class MasterTimer;

/** @addtogroup engine Engine
 * @{
 */

class MasterTimerPrivate
{
public:
    MasterTimerPrivate(MasterTimer* masterTimer);
    ~MasterTimerPrivate();

    void start();
    void stop();
    bool isRunning() const;

    void timerTick();

private:
    MasterTimer* m_masterTimer;
    UINT m_systemTimerResolution;
    HANDLE m_phTimer;
    bool m_run;
};

/** @} */

#endif
