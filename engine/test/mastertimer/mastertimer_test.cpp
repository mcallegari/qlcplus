/*
  Q Light Controller
  mastertimer_test.cpp

  Copyright (C) Heikki Junnila

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
#include "mastertimer_test.h"
#include "dmxsource_stub.h"
#include "function_stub.h"
#include "inputoutputmap.h"
#include "mastertimer.h"
#include "qlcchannel.h"
#include "universe.h"
#include "qlcfile.h"
#include "doc.h"
#undef private

#include "linkservice_stub.h"
#include "../common/resource_paths.h"

void MasterTimer_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void MasterTimer_Test::cleanupTestCase()
{
    delete m_doc;
}

void MasterTimer_Test::init()
{
}

void MasterTimer_Test::cleanup()
{
    m_doc->clearContents();
}

void MasterTimer_Test::initial()
{
    MasterTimer* mt = m_doc->masterTimer();

    QVERIFY(mt->runningFunctions() == 0);
    QVERIFY(mt->m_functionList.size() == 0);
    QVERIFY(mt->m_functionListMutex.tryLock() == true);
    mt->m_functionListMutex.unlock();

    QVERIFY(mt->m_dmxSourceList.size() == 0);
    QVERIFY(mt->m_dmxSourceListMutex.tryLock() == true);
    mt->m_dmxSourceListMutex.unlock();

    //QVERIFY(mt->m_running == false);
    QVERIFY(mt->m_stopAllFunctions == false);
}

void MasterTimer_Test::startStop()
{
    MasterTimer* mt = m_doc->masterTimer();

    mt->start();
    QTest::qWait(100);

    QVERIFY(mt->runningFunctions() == 0);
    QVERIFY(mt->m_functionList.size() == 0);
    QVERIFY(mt->m_dmxSourceList.size() == 0);
    // QVERIFY(mt->m_running == true);
    QVERIFY(mt->m_stopAllFunctions == false);

    mt->stop();
    QTest::qWait(100);

    QVERIFY(mt->runningFunctions() == 0);
    QVERIFY(mt->m_functionList.size() == 0);
    QVERIFY(mt->m_dmxSourceList.size() == 0);
    // QVERIFY(mt->m_running == false);
    QVERIFY(mt->m_stopAllFunctions == false);
}

void MasterTimer_Test::startStopFunction()
{
    MasterTimer* mt = m_doc->masterTimer();
    mt->start();

    Function_Stub fs(m_doc);

    QVERIFY(mt->runningFunctions() == 0);

    mt->startFunction(NULL);
    QVERIFY(mt->runningFunctions() == 0);

    mt->startFunction(&fs);
    mt->timerTick();
    QVERIFY(mt->runningFunctions() == 1);

    mt->startFunction(&fs);
    QVERIFY(mt->runningFunctions() == 1);

    QTest::qWait(100);
    fs.stop(FunctionParent::master());
    QTest::qWait(100);

    QVERIFY(mt->runningFunctions() == 0);
}

void MasterTimer_Test::registerUnregisterDMXSource()
{
    MasterTimer* mt = m_doc->masterTimer();
    QVERIFY(mt->m_dmxSourceList.size() == 0);

    DMXSource_Stub s1;
    /* Normal registration */
    mt->registerDMXSource(&s1);
    QVERIFY(mt->m_dmxSourceList.size() == 1);
    QVERIFY(mt->m_dmxSourceList.at(0) == &s1);

    /* No double additions */
    mt->registerDMXSource(&s1);
    QVERIFY(mt->m_dmxSourceList.size() == 1);
    QVERIFY(mt->m_dmxSourceList.at(0) == &s1);

    DMXSource_Stub s2;
    /* Normal registration of another source */
    mt->registerDMXSource(&s2);
    QVERIFY(mt->m_dmxSourceList.size() == 2);
    QVERIFY(mt->m_dmxSourceList.at(0) == &s1);
    QVERIFY(mt->m_dmxSourceList.at(1) == &s2);

    /* No double additions */
    mt->registerDMXSource(&s2);
    QVERIFY(mt->m_dmxSourceList.size() == 2);
    QVERIFY(mt->m_dmxSourceList.at(0) == &s1);
    QVERIFY(mt->m_dmxSourceList.at(1) == &s2);

    /* No double additions */
    mt->registerDMXSource(&s1);
    QVERIFY(mt->m_dmxSourceList.size() == 2);
    QVERIFY(mt->m_dmxSourceList.at(0) == &s1);
    QVERIFY(mt->m_dmxSourceList.at(1) == &s2);

    /* Removal of a source */
    mt->unregisterDMXSource(&s1);
    QVERIFY(mt->m_dmxSourceList.size() == 1);
    QVERIFY(mt->m_dmxSourceList.at(0) == &s2);

    /* No double removals */
    mt->unregisterDMXSource(&s1);
    QVERIFY(mt->m_dmxSourceList.size() == 1);
    QVERIFY(mt->m_dmxSourceList.at(0) == &s2);

    /* Removal of the last source */
    mt->unregisterDMXSource(&s2);
    QVERIFY(mt->m_dmxSourceList.size() == 0);
}

