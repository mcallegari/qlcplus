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
#include "showrunner.h"
#include "mastertimer.h"
#include "inputoutputmap.h"
#undef private
#include "showfunction.h"
#include "fixture.h"
#include "show.h"
#include "track.h"
#include "scene.h"
#include "doc.h"
#include "showrunner_test.h"

void ShowRunner_Test::initTestCase()
{
    m_doc = new Doc(this);

    m_fixture = new Fixture(m_doc);
    m_fixture->setAddress(0);
    m_fixture->setUniverse(0);
    m_fixture->setChannels(1);
    QVERIFY(m_doc->addFixture(m_fixture));

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

Scene *ShowRunner_Test::createScene()
{
    Scene *scene = new Scene(m_doc);
    scene->setValue(m_fixture->id(), 0, 255);
    m_doc->addFunction(scene);
    return scene;
}

Show *ShowRunner_Test::createShow(quint32 fid1, quint32 start1, quint32 fid2, quint32 start2, bool sameTrack)
{
    Show *show = new Show(m_doc);
    m_doc->addFunction(show);

    Track *t1 = new Track(Function::invalidId(), show);
    show->addTrack(t1);
    Track *t2 = t1;
    if (sameTrack == false)
    {
        t2 = new Track(Function::invalidId(), show);
        show->addTrack(t2);
    }

    ShowFunction *sf1 = t1->createShowFunction(fid1);
    sf1->setStartTime(start1);
    sf1->setDuration(200);
    ShowFunction *sf2 = t2->createShowFunction(fid2);
    sf2->setStartTime(start2);
    sf2->setDuration(200);

    return show;
}

/* Run $show until $until ms and return the number of ticks in which
 * $scene was running, between $from and $to ms */
int ShowRunner_Test::runningTicks(Show *show, Scene *scene, quint32 from, quint32 to, quint32 until)
{
    // ShowRunner starts the Show items on the Doc's own MasterTimer
    MasterTimer *timer = m_doc->masterTimer();
    int ticks = 0;

    show->start(timer, FunctionParent::master());
    for (quint32 ms = 0; ms < until; ms += MasterTimer::tick())
    {
        timer->timerTick();
        if (ms >= from && ms < to && scene->isRunning())
            ticks++;
    }
    show->stop(FunctionParent::master());
    timer->timerTick();
    timer->timerTick();

    return ticks;
}

void ShowRunner_Test::sameFunctionConsecutiveItems()
{
    // one item ends exactly where the next one, of the same Function, begins:
    // the Function must keep running for the second item too
    Scene *scene = createScene();
    Show *show = createShow(scene->id(), 0, scene->id(), 200, true);
    quint32 tick = MasterTimer::tick();

    int expected = int(160 / tick);
    QCOMPARE(runningTicks(show, scene, 220, 380, 380), expected);
    QVERIFY(scene->isRunning() == false);
}

void ShowRunner_Test::sameFunctionOverlappingItems()
{
    // the same Function on two tracks, 0-200 and 100-300: it must keep
    // running until the last of its items ends
    Scene *scene = createScene();
    Show *show = createShow(scene->id(), 0, scene->id(), 100, false);
    quint32 tick = MasterTimer::tick();

    int expected = int(60 / tick);
    QCOMPARE(runningTicks(show, scene, 220, 280, 300), expected);
    QVERIFY(scene->isRunning() == false);
}

void ShowRunner_Test::differentFunctionsConsecutiveItems()
{
    // unchanged behaviour: the first Function stops, the second one runs
    Scene *scene1 = createScene();
    Scene *scene2 = createScene();
    Show *show = createShow(scene1->id(), 0, scene2->id(), 200, true);

    QCOMPARE(runningTicks(show, scene1, 240, 380, 380), 0);
    Show *show2 = createShow(scene1->id(), 0, scene2->id(), 200, true);
    QCOMPARE(runningTicks(show2, scene2, 220, 380, 380), int(160 / MasterTimer::tick()));
}

void ShowRunner_Test::beatItemsWhenStartingMidway()
{
    // at 30 BPM a beat lasts 2000ms, so starting at 10000ms means starting
    // at beat 5 (5000 in "beats as ms"). An item on beats 6-8 must still run,
    // one on beats 0-4 must not
    m_doc->inputOutputMap()->setBeatGeneratorType(InputOutputMap::Internal);
    m_doc->inputOutputMap()->setBpmNumber(30);

    Scene *early = createScene();
    early->setTempoType(Function::Beats);
    Scene *late = createScene();
    late->setTempoType(Function::Beats);

    Show *show = new Show(m_doc);
    m_doc->addFunction(show);
    Track *track = new Track(Function::invalidId(), show);
    show->addTrack(track);
    ShowFunction *sf1 = track->createShowFunction(early->id());
    sf1->setStartTime(0);
    sf1->setDuration(4000);
    ShowFunction *sf2 = track->createShowFunction(late->id());
    sf2->setStartTime(6000);
    sf2->setDuration(2000);

    ShowRunner runner(m_doc, show->id(), 10000);
    QCOMPARE(runner.m_elapsedBeats, quint32(5000));
    QCOMPARE(runner.m_beatFunctions.count(), 1);
    QCOMPARE(runner.m_beatFunctions.first()->functionID(), late->id());

    m_doc->inputOutputMap()->setBeatGeneratorType(InputOutputMap::Disabled);
}

QTEST_GUILESS_MAIN(ShowRunner_Test)
