/*
  Q Light Controller
  mastertimer-unix.cpp

  Copyright (C) Heikki Junnila
                Christopher Staite

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

#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <QDebug>

#include "mastertimer-unix.h"
#include "mastertimer.h"

/****************************************************************************
 * MasterTimerPrivate
 ****************************************************************************/

MasterTimerPrivate::MasterTimerPrivate(MasterTimer* masterTimer)
    : QThread(masterTimer)
    , m_run(false)
{
    Q_ASSERT(masterTimer != NULL);
}

MasterTimerPrivate::~MasterTimerPrivate()
{
    stop();
}

void MasterTimerPrivate::stop()
{
    m_run = false;
    wait();
}

void MasterTimerPrivate::run()
{
    /* Don't start another thread */
    if (m_run == true)
        return;

    MasterTimer* mt = qobject_cast <MasterTimer*> (parent());
    Q_ASSERT(mt != NULL);

    /* How long to wait each loop */
    int tickTime = 1000000 / mt->frequency();

    /* Allocate this from stack here so that GCC doesn't have
       to do it everytime implicitly when gettimeofday() is called */
    int tod = 0;

    /* Allocate all the memory at the start so we don't waste any time */
    timeval* finish = static_cast<timeval*> (malloc(sizeof(timeval)));
    timeval* current = static_cast<timeval*> (malloc(sizeof(timeval)));
    timespec* sleepTime = static_cast<timespec*> (malloc(sizeof(timespec)));
    timespec* remainingTime = static_cast<timespec*> (malloc(sizeof(timespec)));

    sleepTime->tv_sec = 0;

    /* This is the start time for the timer */
    tod = gettimeofday(finish, NULL);
    if (tod == -1)
    {
        qWarning() << Q_FUNC_INFO << "Unable to get the time accurately:"
                   << strerror(errno) << "- Stopping MasterTimerPrivate";
        m_run = false;
    }
    else
    {
        m_run = true;
    }

    while (m_run == true)
    {
        /* Increment the finish time for this loop */
        finish->tv_sec += (finish->tv_usec + tickTime) / 1000000;
        finish->tv_usec = (finish->tv_usec + tickTime) % 1000000;

        tod = gettimeofday(current, NULL);
        if (tod == -1)
        {
            qWarning() << Q_FUNC_INFO << "Unable to get the current time:"
                       << strerror(errno);
            m_run = false;
            break;
        }

        /* Do a rough sleep using the kernel to return control.
           We know that this will never be seconds as we are dealing
           with jumps of under a second every time. */
        sleepTime->tv_nsec = ((finish->tv_usec - current->tv_usec) * 1000) +
                             ((finish->tv_sec - current->tv_sec) * 1000000000) - 1000;
        //qDebug() << Q_FUNC_INFO << sleepTime->tv_nsec;
        if (sleepTime->tv_nsec > 0)
        {
            tod = nanosleep(sleepTime, remainingTime);
            while (tod == -1 && sleepTime->tv_nsec > 100)
            {
                sleepTime->tv_nsec = remainingTime->tv_nsec;
                tod = nanosleep(sleepTime, remainingTime);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Time went forward in the future ! Recalibrating...";
            gettimeofday(finish, NULL);
            continue;
        }

        /* Now take full CPU for precision (only a few nanoseconds,
           at maximum 1000 nanoseconds) */
        sleepTime->tv_nsec = finish->tv_usec - current->tv_usec +
                (finish->tv_sec - current->tv_sec) * 1000000;
        while (sleepTime->tv_nsec > 5)
        {
            //qDebug() << Q_FUNC_INFO << "Loop here" << sleepTime->tv_nsec;
            if (sleepTime->tv_nsec > 1000000)
            {
                qWarning() << Q_FUNC_INFO << "Time went back in the past ! Recalibrating...";
                gettimeofday(finish, NULL);
            }
            tod = gettimeofday(current, NULL);
            if (tod == -1)
            {
                qWarning() << Q_FUNC_INFO << "Unable to get the current time:"
                           << strerror(errno);
                m_run = false;
                break;
            }
            sleepTime->tv_nsec = finish->tv_usec - current->tv_usec +
                    (finish->tv_sec - current->tv_sec) * 1000000;
        }

        /* Execute the next timer event */
        mt->timerTick();
    }

    free(finish);
    free(current);
    free(sleepTime);
    free(remainingTime);
}
