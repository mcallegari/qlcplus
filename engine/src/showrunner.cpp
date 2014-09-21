/*
  Q Light Controller
  showrunner.cpp

  Copyright (c) Massimo Callegari

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

#include <QMutex>
#include <QDebug>

#include "showrunner.h"
#include "chaserstep.h"
#include "function.h"
#include "chaser.h"
#include "track.h"
#include "scene.h"
#include "audio.h"
#include "show.h"
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include "video.h"
#endif

#define TIMER_INTERVAL 50

static bool compareShowFunctions(const ShowFunction *sf1, const ShowFunction *sf2)
{
    if (sf1->startTime() < sf2->startTime())
        return true;
    return false;
}

ShowRunner::ShowRunner(const Doc* doc, quint32 showID, quint32 startTime)
    : QObject(NULL)
    , m_doc(doc)
    , m_elapsedTime(startTime)
    , m_totalRunTime(0)
    , m_currentFunctionIndex(0)
{
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(showID != Show::invalidId());

    m_show = qobject_cast<Show*>(m_doc->function(showID));
    if (m_show == NULL)
        return;

    foreach(Track *track, m_show->tracks())
    {
        // some sanity checks
        if (track == NULL ||
            track->id() == Track::invalidId())
                continue;

        if (track->isMute())
            continue;

        // get all the functions of the track and append them to the runner queue
        foreach(ShowFunction *sfunc, track->showFunctions())
        {
            if (sfunc->startTime() + sfunc->duration() <= startTime)
                continue;
            Function *f = m_doc->function(sfunc->functionID());
            if (f == NULL)
                continue;

            m_functions.append(sfunc);
            connect(f, SIGNAL(stopped(quint32)),
                    this, SLOT(slotFunctionStopped(quint32)));

            if (sfunc->startTime() + sfunc->duration() > m_totalRunTime)
                m_totalRunTime = sfunc->startTime() + sfunc->duration();
        }

        // Initialize the intensity map
        m_intensityMap[track->id()] = 1.0;
    }

    qSort(m_functions.begin(), m_functions.end(), compareShowFunctions);

#if 0
    qDebug() << "Ordered list of ShowFunctions:";
    foreach (ShowFunction *sfunc, m_functions)
        qDebug() << "ID:" << sfunc->functionID() << "st:" << sfunc->startTime() << "dur:" << sfunc->duration();
#endif
    m_runningQueue.clear();

    qDebug() << "ShowRunner created";
}

ShowRunner::~ShowRunner()
{
}

void ShowRunner::start()
{
    //stop();
    qDebug() << "ShowRunner started";
}

void ShowRunner::stop()
{
    m_elapsedTime = 0;
    m_currentFunctionIndex = 0;
    foreach (Function *f, m_runningQueue)
        f->stop();

    m_runningQueue.clear();
    qDebug() << "ShowRunner stopped";
}

void ShowRunner::slotFunctionStopped(quint32 id)
{
    m_runningQueueMutex.lock();
    for (int i = 0; i < m_runningQueue.count(); i++)
        if (m_runningQueue.at(i)->id() == id)
            m_runningQueue.removeAt(i);
    m_runningQueueMutex.unlock();
}

void ShowRunner::write()
{
    //qDebug() << Q_FUNC_INFO << "elapsed:" << m_elapsedTime << ", total:" << m_totalRunTime;
    if (m_currentFunctionIndex < m_functions.count())
    {
        ShowFunction *sf = m_functions.at(m_currentFunctionIndex);
        quint32 funcStartTime = sf->startTime();
        quint32 functionTimeOffset = 0;
        Function *f = m_doc->function(sf->functionID());

        // this should happen only when a Show is not started from 0
        if (m_elapsedTime > funcStartTime)
        {
            functionTimeOffset = m_elapsedTime - funcStartTime;
            funcStartTime = m_elapsedTime;
        }
        if (m_elapsedTime == funcStartTime)
        {
            foreach (Track *track, m_show->tracks())
            {
                if (track->showFunctions().contains(sf))
                {
                    f->adjustAttribute(m_intensityMap[track->id()], Function::Intensity);
                    break;
                }
            }

            f->start(m_doc->masterTimer(), true, functionTimeOffset);
            m_runningQueue.append(f);
            m_currentFunctionIndex++;

            // Add the Function to a map that keeps track of the functions
            // that need to be stopped otherwise they would play endlessly.
            // In this case we assume that it is impossible that 2 Functions
            // with the same ID are running at the same time !
            if (f->type() == Function::EFX || f->type() == Function::RGBMatrix)
                m_stopTimeMap[f->id()] = sf->startTime() + sf->duration();
        }
    }

    // check if we need to stop some "endless" Functions
    m_runningQueueMutex.lock();
    foreach(Function *f, m_runningQueue)
    {
        if (f->type() == Function::EFX || f->type() == Function::RGBMatrix)
        {
            //qDebug() << "elapsed:" << m_elapsedTime << "stopTime:" << m_stopTimeMap[f->id()];
            if (m_elapsedTime == m_stopTimeMap[f->id()])
                f->stop();
        }
    }
    m_runningQueueMutex.unlock();

    // end of the show
    if (m_elapsedTime >= m_totalRunTime)
    {
        if (m_show != NULL)
            m_show->stop();
        emit showFinished();
        return;
    }

    m_elapsedTime += MasterTimer::tick();
    emit timeChanged(m_elapsedTime);
}

/************************************************************************
 * Intensity
 ************************************************************************/

void ShowRunner::adjustIntensity(qreal fraction, Track *track)
{
    if (track == NULL)
        return;

    qDebug() << Q_FUNC_INFO << "Track ID: " << track->id() << ", val:" << fraction;
    m_intensityMap[track->id()] = fraction;

    foreach (ShowFunction *sf, track->showFunctions())
    {
        Function *f = m_doc->function(sf->functionID());

        if (f != NULL && m_runningQueue.contains(f))
            f->adjustAttribute(fraction, Function::Intensity);
    }
}

