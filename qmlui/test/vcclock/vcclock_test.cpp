/*
  Q Light Controller Plus - Test Unit
  vcclock_test.cpp

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

#include "vcclock_test.h"
#include "vctestenv.h"
#include "vcclock.h"
#include "scene.h"

void VCClock_Test::init()
{
    m_env = new VCTestEnv();
}

void VCClock_Test::cleanup()
{
    delete m_env;
}

void VCClock_Test::initial()
{
    VCClock clock(m_env->doc());

    QCOMPARE(clock.type(), int(VCWidget::ClockWidget));
    QCOMPARE(clock.clockType(), VCClock::Clock);
    QCOMPARE(clock.targetTime(), 0);
    QCOMPARE(clock.timerRunning(), false);
    QCOMPARE(clock.enableSchedule(), false);
    QVERIFY(clock.schedules().isEmpty());
    QVERIFY(clock.scheduleList().isEmpty());
    QCOMPARE(clock.propertiesResource(), QString("qrc:/VCClockProperties.qml"));

    /* In clock mode, the current time is the time of the day in seconds */
    QVERIFY(clock.currentTime() >= 0);
    QVERIFY(clock.currentTime() < 24 * 60 * 60);

    clock.setID(1);
    QCOMPARE(clock.caption(), QString("Clock 2"));
}

void VCClock_Test::typeStrings()
{
    VCClock clock(m_env->doc());

    QCOMPARE(clock.typeToString(VCClock::Clock), QString("Clock"));
    QCOMPARE(clock.typeToString(VCClock::Stopwatch), QString("Stopwatch"));
    QCOMPARE(clock.typeToString(VCClock::Countdown), QString("Countdown"));

    QCOMPARE(clock.stringToType("Clock"), VCClock::Clock);
    QCOMPARE(clock.stringToType("Stopwatch"), VCClock::Stopwatch);
    QCOMPARE(clock.stringToType("Countdown"), VCClock::Countdown);
    QCOMPARE(clock.stringToType("Foo"), VCClock::Clock);
}

void VCClock_Test::clockType()
{
    VCClock clock(m_env->doc());

    QSignalSpy spy(&clock, &VCClock::clockTypeChanged);
    clock.setClockType(VCClock::Stopwatch);
    QCOMPARE(clock.clockType(), VCClock::Stopwatch);
    QCOMPARE(clock.currentTime(), 0);
    QCOMPARE(spy.count(), 1);
    clock.setClockType(VCClock::Stopwatch);
    QCOMPARE(spy.count(), 1);

    clock.setClockType(VCClock::Clock);
    QCOMPARE(clock.clockType(), VCClock::Clock);
    QCOMPARE(spy.count(), 2);

    /* Play/pause and reset are meaningless in clock mode */
    clock.playPauseTimer();
    QCOMPARE(clock.timerRunning(), false);
}

void VCClock_Test::stopwatch()
{
    VCClock clock(m_env->doc());
    clock.setClockType(VCClock::Stopwatch);

    QSignalSpy spy(&clock, &VCClock::timerRunningChanged);
    clock.playPauseTimer();
    QCOMPARE(clock.timerRunning(), true);
    QCOMPARE(spy.count(), 1);

    clock.playPauseTimer();
    QCOMPARE(clock.timerRunning(), false);
    QCOMPARE(spy.count(), 2);

    clock.playPauseTimer();
    clock.resetTimer();
    QCOMPARE(clock.timerRunning(), false);
    QCOMPARE(clock.currentTime(), 0);
}

void VCClock_Test::countdown()
{
    VCClock clock(m_env->doc());
    clock.setClockType(VCClock::Countdown);

    /* Nothing to count down */
    clock.playPauseTimer();
    QCOMPARE(clock.timerRunning(), false);

    QSignalSpy spy(&clock, &VCClock::targetTimeChanged);
    clock.setTargetTime(10);
    QCOMPARE(clock.targetTime(), 10);
    QCOMPARE(clock.currentTime(), 10);
    QCOMPARE(spy.count(), 1);
    clock.setTargetTime(10);
    QCOMPARE(spy.count(), 1);

    /* Negative times are clamped */
    clock.setTargetTime(-5);
    QCOMPARE(clock.targetTime(), 0);
    QCOMPARE(clock.currentTime(), 0);

    clock.setTargetTime(20);
    clock.playPauseTimer();
    QCOMPARE(clock.timerRunning(), true);

    /* Changing the target while running doesn't affect the current time */
    clock.setTargetTime(30);
    QCOMPARE(clock.targetTime(), 30);
    QCOMPARE(clock.currentTime(), 20);

    clock.resetTimer();
    QCOMPARE(clock.timerRunning(), false);
    QCOMPARE(clock.currentTime(), 30);

    /* Switching type resets the current time */
    clock.setClockType(VCClock::Stopwatch);
    QCOMPARE(clock.currentTime(), 0);
    clock.setClockType(VCClock::Countdown);
    QCOMPARE(clock.currentTime(), 30);
}

