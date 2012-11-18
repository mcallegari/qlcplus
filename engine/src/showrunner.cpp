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
#include "show.h"

#define TIMER_INTERVAL 50

ShowRunner::ShowRunner(const Doc* doc, quint32 showID)
    : QObject(NULL)
    , m_doc(doc)
    , m_showID(showID)
    , m_elapsedTime(0)
    , m_totalRunTime(0)
    , m_currentStepIndex(0)
{
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(m_showID != Show::invalidId());

    Show *show = qobject_cast<Show*>(m_doc->function(m_showID));
    if (show == NULL)
        return;

    foreach(Track *track, show->tracks())
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
            m_chasers.append(chaser);
            connect(chaser, SIGNAL(stopped(quint32)), this, SLOT(slotSequenceStopped(quint32)));

            // offline calculation of the chaser duration
            quint32 seq_duration = 0;
            foreach (ChaserStep step, chaser->steps())
                seq_duration += step.duration;
            m_durations.append(seq_duration);
            if (chaser->getStartTime() + seq_duration > m_totalRunTime)
                m_totalRunTime = chaser->getStartTime() + seq_duration;
            }
        }
    }

    qSort(m_chasers.begin(), m_chasers.end());

    m_runningQueue.clear();

    qDebug() << "ShowRunner created";
}

ShowRunner::~ShowRunner()
{
}

void ShowRunner::start()
{
    stop();
    qDebug() << "ShowRunner started";
}

void ShowRunner::stop()
{
    m_elapsedTime = 0;
    m_currentStepIndex = 0;
    m_runningQueue.clear();
    qDebug() << "ShowRunner stopped";
}

void ShowRunner::slotSequenceStopped(quint32 id)
{
    m_runningQueueMutex.lock();
    for (int i = 0; i < m_runningQueue.count(); i++)
        if (m_runningQueue.at(i) == id)
            m_runningQueue.removeAt(i);
    m_runningQueueMutex.unlock();
}

void ShowRunner::write()
{
    if (m_currentStepIndex < m_chasers.count())
    {
        Chaser *chaser = m_chasers.at(m_currentStepIndex);
        if (m_elapsedTime >= chaser->getStartTime())
        {
            /*
            bool startAsChild = true;
            if (m_currentStepIndex > 0 && m_runningQueue.count() > 0)
                startAsChild = true;
            */
            chaser->start(m_doc->masterTimer(), true);
            m_runningQueue.append(chaser->id());
            m_currentStepIndex++;
        }
    }
    if (m_elapsedTime >= m_totalRunTime)
    {
        //stop();
        emit showFinished();
        return;
    }

    m_elapsedTime += MasterTimer::tick();
    emit timeChanged(m_elapsedTime);
}
