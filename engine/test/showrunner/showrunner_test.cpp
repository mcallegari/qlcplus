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
#undef private
#include "show.h"
#include "track.h"
#include "tempomap.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
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
void ShowRunner_Test::tempoMapRunner()
{
    Show *show = new Show(m_doc);
    m_doc->addFunction(show);

    Chaser *chaser = new Chaser(m_doc);
    chaser->setTempoType(Function::Beats);
    m_doc->addFunction(chaser);

    Track *track = new Track(Function::invalidId(), show);
    show->addTrack(track);

    TempoMap map;
    map.addSection(TempoSection(0, 60000, 127.5, 4, "Song"));
    show->setTempoMap(map);

    // with a tempo map, a Beats tempo item is positioned in ms
    ShowFunction *sf = track->createShowFunction(chaser->id());
    sf->setStartTime(1000);
    sf->setDuration(3000);

    ShowRunner runner(m_doc, show->id());
    QCOMPARE(runner.m_tempoMapActive, true);
    QCOMPARE(runner.m_timeFunctions.count(), 1);
    QCOMPARE(runner.m_beatFunctions.count(), 0);
    QCOMPARE(runner.m_totalRunTime, quint32(4000));

    // the Chaser is started at 1000 ms with the tempo map, from the item start
    int i = 0;
    for (; i < 100 && runner.m_runningQueue.isEmpty(); i++)
        runner.write(m_doc->masterTimer());
    QCOMPARE(runner.m_elapsedTime, quint32(1020));
    QVERIFY(chaser->tempoMapClock().isNull() == false);
    QCOMPARE(chaser->tempoMapClock()->origin, quint32(1000));
    QCOMPARE(chaser->tempoMapClock()->map.count(), 1);

    // and stopped at 4000 ms, not after 4000 beats, which also ends the Show
    for (; i < 300 && runner.m_runningQueue.isEmpty() == false; i++)
        runner.write(m_doc->masterTimer());
    QCOMPARE(runner.m_elapsedTime, quint32(4000));

    // started in the middle of the item: the Chaser gets the same origin
    // and the offset into the item
    chaser->stop(FunctionParent::master());
    ShowRunner midRunner(m_doc, show->id(), 2500);
    midRunner.write(m_doc->masterTimer());
    QCOMPARE(midRunner.m_runningQueue.count(), 1);
    QCOMPARE(chaser->tempoMapClock()->origin, quint32(1000));
    QCOMPARE(chaser->elapsed(), quint32(1500));
    midRunner.stop();
}

QTEST_APPLESS_MAIN(ShowRunner_Test)
