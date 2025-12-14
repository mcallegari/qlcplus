/*
  Q Light Controller Plus
  mastertimer-unix.h

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

#include <QThread>

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#include <mach/clock.h>
#include <mach/mach.h>
#endif

class MasterTimer;

/** @addtogroup engine Engine
 * @{
 */

class MasterTimerPrivate : public QThread
{
public:
    MasterTimerPrivate(MasterTimer* masterTimer);
    ~MasterTimerPrivate();

    void stop();

private:
    void run();
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    int compareTime(mach_timespec_t *time1, mach_timespec_t *time2);
#else
    int compareTime(struct timespec *time1, struct timespec *time2);
#endif

private:
    bool m_run;
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    clock_serv_t cclock;
#endif
};

/** @} */

#endif