void MasterTimer_Test::interval()
{
    MasterTimer* mt = m_doc->masterTimer();
    Function_Stub fs(m_doc);
    DMXSource_Stub dss;

    mt->start();
    QTest::qWait(100);

    fs.start(mt, FunctionParent::master());
    mt->timerTick();
    QVERIFY(mt->runningFunctions() == 1);

    mt->registerDMXSource(&dss);
    QVERIFY(mt->m_dmxSourceList.size() == 1);

    /* Wait for one second */
    QTest::qWait(1000);

#ifndef SKIP_TEST
    /* It's not guaranteed that context switch happens exactly after 50
       cycles, so we just have to estimate here... */
    QVERIFY(fs.m_writeCalls >= 49 && fs.m_writeCalls <= 51);
    QVERIFY(dss.m_writeCalls >= 49 && dss.m_writeCalls <= 51);
#endif

    fs.stop(FunctionParent::master());
    QTest::qWait(1000);
    QVERIFY(mt->runningFunctions() == 0);

    mt->unregisterDMXSource(&dss);
    QVERIFY(mt->m_dmxSourceList.size() == 0);
}

void MasterTimer_Test::functionInitiatedStop()
{
    MasterTimer* mt = m_doc->masterTimer();
    Function_Stub fs(m_doc);

    mt->start();

    fs.start(mt, FunctionParent::master());
    mt->timerTick();
    QVERIFY(mt->runningFunctions() == 1);

    /* Wait a while so that the function starts running */
    QTest::qWait(100);

    /* Stop the function after it has been running for a while */
    fs.stop(FunctionParent::master());

    /* Wait a while so that the function stops */
    QTest::qWait(100);

    /* Verify that the function is really stopped and the correct
       pre&post handlers have been called. */
    QVERIFY(mt->runningFunctions() == 0);
    QVERIFY(fs.m_preRunCalls == 1);
    QVERIFY(fs.m_writeCalls > 0);
    QVERIFY(fs.m_postRunCalls == 1);
}

void MasterTimer_Test::runMultipleFunctions()
{
    MasterTimer* mt = m_doc->masterTimer();
    mt->start();

    Function_Stub fs1(m_doc);
    fs1.start(mt, FunctionParent::master());
    mt->timerTick();
    QVERIFY(mt->runningFunctions() == 1);

    Function_Stub fs2(m_doc);
    fs2.start(mt, FunctionParent::master());
    mt->timerTick();
    QVERIFY(mt->runningFunctions() == 2);

    Function_Stub fs3(m_doc);
    fs3.start(mt, FunctionParent::master());
    mt->timerTick();
    QVERIFY(mt->runningFunctions() == 3);

    /* Wait a while so that the functions start running */
    QTest::qWait(100);

    /* Stop the functions after they have been running for a while */
    fs1.stop(FunctionParent::master());
    fs2.stop(FunctionParent::master());
    fs3.stop(FunctionParent::master());

    /* Wait a while so that the functions stop */
    QTest::qWait(100);

    QVERIFY(mt->runningFunctions() == 0);
}

