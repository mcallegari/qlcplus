/*
  Q Light Controller
  functionselection.cpp

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

#include <QDebug>

#include "scenerunner.h"
#include "chaserstep.h"
#include "function.h"
#include "chaser.h"
#include "scene.h"

#define TIMER_INTERVAL 50

SceneRunner::SceneRunner(const Doc* doc, quint32 sceneID)
    : QObject(NULL)
    , m_doc(doc)
    , m_sceneID(sceneID)
    , m_timer(NULL)
    , m_elapsedTime(0)
    , m_currentStepIndex(0)
{
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(sceneID != Scene::invalidId());

    foreach(Function *f, m_doc->functions())
    {
        if (f->type() == Function::Chaser)
        {
            Chaser *chaser = qobject_cast<Chaser*> (f);
            if (chaser->getBoundedSceneID() == sceneID)
            {
                m_chasers.append(chaser);
                /** offline calculation of the chaser duration */
                quint32 seq_duration = 0;
                foreach (ChaserStep step, chaser->steps())
                    seq_duration += step.duration;
                m_durations.append(seq_duration);
            }
        }
    }
    qSort(m_chasers.begin(), m_chasers.end());

    qDebug() << "SceneRunner created";
}

SceneRunner::~SceneRunner()
{
}

void SceneRunner::start()
{
    stop();
    m_timer = new QTimer(this);
    m_timer->setInterval(TIMER_INTERVAL);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    m_timer->start();
    qDebug() << "SceneRunner started";
}

void SceneRunner::stop()
{
    if (m_timer != NULL)
    {
        m_timer->stop();
        delete m_timer;
        m_timer = NULL;
    }
    m_elapsedTime = 0;
    qDebug() << "SceneRunner stopped";
}

void SceneRunner::timerTimeout()
{
    if (m_elapsedTime >= m_chasers.at(m_currentStepIndex)->getStartTime())
    {
        m_chasers.at(m_currentStepIndex)->start(m_doc->masterTimer());
        m_currentStepIndex++;
        if (m_currentStepIndex == m_chasers.count())
            //&& m_elapsedTime >= m_chasers.at(m_currentStepIndex - 1)->getStartTime() + m_durations.at(m_currentStepIndex - 1))
            stop();
    }

    m_elapsedTime += TIMER_INTERVAL;
    emit timeChanged(m_elapsedTime);
}
