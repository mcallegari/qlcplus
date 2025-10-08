/*
  Q Light Controller Plus
  mastertimer-win32.cpp

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

// Let's assume we have at least W2K (http://msdn.microsoft.com/en-us/library/Aa383745)
#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x05000000
  #define _WIN32_WINDOWS 0x05000000
  #define WINVER 0x05000000
#endif

#include <QDebug>

#include "mastertimer-win32.h"
#include "mastertimer.h"
#include "qlcmacros.h"

/****************************************************************************
 * Timer callback
 ****************************************************************************/
extern "C"
{
    void CALLBACK masterTimerWin32Callback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
    {
        Q_UNUSED(TimerOrWaitFired);

        MasterTimerPrivate* mtp = (MasterTimerPrivate*) lpParameter;
        Q_ASSERT(mtp != NULL);
        mtp->timerTick();
    }
}

/****************************************************************************
 * MasterTimerPrivate
 ****************************************************************************/

MasterTimerPrivate::MasterTimerPrivate(MasterTimer* masterTimer)
    : m_masterTimer(masterTimer)
    , m_systemTimerResolution(0)
    , m_phTimer(NULL)
    , m_run(false)
{
    Q_ASSERT(masterTimer != NULL);
}

MasterTimerPrivate::~MasterTimerPrivate()
{
    stop();
}

void MasterTimerPrivate::start()
{
    if (m_run == true)
        return;

    /* Find out the smallest possible timer tick in milliseconds */
    TIMECAPS ptc;
    MMRESULT result = timeGetDevCaps(&ptc, sizeof(TIMECAPS));
    if (result != TIMERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "Unable to query system timer resolution.";
        return;
    }

    /* Adjust system timer to operate on its minimum tick period */
    m_systemTimerResolution = MIN(MAX(ptc.wPeriodMin, m_masterTimer->tick()), ptc.wPeriodMax);
    result = timeBeginPeriod(m_systemTimerResolution);
    if (result != TIMERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "Unable to adjust system timer resolution.";
        return;
    }

    BOOL ok = CreateTimerQueueTimer(&m_phTimer,
                                    NULL,
                                    (WAITORTIMERCALLBACK) masterTimerWin32Callback,
                                    this,
                                    0,
                                    m_masterTimer->tick(),
                                    WT_EXECUTELONGFUNCTION);
    if (!ok)
    {
        qWarning() << Q_FUNC_INFO << "Unable to create a timer:" << GetLastError();
        timeEndPeriod(m_systemTimerResolution);
        m_systemTimerResolution = 0;
        return;
    }

    m_run = true;
}

void MasterTimerPrivate::stop()
{
    if (m_run == false)
        return;

    // Destroy the timer and wait for it to complete its last firing (if applicable)
    if (DeleteTimerQueueTimer(NULL, m_phTimer, INVALID_HANDLE_VALUE))
        timeEndPeriod(m_systemTimerResolution);

    m_systemTimerResolution = 0;
    m_phTimer = NULL;
    m_run = false;
}

bool MasterTimerPrivate::isRunning() const
{
    return m_run;
}

void MasterTimerPrivate::timerTick()
{
    m_masterTimer->timerTick();
}