void MasterTimer_Test::stopAllFunctions()
{
    MasterTimer* mt = m_doc->masterTimer();
    mt->start();

    Function_Stub fs1(m_doc);
    fs1.start(mt, FunctionParent::master());

    DMXSource_Stub s1;
    mt->registerDMXSource(&s1);

    Function_Stub fs2(m_doc);
    fs2.start(mt, FunctionParent::master());

    DMXSource_Stub s2;
    mt->registerDMXSource(&s2);

    Function_Stub fs3(m_doc);
    fs3.start(mt, FunctionParent::master());

    QTest::qWait(60);

    QVERIFY(mt->runningFunctions() == 3);
    QVERIFY(mt->m_dmxSourceList.size() == 2);

    mt->stopAllFunctions();
    QVERIFY(mt->runningFunctions() == 0);
    QVERIFY(mt->m_dmxSourceList.size() == 2); // Shouldn't stop

    mt->unregisterDMXSource(&s1);
    mt->unregisterDMXSource(&s2);
}

void MasterTimer_Test::stop()
{
    MasterTimer* mt = m_doc->masterTimer();
    mt->start();

    Function_Stub fs1(m_doc);
    fs1.start(mt, FunctionParent::master());

    Function_Stub fs2(m_doc);
    fs2.start(mt, FunctionParent::master());

    Function_Stub fs3(m_doc);
    fs3.start(mt, FunctionParent::master());

    QTest::qWait(60);
    QVERIFY(mt->runningFunctions() == 3);

    mt->stop();
    QTest::qWait(60);
    QVERIFY(mt->runningFunctions() == 0);
    // QVERIFY(mt->m_running == false);
}

void MasterTimer_Test::restart()
{
    MasterTimer* mt = m_doc->masterTimer();
    mt->start();

    Function_Stub fs1(m_doc);
    fs1.start(mt, FunctionParent::master());

    Function_Stub fs2(m_doc);
    fs2.start(mt, FunctionParent::master());

    Function_Stub fs3(m_doc);
    fs3.start(mt, FunctionParent::master());

    QTest::qWait(60);
    QVERIFY(mt->runningFunctions() == 3);

    mt->stop();
    QTest::qWait(60);
    QVERIFY(mt->runningFunctions() == 0);
    QVERIFY(mt->m_functionList.size() == 0);
    QVERIFY(mt->m_functionListMutex.tryLock() == true);
    mt->m_functionListMutex.unlock();
    // QVERIFY(mt->m_running == false);
    QVERIFY(mt->m_stopAllFunctions == false);

    mt->start();
    QVERIFY(mt->runningFunctions() == 0);
    QVERIFY(mt->m_functionList.size() == 0);
    QVERIFY(mt->m_functionListMutex.tryLock() == true);
    mt->m_functionListMutex.unlock();
    // QVERIFY(mt->m_running == true);
    QVERIFY(mt->m_stopAllFunctions == false);

    fs1.start(mt, FunctionParent::master());
    fs2.start(mt, FunctionParent::master());
    fs3.start(mt, FunctionParent::master());
    QTest::qWait(60);
    QVERIFY(mt->runningFunctions() == 3);

    mt->stopAllFunctions();
}

void MasterTimer_Test::beatsPerBar()
{
    MasterTimer* mt = m_doc->masterTimer();

    QCOMPARE(mt->beatsPerBar(), MASTERTIMER_DEFAULT_BEATS_PER_BAR);

    mt->setBeatsPerBar(3);
    QCOMPARE(mt->beatsPerBar(), 3);

    // invalid values are ignored
    mt->setBeatsPerBar(0);
    QCOMPARE(mt->beatsPerBar(), 3);
    mt->setBeatsPerBar(-2);
    QCOMPARE(mt->beatsPerBar(), 3);

    mt->setBeatsPerBar(MASTERTIMER_DEFAULT_BEATS_PER_BAR);
}

void MasterTimer_Test::barPhase()
{
    MasterTimer* mt = m_doc->masterTimer();
    mt->setBeatsPerBar(4);

    mt->m_beatPosition = 0;
    QCOMPARE(mt->barPhase(), 0.0);

    mt->m_beatPosition = 5.5;
    QCOMPARE(mt->barPhase(), 1.5);

    mt->m_beatPosition = 8;
    QCOMPARE(mt->barPhase(), 0.0);

    // a timeline beat can be negative (e.g. a Link session)
    mt->m_beatPosition = -0.5;
    QCOMPARE(mt->barPhase(), 3.5);

    mt->setBeatsPerBar(3);
    mt->m_beatPosition = 7.25;
    QCOMPARE(mt->barPhase(), 1.25);

    mt->setBeatsPerBar(MASTERTIMER_DEFAULT_BEATS_PER_BAR);
    mt->m_beatPosition = 0;
}

