/*
  Q Light Controller Plus - Test Unit
  showrunner_test.cpp

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

#include <QtTest>
#define private public
#define protected public
#include "showrunner.h"
#include "mastertimer.h"
#include "universe.h"
#undef protected
#undef private
#include "show.h"
#include "track.h"
#include "scene.h"
#include "doc.h"
#include "chaser.h"
#include "fixture.h"
#include "showfunction.h"
#include "inputoutputmap.h"
#include "showrunner_test.h"

void ShowRunner_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_show = new Show(m_doc);
    m_doc->addFunction(m_show);
    m_scene = new Scene(m_doc);
    m_doc->addFunction(m_scene);
    m_track = new Track(m_scene->id());
    ShowFunction *sf = new ShowFunction(m_show->getLatestShowFunctionId());
    sf->setFunctionID(m_scene->id());
    sf->setStartTime(0);
    sf->setDuration(1000);
    m_track->addShowFunction(sf);
    m_show->addTrack(m_track);
}

void ShowRunner_Test::cleanupTestCase()
{
    delete m_doc;
}

void ShowRunner_Test::initRunner()
{
    ShowRunner runner(m_doc, m_show->id());
    QCOMPARE(runner.m_timeFunctions.count(), 1);
    QCOMPARE(runner.m_totalRunTime, quint32(1000));
}

void ShowRunner_Test::intensity()
{
    ShowRunner runner(m_doc, m_show->id());
    runner.adjustIntensity(0.5, m_track);
    QCOMPARE(runner.m_intensityMap[m_track->id()], 0.5);
}

void ShowRunner_Test::stopRunner()
{
    ShowRunner runner(m_doc, m_show->id());
    runner.m_elapsedTime = 500;
    runner.m_runningQueue.append(QPair<Function*,quint32>(m_scene,1000));
    runner.stop();
    QCOMPARE(runner.m_elapsedTime, quint32(0));
    QCOMPARE(runner.m_runningQueue.count(), 0);
}

void ShowRunner_Test::writeNullFunction()
{
    // Add a ShowFunction with invalid function ID (will return null from doc->function())
    ShowFunction *sfInvalid = new ShowFunction(m_show->getLatestShowFunctionId());
    sfInvalid->setFunctionID(Function::invalidId());
    sfInvalid->setStartTime(0);
    sfInvalid->setDuration(500);
    m_track->addShowFunction(sfInvalid);

    // This must not crash - the null function should be skipped
    ShowRunner runner(m_doc, m_show->id());
    runner.write(m_doc->masterTimer());

    // Clean up
    m_track->removeShowFunction(sfInvalid);
    delete sfInvalid;
}

void ShowRunner_Test::writeFunctionStartStop()
{
    ShowRunner runner(m_doc, m_show->id());
    QCOMPARE(runner.m_runningQueue.count(), 0);

    // Write at time 0 - function should start
    runner.write(m_doc->masterTimer());
    QCOMPARE(runner.m_runningQueue.count(), 1);
    QCOMPARE(runner.m_runningQueue.first().first, (Function*)m_scene);
}

void ShowRunner_Test::writeTimeFunctions()
{
    // Verify that the existing function is in the time functions list
    ShowRunner runner(m_doc, m_show->id());
    QCOMPARE(runner.m_timeFunctions.count(), 1);
    QCOMPARE(runner.m_timeFunctions.at(0)->functionID(), m_scene->id());
}

void ShowRunner_Test::writeBeatFunctions()
{
    // Beat functions require tempo type to be Beats
    // Just verify the beat function list is empty for time-based show
    ShowRunner runner(m_doc, m_show->id());
    QCOMPARE(runner.m_beatFunctions.count(), 0);
}

void ShowRunner_Test::pauseResume()
{
    ShowRunner runner(m_doc, m_show->id());
    // Start by writing to populate running queue
    runner.write(m_doc->masterTimer());
    QVERIFY(runner.m_runningQueue.count() > 0);

    // Pause should not crash
    runner.setPause(true);
    runner.setPause(false);
}

void ShowRunner_Test::functionStopTime()
{
    ShowRunner runner(m_doc, m_show->id());
    runner.write(m_doc->masterTimer());

    // The running queue entry should have correct stop time
    QCOMPARE(runner.m_runningQueue.count(), 1);
    quint32 expectedStop = m_track->showFunctions().first()->startTime() +
                           m_track->showFunctions().first()->duration(m_doc);
    QCOMPARE(runner.m_runningQueue.first().second, expectedStop);
}

void ShowRunner_Test::totalRunTime()
{
    ShowRunner runner(m_doc, m_show->id());
    // Total run time should be startTime + duration of the last function
    quint32 expected = m_track->showFunctions().first()->startTime() +
                       m_track->showFunctions().first()->duration(m_doc);
    QCOMPARE(runner.m_totalRunTime, expected);
}

void ShowRunner_Test::fadeInFadeOutE2E()
{
    // E2E test: Show -> ShowRunner -> Scene with fade in/out -> DMX output
    // Verifies the complete pipeline: Show.start() -> MasterTimer ticks ->
    // ShowRunner schedules Scene -> Scene creates FadeChannels ->
    // GenericFader interpolates -> Universe DMX output

    // Use Doc's own MasterTimer so ShowRunner sub-functions register on the same timer
    Doc doc(this);
    MasterTimer *timer = doc.masterTimer();
    QList<Universe*> ua;

    Fixture *fxi = new Fixture(&doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(4);
    doc.addFixture(fxi);

    // Scene: ch0=200, ch1=100, fade in = 2 ticks (40ms), fade out = 2 ticks
    Scene *scene = new Scene(&doc);
    scene->setFadeInSpeed(MasterTimer::tick() * 2);   // 40ms
    scene->setFadeOutSpeed(MasterTimer::tick() * 2);   // 40ms
    scene->setValue(fxi->id(), 0, 200);
    scene->setValue(fxi->id(), 1, 100);
    doc.addFunction(scene);

    // Show -> Track -> ShowFunction with 120ms duration (6 ticks)
    Show *show = new Show(&doc);
    doc.addFunction(show);
    Track *track = new Track(scene->id());
    ShowFunction *sf = new ShowFunction(show->getLatestShowFunctionId());
    sf->setFunctionID(scene->id());
    sf->setStartTime(0);
    sf->setDuration(MasterTimer::tick() * 6);  // 120ms
    track->addShowFunction(sf);
    show->addTrack(track);

    // --- Before start: all zeros ---
    timer->timerTick();
    ua = doc.inputOutputMap()->claimUniverses();
    ua[0]->processFaders();
    QCOMPARE((uchar)ua[0]->preGMValues()[0], (uchar)0);
    QCOMPARE((uchar)ua[0]->preGMValues()[1], (uchar)0);
    doc.inputOutputMap()->releaseUniverses(false);

    // --- Start the Show ---
    show->start(timer, FunctionParent::master());

    // Tick 0: Show preRun + first write, Scene preRun + first write (fade-in 50%)
    timer->timerTick();
    ua = doc.inputOutputMap()->claimUniverses();
    ua[0]->processFaders();
    QCOMPARE((uchar)ua[0]->preGMValues()[0], (uchar)100);  // 200 * 50%
    QCOMPARE((uchar)ua[0]->preGMValues()[1], (uchar)50);   // 100 * 50%
    doc.inputOutputMap()->releaseUniverses(false);
    QVERIFY(show->isRunning());
    QVERIFY(scene->isRunning());

    // Tick 1: Fade-in complete (100%)
    timer->timerTick();
    ua = doc.inputOutputMap()->claimUniverses();
    ua[0]->processFaders();
    QCOMPARE((uchar)ua[0]->preGMValues()[0], (uchar)200);
    QCOMPARE((uchar)ua[0]->preGMValues()[1], (uchar)100);
    doc.inputOutputMap()->releaseUniverses(false);

    // Ticks 2-5: Values hold at target
    for (int i = 0; i < 4; i++)
    {
        timer->timerTick();
        ua = doc.inputOutputMap()->claimUniverses();
        ua[0]->processFaders();
        QCOMPARE((uchar)ua[0]->preGMValues()[0], (uchar)200);
        QCOMPARE((uchar)ua[0]->preGMValues()[1], (uchar)100);
        doc.inputOutputMap()->releaseUniverses(false);
    }

    // Tick 6: ShowRunner stops Scene (duration reached), fade-out in progress (50%)
    timer->timerTick();
    ua = doc.inputOutputMap()->claimUniverses();
    ua[0]->processFaders();
    QCOMPARE((uchar)ua[0]->preGMValues()[0], (uchar)100);  // 200 * 50% fade-out
    QCOMPARE((uchar)ua[0]->preGMValues()[1], (uchar)50);   // 100 * 50% fade-out
    doc.inputOutputMap()->releaseUniverses(false);

    // Tick 7: Fade-out complete
    timer->timerTick();
    ua = doc.inputOutputMap()->claimUniverses();
    ua[0]->processFaders();
    QCOMPARE((uchar)ua[0]->preGMValues()[0], (uchar)0);
    QCOMPARE((uchar)ua[0]->preGMValues()[1], (uchar)0);
    doc.inputOutputMap()->releaseUniverses(false);

    // Show should have stopped
    QVERIFY(show->stopped());
}

QTEST_MAIN(ShowRunner_Test)
