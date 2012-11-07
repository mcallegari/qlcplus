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

#ifndef SCENERUNNER_H
#define SCENERUNNER_H

#include <QObject>
#include <QTimer>

class Chaser;
class Doc;

class SceneRunner : public QObject
{
    Q_OBJECT

public:
    SceneRunner(const Doc* doc, quint32 sceneID);
    ~SceneRunner();

    /** Start the runner */
    void start();

    /** Stop the runner */
    void stop();

private:
    const Doc* m_doc;
    quint32 m_sceneID;
    QList <Chaser*> m_chasers;
    QTimer *m_timer;
    quint32 m_elapsedTime;

private slots:
    void timerTimeout();

signals:
    void timeChanged(quint32 time);

};

#endif
