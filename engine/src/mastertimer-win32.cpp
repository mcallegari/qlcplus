/*
  Q Light Controller
  mastertimer-win32.cpp

  Copyright (C) Heikki Junnila

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

// Let's assume we have at least W2K (http://msdn.microsoft.com/en-us/library/Aa383745)
#define _WIN32_WINNT 0x05000000
#define _WIN32_WINDOWS 0x05000000
#define WINVER 0x05000000
#include <Windows.h>
#include <QDebug>

#include "mastertimer-win32.h"
#include "mastertimer.h"
#include "qlcmacros.h"

// Target timer resolution in milliseconds
#define TARGET_RESOLUTION_MS 1

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
