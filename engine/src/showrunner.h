/*
  Q Light Controller
  showrunner.h

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

#ifndef SHOWRUNNER_H
#define SHOWRUNNER_H

#include <QObject>
#include <QMutex>
#include <QMap>

#include <function.h>

class ShowFunction;
class Function;
class Track;
class Show;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

class ShowRunner : public QObject
{
    Q_OBJECT

public:
    ShowRunner(const Doc* doc, quint32 showID, quint32 startTime = 0);
    ~ShowRunner();

    /** Start the runner */
    void start();

    /** If running, pauses the runner and all the current running functions. */
    void setPause(bool enable);

    /** Stop the runner */
    void stop();

    void write();

private:
    const Doc* m_doc;

    /** The reference of the show to play */
    Show* m_show;

    /** The list of Functions of the show to play */
    QList <ShowFunction *> m_functions;

    /** Elapsed time since runner start. Used also to move the cursor in MultiTrackView */
    quint32 m_elapsedTime;

    /** Total time the runner has to run */
    quint32 m_totalRunTime;

    /** List of the currently running Functions and their stop time */
    QList < QPair<Function *, quint32> > m_runningQueue;

    /** Index of the item in m_functions to be considered for playback */
    int m_currentFunctionIndex;

private:
    FunctionParent functionParent() const;

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

/** @} */

#endif