void MasterTimer_Test::latencyOffset()
{
    MasterTimer* mt = m_doc->masterTimer();

    // the offset is saved in the machine settings: restore it at the end
    int previous = mt->latencyOffset();

    mt->setLatencyOffset(25);
    QCOMPARE(mt->latencyOffset(), 25);

    mt->setLatencyOffset(-40);
    QCOMPARE(mt->latencyOffset(), -40);

    mt->setLatencyOffset(previous);
    QCOMPARE(mt->latencyOffset(), previous);
}

void MasterTimer_Test::internalBeatPosition()
{
    MasterTimer* mt = m_doc->masterTimer();

    mt->setBeatSourceType(MasterTimer::Internal);
    mt->requestBpmNumber(120);
    QCOMPARE(mt->beatTimeDuration(), 500);

    // halfway through the third beat
    mt->m_beatCount = 2;
    mt->m_lastBeatOffset = 250;
    mt->timerTick();
    QVERIFY(mt->beatPosition() >= 2.5);
    QVERIFY(mt->beatPosition() < 2.6);
    QCOMPARE(mt->m_beatCount, quint64(2));

    // a full beat elapsed: a beat is fired and the count advances
    mt->m_beatTimer.restart();
    mt->m_lastBeatOffset = 500;
    mt->timerTick();
    QCOMPARE(mt->m_beatCount, quint64(3));
    QVERIFY(mt->beatPosition() >= 3.0);
    QVERIFY(mt->beatPosition() < 3.1);

    mt->setBeatSourceType(MasterTimer::None);
}

void MasterTimer_Test::externalBeatPosition()
{
    MasterTimer* mt = m_doc->masterTimer();
    int previousLatency = mt->m_latencyOffset;

    mt->setBeatSourceType(MasterTimer::External);
    mt->m_beatTimeDuration = 500;
    mt->m_latencyOffset = 0;

    // no beat yet
    mt->timerTick();
    QCOMPARE(mt->m_beatCount, quint64(0));

    // an external beat advances the count and restarts the estimate
    mt->requestBeat();
    mt->timerTick();
    QCOMPARE(mt->m_beatCount, quint64(1));
    QVERIFY(mt->beatPosition() >= 1.0);
    QVERIFY(mt->beatPosition() < 1.1);

    // the latency offset moves the estimate ahead...
    mt->m_latencyOffset = 250;
    mt->timerTick();
    QVERIFY(mt->beatPosition() >= 1.5);
    QVERIFY(mt->beatPosition() < 1.6);

    // ...but never past a beat that didn't happen yet
    mt->m_latencyOffset = 1000;
    mt->timerTick();
    QVERIFY(mt->beatPosition() < 2.0);
    QCOMPARE(mt->m_beatCount, quint64(1));

    mt->m_latencyOffset = previousLatency;
    mt->setBeatSourceType(MasterTimer::None);
}

void MasterTimer_Test::beatSourceChangeResetsPosition()
{
    MasterTimer* mt = m_doc->masterTimer();

    mt->setBeatSourceType(MasterTimer::Internal);
    mt->m_beatCount = 7;
    mt->m_beatPosition = 7.5;

    mt->setBeatSourceType(MasterTimer::External);
    QCOMPARE(mt->m_beatCount, quint64(0));
    QCOMPARE(mt->beatPosition(), 0.0);

    mt->setBeatSourceType(MasterTimer::None);
}

/* Run a tick of an Internal beat source, firing a beat if $beat is true */
static void internalTick(MasterTimer *mt, bool beat)
{
    mt->m_beatTimer.restart();
    mt->m_lastBeatOffset = beat ? mt->m_beatTimeDuration : 0;
    mt->timerTick();
}

