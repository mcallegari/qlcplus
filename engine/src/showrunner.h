/*
  Q Light Controller
  showrunner.h

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

#ifndef SHOWRUNNER_H
#define SHOWRUNNER_H

#include <QObject>
#include <QMutex>
#include <QMap>

class Function;
class Track;
class Show;
class Doc;

class ShowRunner : public QObject
{
    Q_OBJECT

public:
    ShowRunner(const Doc* doc, quint32 showID, quint32 startTime = 0);
    ~ShowRunner();

    /** Start the runner */
    void start();

    /** Stop the runner */
    void stop();

    void write();

private:
    const Doc* m_doc;

    /** The reference of the show to play */
    Show* m_show;

    /** The list of Functions of the show to play */
    QList <Function *> m_functions;

    /** List of duration of each function */
    QList <quint32> m_durations;

    /** Elapsed time since runner start. Used also to move the cursor in MultiTrackView */
    quint32 m_elapsedTime;

    /** Total time the runner has to run */
    quint32 m_totalRunTime;

    /** List of running Functions and its mutex */
    QList <Function *> m_runningQueue;
    QMutex m_runningQueueMutex;

    /** Current step being played */
    int m_currentFunctionIndex;

private slots:
    void slotSequenceStopped(quint32);

signals:
    void timeChanged(quint32 time);
    void showFinished();

    /************************************************************************
     * Intensity
     ************************************************************************/
public:
    /**
     * Adjust the intensity of show track
     */
    void adjustIntensity(qreal fraction, Track *track);

private:
    QMap<quint32, qreal> m_intensityMap;

};

#endif
