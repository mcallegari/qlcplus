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

            if (sfunc->startTime() + sfunc->duration() > m_totalRunTime)
                m_totalRunTime = sfunc->startTime() + sfunc->duration();
        }

        // Initialize the intensity map
        m_intensityMap[track->id()] = 1.0;
    }

    qSort(m_functions.begin(), m_functions.end(), compareShowFunctions);

#if 1
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
    qDebug() << "ShowRunner started";
}

void ShowRunner::setPause(bool enable)
{
    for (int i = 0; i < m_runningQueue.count(); i++)
    {
        Function *f = m_runningQueue.at(i).first;
        f->setPause(enable);
    }
}

void ShowRunner::stop()
{
    m_elapsedTime = 0;
    m_currentFunctionIndex = 0;
    for (int i = 0; i < m_runningQueue.count(); i++)
    {
        Function *f = m_runningQueue.at(i).first;
        f->stop(functionParent());
    }

    m_runningQueue.clear();
    qDebug() << "ShowRunner stopped";
}

FunctionParent ShowRunner::functionParent() const
{
    return FunctionParent(FunctionParent::Function, m_show->id());
}

void ShowRunner::write()
{
    //qDebug() << Q_FUNC_INFO << "elapsed:" << m_elapsedTime << ", total:" << m_totalRunTime;

    // Phase 1. Check all the Functions that need to be started
    // m_functions is ordered by startup time, so when we found an entry
    // with start time greater than m_elapsed, this phase is over
    bool startupDone = false;

    while(startupDone == false)
    {
        if (m_currentFunctionIndex == m_functions.count())
            break;

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
        if (m_elapsedTime >= funcStartTime)
        {
            foreach (Track *track, m_show->tracks())
            {
                if (track->showFunctions().contains(sf))
                {
                    int intOverrideId = f->requestAttributeOverride(Function::Intensity, m_intensityMap[track->id()]);
                    //f->adjustAttribute(m_intensityMap[track->id()], Function::Intensity);
                    sf->setIntensityOverrideId(intOverrideId);
                    break;
                }
            }

            f->start(m_doc->masterTimer(), functionParent(), functionTimeOffset);
            m_runningQueue.append(QPair<Function *, quint32>(f, sf->startTime() + sf->duration()));
            m_currentFunctionIndex++;
        }
        else
            startupDone = true;
    }

    // Phase 2. Check if we need to stop some running Functions
    // It is done in reverse order for two reasons:
    // 1- m_runningQueue is not ordered by stop time
    // 2- to avoid messing up with indices when an entry is removed
    for(int i = m_runningQueue.count() - 1; i >= 0; i--)
    {
        Function *func = m_runningQueue.at(i).first;
        quint32 stopTime = m_runningQueue.at(i).second;

        // if we passed the function stop time
        if (m_elapsedTime >= stopTime)
        {
            // stop the function
            func->stop(functionParent());
            // remove it from the running queue
            m_runningQueue.removeAt(i);
        }
    }

    // Phase 3. Check if this is the end of the Show
    if (m_elapsedTime >= m_totalRunTime)
    {
        if (m_show != NULL)
            m_show->stop(functionParent());
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
        if (f == NULL)
            continue;

        for (int i = 0; i < m_runningQueue.count(); i++)
        {
            Function *rf = m_runningQueue.at(i).first;
            if (f == rf)
                f->adjustAttribute(fraction, sf->intensityOverrideId());
        }
    }
}

