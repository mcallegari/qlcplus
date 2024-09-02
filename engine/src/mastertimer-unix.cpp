/*
  Q Light Controller Plus
  mastertimer-unix.cpp

  Copyright (C) Heikki Junnila
                Christopher Staite
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
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
#endif
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

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
int MasterTimerPrivate::compareTime(mach_timespec_t *time1, mach_timespec_t *time2)
#else
int MasterTimerPrivate::compareTime(struct timespec *time1, struct timespec *time2)
#endif
{
    if (time1->tv_sec < time2->tv_sec)
    {
        qDebug() << "Time is late by" << (time2->tv_sec - time1->tv_sec) << "seconds";
        return -1;
    }
    else if (time1->tv_sec > time2->tv_sec)
        return 1;
    else if (time1->tv_nsec < time2->tv_nsec)
    {
        qDebug() << "Time is late by" << (time2->tv_nsec - time1->tv_nsec) << "nanoseconds";
        return -1;
    }
    else if (time1->tv_nsec > time2->tv_nsec)
        return 1;
    else
        return 0;
}

void MasterTimerPrivate::run()
{
    /* Don't start another thread */
    if (m_run == true)
        return;

    MasterTimer* mt = qobject_cast <MasterTimer*> (parent());
    Q_ASSERT(mt != NULL);

    /* How long to wait each loop, in nanoseconds */
    int nsTickTime = 1000000000L / mt->frequency();

    /* Allocate this from stack here so that GCC doesn't have
       to do it everytime implicitly when gettimeofday() is called */
    int ret = 0;

    /* Allocate all the memory at the start so we don't waste any time */
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    mach_timespec_t* finish = static_cast<mach_timespec_t*> (malloc(sizeof(mach_timespec_t)));
    mach_timespec_t* current = static_cast<mach_timespec_t*> (malloc(sizeof(mach_timespec_t)));
#else
    struct timespec* finish = static_cast<struct timespec*> (malloc(sizeof(struct timespec)));
    struct timespec* current = static_cast<struct timespec*> (malloc(sizeof(struct timespec)));
#endif
    struct timespec* sleepTime = static_cast<struct timespec*> (malloc(sizeof(struct timespec)));
    struct timespec* remainingTime = static_cast<struct timespec*> (malloc(sizeof(struct timespec)));

    sleepTime->tv_sec = 0;

    /* This is the start time for the timer */
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    ret = clock_get_time(cclock, finish);
#else
    ret = clock_gettime(CLOCK_MONOTONIC, finish);
#endif
    if (ret == -1)
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
        /* Add nsTickTime to the finish time, to calculate the end timestamp of this loop */
        finish->tv_sec += (finish->tv_nsec + nsTickTime) / 1000000000L;
        finish->tv_nsec = (finish->tv_nsec + nsTickTime) % 1000000000L;

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
        ret = clock_get_time(cclock, current);
#else
        ret = clock_gettime(CLOCK_MONOTONIC, current);
#endif
        if (ret == -1)
        {
            qWarning() << Q_FUNC_INFO << "Unable to get the current time:"
                       << strerror(errno);
            m_run = false;
            break;
        }

        /* Check if we're running late. This means that a tick is not enough
         * to process all the running Functions :'( */
        if (compareTime(finish, current) <= 0)
        {
            qDebug() << Q_FUNC_INFO << "MasterTimer is running late!";
            /* No need to sleep. Immediately process the next tick */
            mt->timerTick();
            /* Now the finish time needs to be recalibrated */
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
            clock_get_time(cclock, finish);
#else
            clock_gettime(CLOCK_MONOTONIC, finish);
#endif
            continue;
        }

        /* Do a rough sleep using the kernel to return control.
           We know that this will never be seconds as we are dealing
           with jumps of under a second every time. */
        sleepTime->tv_sec = finish->tv_sec - current->tv_sec;
        if (finish->tv_nsec < current->tv_nsec)
        {
            sleepTime->tv_nsec = finish->tv_nsec + 1000000000L - current->tv_nsec ;
            sleepTime->tv_sec--; /* Decrease a second. */
        }
        else
            sleepTime->tv_nsec = finish->tv_nsec - current->tv_nsec;

        //qDebug() << Q_FUNC_INFO << "Sleeping ns:" << sleepTime->tv_nsec;

        ret = nanosleep(sleepTime, remainingTime);
        while (ret == -1 && sleepTime->tv_nsec > 100)
        {
            sleepTime->tv_nsec = remainingTime->tv_nsec;
            ret = nanosleep(sleepTime, remainingTime);
        }

#if 0
        /* Now take full CPU for precision (only a few nanoseconds,
           at maximum 100 nanoseconds) */
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
        ret = clock_get_time(cclock, current);
#else
        ret = clock_gettime(CLOCK_MONOTONIC, current);
#endif
        sleepTime->tv_nsec = finish->tv_nsec - current->tv_nsec;

        while (sleepTime->tv_nsec > 5)
        {
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
            ret = clock_get_time(cclock, current);
#else
            ret = clock_gettime(CLOCK_MONOTONIC, current);
#endif
            sleepTime->tv_nsec = finish->tv_nsec - current->tv_nsec;
            qDebug() << "Full CPU wait:" << sleepTime->tv_nsec;
        }
#endif
        /* Execute the next timer event */
        mt->timerTick();
    }

    free(finish);
    free(current);
    free(sleepTime);
    free(remainingTime);
}