void MasterTimer_Test::quantizedStartBeat()
{
    MasterTimer* mt = m_doc->masterTimer();
    mt->setBeatSourceType(MasterTimer::Internal);
    mt->requestBpmNumber(120);
    mt->setLaunchQuantize(MasterTimer::QuantizeBeat);

    Function_Stub fs(m_doc);
    mt->requestQuantizedStart(&fs);
    fs.start(mt, FunctionParent::master());

    // no beat: the start is held
    internalTick(mt, false);
    QCOMPARE(mt->runningFunctions(), 0);
    QCOMPARE(mt->m_heldFunctions.count(), 1);
    QCOMPARE(mt->m_quantizeRequests.count(), 0);

    // on the beat, the Function starts
    internalTick(mt, true);
    QCOMPARE(mt->runningFunctions(), 1);
    QCOMPARE(mt->m_heldFunctions.count(), 0);

    fs.stop(FunctionParent::master());
    internalTick(mt, false);
    QCOMPARE(mt->runningFunctions(), 0);

    mt->setLaunchQuantize(MasterTimer::QuantizeOff);
    mt->setBeatSourceType(MasterTimer::None);
}

void MasterTimer_Test::quantizedStartBar()
{
    MasterTimer* mt = m_doc->masterTimer();
    mt->setBeatSourceType(MasterTimer::Internal);
    mt->requestBpmNumber(120);
    mt->setBeatsPerBar(4);
    mt->setLaunchQuantize(MasterTimer::QuantizeBar);

    Function_Stub fs(m_doc);
    mt->requestQuantizedStart(&fs);
    fs.start(mt, FunctionParent::master());

    // a beat in the middle of the bar (beat 3 -> 4): still held
    mt->m_beatCount = 2;
    internalTick(mt, true);
    QCOMPARE(mt->runningFunctions(), 0);
    QCOMPARE(mt->m_heldFunctions.count(), 1);

    // the first beat of the next bar: started
    mt->m_beatCount = 3;
    internalTick(mt, true);
    QCOMPARE(mt->runningFunctions(), 1);
    QCOMPARE(mt->m_heldFunctions.count(), 0);

    fs.stop(FunctionParent::master());
    internalTick(mt, false);

    mt->setLaunchQuantize(MasterTimer::QuantizeOff);
    mt->setBeatSourceType(MasterTimer::None);
}

void MasterTimer_Test::quantizedStartStoppedWhileHeld()
{
    MasterTimer* mt = m_doc->masterTimer();
    mt->setBeatSourceType(MasterTimer::Internal);
    mt->requestBpmNumber(120);
    mt->setLaunchQuantize(MasterTimer::QuantizeBeat);

    Function_Stub fs(m_doc);
    mt->requestQuantizedStart(&fs);
    fs.start(mt, FunctionParent::master());
    internalTick(mt, false);
    QCOMPARE(mt->m_heldFunctions.count(), 1);

    // stopped before the beat: it never starts
    fs.stop(FunctionParent::master());
    internalTick(mt, true);
    QCOMPARE(mt->runningFunctions(), 0);
    QCOMPARE(mt->m_heldFunctions.count(), 0);

    // stop all, with nothing running: the held start is dropped too
    mt->requestQuantizedStart(&fs);
    fs.start(mt, FunctionParent::master());
    internalTick(mt, false);
    QCOMPARE(mt->m_heldFunctions.count(), 1);
    mt->stopAllFunctions();
    QCOMPARE(mt->m_heldFunctions.count(), 0);
    QVERIFY(fs.stopped() == true);
    internalTick(mt, true);
    QCOMPARE(mt->runningFunctions(), 0);

    mt->setLaunchQuantize(MasterTimer::QuantizeOff);
    mt->setBeatSourceType(MasterTimer::None);
}

void MasterTimer_Test::quantizedStartOff()
{
    MasterTimer* mt = m_doc->masterTimer();
    Function_Stub fs(m_doc);

    // quantize on, but no beat source: immediate start
    mt->setBeatSourceType(MasterTimer::None);
    mt->setLaunchQuantize(MasterTimer::QuantizeBar);
    mt->requestQuantizedStart(&fs);
    fs.start(mt, FunctionParent::master());
    mt->timerTick();
    QCOMPARE(mt->runningFunctions(), 1);
    fs.stop(FunctionParent::master());
    mt->timerTick();
    QCOMPARE(mt->runningFunctions(), 0);

    // beat source, but quantize off: immediate start
    mt->setBeatSourceType(MasterTimer::Internal);
    mt->setLaunchQuantize(MasterTimer::QuantizeOff);
    mt->requestQuantizedStart(&fs);
    fs.start(mt, FunctionParent::master());
    internalTick(mt, false);
    QCOMPARE(mt->runningFunctions(), 1);
    fs.stop(FunctionParent::master());
    internalTick(mt, false);
    QCOMPARE(mt->runningFunctions(), 0);

    // a start without a request is never held
    mt->setLaunchQuantize(MasterTimer::QuantizeBar);
    fs.start(mt, FunctionParent::master());
    internalTick(mt, false);
    QCOMPARE(mt->runningFunctions(), 1);
    fs.stop(FunctionParent::master());
    internalTick(mt, false);

    mt->setLaunchQuantize(MasterTimer::QuantizeOff);
    mt->setBeatSourceType(MasterTimer::None);
}

