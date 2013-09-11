/*
  Q Light Controller
  showrunner.cpp

  Copyright (c) Massimo Callegari

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

#define TIMER_INTERVAL 50

static bool compareFunctions(const Function *f1, const Function *f2)
{
    if (f1 == NULL || f2 == NULL)
        return false;

    quint32 st1 = 0;
    quint32 st2 = 0;
    if (f1->type() == Function::Audio)
    {
        st1 = (qobject_cast<const Audio*> (f1))->getStartTime();
    }
    else
    {
        st1 = (qobject_cast<const Chaser*> (f1))->getStartTime();
    }

    if (f2->type() == Function::Audio)
    {
        st2 = (qobject_cast<const Audio*> (f2))->getStartTime();
    }
    else
    {
        st2 = (qobject_cast<const Chaser*> (f2))->getStartTime();
    }

    if (st1 < st2)
        return true;
    else
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
            track->id() == Track::invalidId() ||
            track->getSceneID() == Scene::invalidId())
                continue;

        if (track->isMute())
            continue;

        // get all the sequences of the track and append them to the runner queue
        foreach (quint32 funcID, track->functionsID())
        {
            if (m_doc->function(funcID)->type() == Function::Chaser)
            {
                Chaser *chaser = qobject_cast<Chaser*> (m_doc->function(funcID));
                if (chaser == NULL)
                    continue;
                quint32 seq_duration = chaser->getDuration();
                if (chaser->getStartTime() + seq_duration <= startTime)
                    continue;
                m_functions.append(m_doc->function(funcID));
                connect(chaser, SIGNAL(stopped(quint32)), this, SLOT(slotSequenceStopped(quint32)));

                // offline calculation of the show
                m_durations.append(seq_duration);
                if (chaser->getStartTime() + seq_duration > m_totalRunTime)
                    m_totalRunTime = chaser->getStartTime() + seq_duration;
            }
            else if (m_doc->function(funcID)->type() == Function::Audio)
            {
                Audio *audio = qobject_cast<Audio*> (m_doc->function(funcID));
                if (audio == NULL)
                    continue;
                if (audio->getStartTime() + audio->getDuration() <= startTime)
                    continue;
                m_functions.append(m_doc->function(funcID));
                connect(audio, SIGNAL(stopped(quint32)), this, SLOT(slotSequenceStopped(quint32)));
                m_durations.append(audio->getDuration());
                if (audio->getStartTime() + audio->getDuration() > m_totalRunTime)
                    m_totalRunTime = audio->getStartTime() + audio->getDuration();
            }
        }

        // Initialize the intensity map
        m_intensityMap[track->id()] = 1.0;
    }

    qSort(m_functions.begin(), m_functions.end(), compareFunctions);

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

void ShowRunner::slotSequenceStopped(quint32 id)
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
        quint32 funcStartTime = 0;
        quint32 functionTimeOffset = 0;
        Function *f = m_functions.at(m_currentFunctionIndex);
        if (f->type() == Function::Chaser)
        {
            Chaser *chaser = qobject_cast<Chaser*>(f);
            funcStartTime = chaser->getStartTime();
        }
        else if (f->type() == Function::Audio)
        {
            Audio *audio = qobject_cast<Audio*>(f);
            funcStartTime = audio->getStartTime();
        }
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
                if (track->functionsID().contains(f->id()))
                {
                    f->adjustAttribute(m_intensityMap[track->id()]);
                    break;
                }
            }

            f->start(m_doc->masterTimer(), true, functionTimeOffset);
            m_runningQueue.append(f);
            m_currentFunctionIndex++;
        }
    }
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

    QList <quint32> funcIDList = track->functionsID();

    foreach (Function *f, m_runningQueue)
    {
        if (funcIDList.contains(f->id()))
        {
            if (f->type() == Function::Chaser)
            {
                Chaser *chaser = qobject_cast<Chaser*>(f);
                chaser->adjustAttribute(fraction);
            }
            else if (f->type() == Function::Audio)
            {
                Audio *audio = qobject_cast<Audio*>(f);
                audio->adjustAttribute(fraction);
            }
        }
    }
}

