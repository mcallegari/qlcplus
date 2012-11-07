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
{
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(sceneID != Scene::invalidId());

    foreach(Function *f, m_doc->functions())
    {
        if (f->type() == Function::Chaser)
        {
            Chaser *chaser = qobject_cast<Chaser*> (f);
            if (chaser->getBoundedSceneID() == sceneID)
                m_chasers.append(chaser);
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
    m_elapsedTime += TIMER_INTERVAL;
    emit timeChanged(m_elapsedTime);
}