void MasterTimer_Test::timelineBeats()
{
    MasterTimer* mt = m_doc->masterTimer();
    InputOutputMap* iom = m_doc->inputOutputMap();
    LinkService_Stub link;
    int previousLatency = mt->m_latencyOffset;

    // Link is the timeline source of InputOutputMap
    iom->setLinkService(&link);
    iom->setBeatGeneratorType(InputOutputMap::Link);
    QVERIFY(link.m_enabled == true);
    QVERIFY(mt->beatSourceType() == MasterTimer::Timeline);
    mt->m_latencyOffset = 30;

    QSignalSpy beatSpy(mt, SIGNAL(beat()));

    // first tick: position and tempo are taken, no beat is fired
    link.m_timeline.beat = 3.5;
    link.m_timeline.tempo = 128;
    mt->timerTick();
    QCOMPARE(mt->beatPosition(), 3.5);
    QCOMPARE(mt->bpmNumber(), 128);
    QCOMPARE(link.m_lastOffset, 30);
    QCOMPARE(beatSpy.count(), 0);

    // the next whole beat fires a beat
    link.m_timeline.beat = 4.0;
    mt->timerTick();
    QCOMPARE(beatSpy.count(), 1);
    QCOMPARE(mt->barPhase(), 0.0);

    // a backward jump of the timeline doesn't
    link.m_timeline.beat = 2.0;
    mt->timerTick();
    QCOMPARE(beatSpy.count(), 1);

    // the tempo is proposed to the session, not set in MasterTimer
    iom->setBpmNumber(140);
    QCOMPARE(link.m_proposedTempo, 140.0);
    QCOMPARE(mt->bpmNumber(), 128);

    mt->m_latencyOffset = previousLatency;
    iom->setBeatGeneratorType(InputOutputMap::Disabled);
    QVERIFY(link.m_enabled == false);
    QVERIFY(mt->beatSourceType() == MasterTimer::None);
    iom->m_link = NULL;
}

void MasterTimer_Test::timelineTransport()
{
    MasterTimer* mt = m_doc->masterTimer();
    InputOutputMap* iom = m_doc->inputOutputMap();
    LinkService_Stub link;

    iom->setLinkService(&link);

    // without Link as the beat source, the transport is not touched
    iom->setTransportPlaying(true);
    QCOMPARE(link.m_playingCalls, 0);

    iom->setBeatGeneratorType(InputOutputMap::Link);

    QSignalSpy playSpy(iom, SIGNAL(transportChanged(bool)));

    // joining a playing session: the state is taken, not notified
    link.m_timeline.playing = true;
    mt->timerTick();
    QCOMPARE(playSpy.count(), 0);

    // the session is already playing: nothing to commit
    iom->setTransportPlaying(true);
    QCOMPARE(link.m_playingCalls, 0);

    // the session stops
    link.m_timeline.playing = false;
    mt->timerTick();
    QCOMPARE(playSpy.count(), 1);
    QCOMPARE(playSpy.at(0).at(0).toBool(), false);

    // QLC+ starts the session
    iom->setTransportPlaying(true);
    QCOMPARE(link.m_playingCalls, 1);
    QVERIFY(link.m_lastPlaying == true);

    // the session starts
    link.m_timeline.playing = true;
    mt->timerTick();
    QCOMPARE(playSpy.count(), 2);
    QCOMPARE(playSpy.at(1).at(0).toBool(), true);

    iom->setBeatGeneratorType(InputOutputMap::Disabled);
    iom->m_link = NULL;
}

QTEST_MAIN(MasterTimer_Test)
