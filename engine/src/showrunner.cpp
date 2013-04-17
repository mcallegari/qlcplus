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
                m_functions.append(m_doc->function(funcID));
                connect(chaser, SIGNAL(stopped(quint32)), this, SLOT(slotSequenceStopped(quint32)));

                // offline calculation of the show
                quint32 seq_duration = chaser->getDuration();
                m_durations.append(seq_duration);
                if (chaser->getStartTime() + seq_duration > m_totalRunTime)
                    m_totalRunTime = chaser->getStartTime() + seq_duration;
            }
            else if (m_doc->function(funcID)->type() == Function::Audio)
            {
                Audio *audio = qobject_cast<Audio*> (m_doc->function(funcID));
                if (audio == NULL)
                    continue;
                m_functions.append(m_doc->function(funcID));
                connect(audio, SIGNAL(stopped(quint32)), this, SLOT(slotSequenceStopped(quint32)));
                m_durations.append(audio->getDuration());
                if (audio->getStartTime() + audio->getDuration() > m_totalRunTime)
                    m_totalRunTime = audio->getStartTime() + audio->getDuration();
            }
        }
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
    stop();
    qDebug() << "ShowRunner started";
}

void ShowRunner::stop()
{
    m_elapsedTime = 0;
    m_currentStepIndex = 0;
    foreach (quint32 id, m_runningQueue)
        m_doc->function(id)->stop();

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
    if (m_currentStepIndex < m_functions.count())
    {
        quint32 funcStartTime = 0;
        Function *f = m_functions.at(m_currentStepIndex);
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
        if (m_elapsedTime >= funcStartTime)
        {
            f->start(m_doc->masterTimer(), true);
            m_runningQueue.append(f->id());
            m_currentStepIndex++;
        }
    }
    if (m_elapsedTime >= m_totalRunTime)
    {
        Show *show = qobject_cast<Show*>(m_doc->function(m_showID));
        if (show != NULL)
            show->stop();
        emit showFinished();
        return;
    }

    m_elapsedTime += MasterTimer::tick();
    emit timeChanged(m_elapsedTime);
}