void VCClock_Test::schedules()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCClock clock(m_env->doc());
    QSignalSpy spy(&clock, &VCClock::scheduleListChanged);

    /* Non existing functions are ignored */
    clock.addSchedules(QVariantList() << scene->id() << 12345);
    QCOMPARE(clock.schedules().count(), 1);
    QCOMPARE(clock.scheduleList().count(), 1);
    QCOMPARE(clock.schedules().first()->functionID(), scene->id());
    QCOMPARE(spy.count(), 1);

    /* Schedules without a function are not added */
    VCClockSchedule *invalid = new VCClockSchedule(m_env->doc());
    clock.addSchedule(invalid);
    QCOMPARE(clock.schedules().count(), 1);

    VCClockSchedule *sch = new VCClockSchedule();
    sch->setFunctionID(scene->id());
    sch->setStartTime(3600);
    clock.addSchedule(sch);
    QCOMPARE(clock.schedules().count(), 2);

    /* Changing the week flags notifies the list change */
    int count = spy.count();
    sch->setWeekFlags(3);
    QCOMPARE(sch->weekFlags(), 3);
    QCOMPARE(spy.count(), count + 1);

    clock.removeSchedule(0);
    QCOMPARE(clock.schedules().count(), 1);
    clock.removeSchedule(-1);
    QCOMPARE(clock.schedules().count(), 1);
    clock.removeSchedule(5);
    QCOMPARE(clock.schedules().count(), 1);
    clock.removeSchedule(0);
    QCOMPARE(clock.schedules().count(), 0);

    QSignalSpy enSpy(&clock, &VCClock::enableScheduleChanged);
    clock.setEnableSchedule(true);
    QCOMPARE(clock.enableSchedule(), true);
    QCOMPARE(enSpy.count(), 1);
    clock.setEnableSchedule(true);
    QCOMPARE(enSpy.count(), 1);
}

void VCClock_Test::copy()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCWidget parent(m_env->doc());

    VCClock clock(m_env->doc());
    clock.setCaption("Copied");
    clock.setClockType(VCClock::Countdown);
    clock.setTargetTime(100);
    clock.setEnableSchedule(true);

    VCClockSchedule *sch = new VCClockSchedule();
    sch->setFunctionID(scene->id());
    sch->setStartTime(3600);
    sch->setStopTime(7200);
    sch->setWeekFlags(5);
    clock.addSchedule(sch);

    VCClock *copy = qobject_cast<VCClock *>(clock.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QCOMPARE(copy->caption(), QString("Copied"));
    QCOMPARE(copy->clockType(), VCClock::Countdown);
    QCOMPARE(copy->enableSchedule(), true);

    QCOMPARE(copy->schedules().count(), 1);
    VCClockSchedule *sch2 = copy->schedules().first();
    QVERIFY(sch2 != sch);
    QCOMPARE(sch2->functionID(), scene->id());
    QCOMPARE(sch2->startTime(), 3600);
    QCOMPARE(sch2->stopTime(), 7200);
    QCOMPARE(sch2->weekFlags(), 5);

    QEXPECT_FAIL("", "copyFrom() doesn't copy the countdown target time", Continue);
    QCOMPARE(copy->targetTime(), 100);
}

void VCClock_Test::saveLoad()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCClock clock(m_env->doc());
    clock.setID(9);
    clock.setCaption("Countdown");
    clock.setClockType(VCClock::Countdown);
    clock.setTargetTime(3725); // 01:02:05

    VCClockSchedule *sch = new VCClockSchedule();
    sch->setFunctionID(scene->id());
    sch->setStartTime(3600);
    sch->setStopTime(7200);
    sch->setWeekFlags(5);
    clock.addSchedule(sch);

    QByteArray data = vcSaveXML(&clock);
    QVERIFY(data.contains("Enable=\"False\""));
    QVERIFY(data.contains("Type=\"Countdown\""));
    QVERIFY(data.contains("Time=\"01:02:05\""));

    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("Clock"));

    VCClock clock2(m_env->doc());
    clock2.setEnableSchedule(true);
    QVERIFY(clock2.loadXML(xml.reader()));
    QCOMPARE(clock2.id(), quint32(9));
    QCOMPARE(clock2.caption(), QString("Countdown"));
    QCOMPARE(clock2.enableSchedule(), false);
    QCOMPARE(clock2.clockType(), VCClock::Countdown);
    QCOMPARE(clock2.targetTime(), 3725);
    QCOMPARE(clock2.currentTime(), 3725);

    QCOMPARE(clock2.schedules().count(), 1);
    VCClockSchedule *sch2 = clock2.schedules().first();
    QCOMPARE(sch2->functionID(), scene->id());
    QCOMPARE(sch2->startTime(), 3600);
    QCOMPARE(sch2->stopTime(), 7200);
    QCOMPARE(sch2->weekFlags(), 5);

    /* Wrong tag */
    VCXMLReader wrongXml(QByteArray("<Label/>"));
    VCClock clock3(m_env->doc());
    QCOMPARE(clock3.loadXML(wrongXml.reader()), false);
}

void VCClock_Test::loadLegacyCountdown()
{
    /* Older projects store the countdown time split in hours/minutes/seconds */
    VCXMLReader xml(QByteArray("<Clock Type=\"Countdown\" Hours=\"1\" Minutes=\"2\" Seconds=\"5\"/>"));

    VCClock clock(m_env->doc());
    QVERIFY(clock.loadXML(xml.reader()));
    QCOMPARE(clock.clockType(), VCClock::Countdown);

    /* The target time is in seconds, but the legacy path computes milliseconds */
    QEXPECT_FAIL("", "The legacy countdown time is loaded in milliseconds instead of seconds", Continue);
    QCOMPARE(clock.targetTime(), 3725);
}

QTEST_MAIN(VCClock_Test)
