/*
  Q Light Controller - Unit test
  chaserrunner_test.cpp

  Copyright (c) Heikki Junnila

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
#include <QMap>

#define private public
#define protected public
#include "chaserrunner_test.h"
#include "mastertimer_stub.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "chaserrunner.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "chaserstep.h"
#include "tempomap.h"
#include "universe.h"
#include "qlcfile.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#undef protected
#undef private

#include "../common/resource_paths.h"

void ChaserRunner_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    m_doc->fixtureDefCache()->loadMap(dir);
}

void ChaserRunner_Test::cleanupTestCase()
{
    delete m_doc;
}

void ChaserRunner_Test::init()
{
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    QVERIFY(fxi != NULL);
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Fixture");
    fxi->setAddress(0);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);

    m_scene1 = new Scene(m_doc);
    m_scene1->setName("S1");
    QVERIFY(m_scene1 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene1->setValue(fxi->id(), i, 255 - i);
    m_doc->addFunction(m_scene1);

    m_scene2 = new Scene(m_doc);
    m_scene2->setName("S2");
    QVERIFY(m_scene2 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene2->setValue(fxi->id(), i, 127 - i);
    m_doc->addFunction(m_scene2);

    m_scene3 = new Scene(m_doc);
    m_scene3->setName("S3");
    QVERIFY(m_scene3 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene3->setValue(fxi->id(), i, 0 + i);
    m_doc->addFunction(m_scene3);

    m_chaser = new Chaser(m_doc);
    m_chaser->addStep(ChaserStep(m_scene1->id()));
    m_chaser->addStep(ChaserStep(m_scene2->id()));
    m_chaser->addStep(ChaserStep(m_scene3->id()));
}

void ChaserRunner_Test::cleanup()
{
    m_doc->clearContents();
}

void ChaserRunner_Test::initial()
{
    ChaserRunner cr(m_doc, m_chaser);
    QCOMPARE(cr.m_doc, m_doc);
    QCOMPARE(cr.m_chaser, m_chaser);

    QCOMPARE(cr.m_updateOverrideSpeeds, false);
    QCOMPARE(cr.m_direction, Function::Forward);
    QCOMPARE(cr.m_startOffset, quint32(0));
    QCOMPARE(cr.m_pendingAction.m_action, ChaserNoAction);
    QCOMPARE(cr.m_pendingAction.m_masterIntensity, 1.0);
    QCOMPARE(cr.m_pendingAction.m_stepIndex, -1);
    QCOMPARE(cr.m_pendingAction.m_fadeMode, (int)Chaser::FromFunction);
    QCOMPARE(cr.m_lastRunStepIdx, -1);
}

void ChaserRunner_Test::nextPrevious()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::SingleShot);

    ChaserRunner cr(m_doc, m_chaser);

    QCOMPARE(cr.m_pendingAction.m_action, ChaserNoAction);

    ChaserAction action;
    action.m_action = ChaserNextStep;

    cr.setAction(action);
    QCOMPARE(cr.m_pendingAction.m_action, ChaserNextStep);

    cr.setAction(action);
    QCOMPARE(cr.m_pendingAction.m_action, ChaserNextStep);

    action.m_action = ChaserPreviousStep;
    cr.setAction(action);
    QCOMPARE(cr.m_pendingAction.m_action, ChaserPreviousStep);

    cr.setAction(action);
    QCOMPARE(cr.m_pendingAction.m_action, ChaserPreviousStep);
}

void ChaserRunner_Test::currentFadeIn()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);

    m_chaser->setFadeInSpeed(100);
    m_chaser->replaceStep(ChaserStep(m_scene1->id(), 1000, 2000, 3000), 0);
    m_chaser->replaceStep(ChaserStep(m_scene2->id(), 1100, 2100, 3100), 1);
    m_chaser->replaceStep(ChaserStep(m_scene3->id(), 1200, 2200, 3200), 2);

    ChaserRunner cr(m_doc, m_chaser);

    m_chaser->setFadeInMode(Chaser::Default);
    QCOMPARE(cr.currentStepIndex(), -1);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), Function::defaultSpeed());
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), Function::defaultSpeed());
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), Function::defaultSpeed());

    m_chaser->setFadeInMode(Chaser::Common);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(100));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(100));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(100));

    m_chaser->setFadeInMode(Chaser::PerStep);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1000));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1100));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1200));
    cr.m_lastRunStepIdx = 3; // Nonexistent step
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), Function::defaultSpeed());

    // Check that override speed really overrides any setting
    m_chaser->setOverrideFadeInSpeed(1234);

    m_chaser->setFadeInMode(Chaser::Default);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));

    m_chaser->setFadeInMode(Chaser::Common);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));

    m_chaser->setFadeInMode(Chaser::PerStep);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 3; // Nonexistent step
    QCOMPARE(cr.stepFadeIn(cr.currentStepIndex()), uint(1234));
}

void ChaserRunner_Test::currentFadeOut()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);

    m_chaser->setFadeOutSpeed(200);
    m_chaser->replaceStep(ChaserStep(m_scene1->id(), 1000, 2000, 3000), 0);
    m_chaser->replaceStep(ChaserStep(m_scene2->id(), 1100, 2100, 3100), 1);
    m_chaser->replaceStep(ChaserStep(m_scene3->id(), 1200, 2200, 3200), 2);

    ChaserRunner cr(m_doc, m_chaser);

    m_chaser->setFadeOutMode(Chaser::Default);
    QCOMPARE(cr.currentStepIndex(), -1);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), Function::defaultSpeed());
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), Function::defaultSpeed());
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), Function::defaultSpeed());

    m_chaser->setFadeOutMode(Chaser::Common);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(200));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(200));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(200));

    m_chaser->setFadeOutMode(Chaser::PerStep);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(3000));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(3100));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(3200));
    cr.m_lastRunStepIdx = 3; // Nonexistent step
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), Function::defaultSpeed());

    // Check that override speed really overrides any setting
    m_chaser->setOverrideFadeOutSpeed(1234);

    m_chaser->setFadeOutMode(Chaser::Default);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));

    m_chaser->setFadeOutMode(Chaser::Common);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));

    m_chaser->setFadeOutMode(Chaser::PerStep);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 3; // Nonexistent step
    QCOMPARE(cr.stepFadeOut(cr.currentStepIndex()), uint(1234));
}

void ChaserRunner_Test::currentDuration()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);

    m_chaser->setDuration(300);
    m_chaser->replaceStep(ChaserStep(m_scene1->id(), 1000, 2000, 3000), 0);
    m_chaser->replaceStep(ChaserStep(m_scene2->id(), 1100, 2100, 3100), 1);
    m_chaser->replaceStep(ChaserStep(m_scene3->id(), 1200, 2200, 3200), 2);

    ChaserRunner cr(m_doc, m_chaser);

    // Default mode for duration is interpreted as Common
    m_chaser->setDurationMode(Chaser::Default);
    QCOMPARE(cr.currentStepIndex(), -1);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(300));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(300));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(300));

    m_chaser->setDurationMode(Chaser::Common);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(300));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(300));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(300));

    m_chaser->setDurationMode(Chaser::PerStep);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(3000));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(3200));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(3400));
    cr.m_lastRunStepIdx = 3; // Nonexistent step
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(300)); // Fall back to common speed

    // Check that override speed really overrides any setting
    m_chaser->setOverrideDuration(1234);

    m_chaser->setDurationMode(Chaser::Default);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));

    m_chaser->setDurationMode(Chaser::Common);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));

    m_chaser->setDurationMode(Chaser::PerStep);
    cr.m_lastRunStepIdx = 0;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 1;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 2;
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));
    cr.m_lastRunStepIdx = 3; // Nonexistent step
    QCOMPARE(cr.stepDuration(cr.currentStepIndex()), uint(1234));
}

/*
void ChaserRunner_Test::roundCheckSingleShotForward()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::SingleShot);
    m_chaser->setDuration(Function::infiniteSpeed());
    ChaserRunner cr(m_doc, m_chaser);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 3);
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = 4; // Over list.size
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = -1; // Under list.size
    QVERIFY(cr.roundCheck() == false);

    cr.reset();
    QCOMPARE(cr.currentStep(), 0);
}

void ChaserRunner_Test::roundCheckSingleShotBackward()
{
    m_chaser->setDirection(Function::Backward);
    m_chaser->setRunOrder(Function::SingleShot);
    m_chaser->setDuration(Function::infiniteSpeed());
    ChaserRunner cr(m_doc, m_chaser);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 3); // Over list.size
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = -1; // Under list.size
    QVERIFY(cr.roundCheck() == false);

    cr.reset();
    QCOMPARE(cr.currentStep(), 2);
}

void ChaserRunner_Test::roundCheckLoopForward()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);
    m_chaser->setDuration(Function::infiniteSpeed());
    ChaserRunner cr(m_doc, m_chaser);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);

    QCOMPARE(cr.currentStep(), 1);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    // Loops around back to index 0
    QCOMPARE(cr.currentStep(), 3);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    // Loops around to index 2
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    cr.reset();
    QCOMPARE(cr.currentStep(), 0);
}

void ChaserRunner_Test::roundCheckLoopBackward()
{
    m_chaser->setDirection(Function::Backward);
    m_chaser->setRunOrder(Function::Loop);
    m_chaser->setDuration(Function::infiniteSpeed());
    ChaserRunner cr(m_doc, m_chaser);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);

    QCOMPARE(cr.currentStep(), 1);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    // Loops around back to index 2
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    // Loops around to index 0
    QCOMPARE(cr.currentStep(), 3);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    cr.reset();
    QCOMPARE(cr.currentStep(), 2);
}

void ChaserRunner_Test::roundCheckPingPongForward()
{
    m_chaser->addStep(m_scene1->id()); // Easier to check direction changes with 4 steps
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::PingPong);
    m_chaser->setDuration(Function::infiniteSpeed());
    ChaserRunner cr(m_doc, m_chaser);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_direction, Function::Forward);

    QCOMPARE(cr.currentStep(), 1);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    QCOMPARE(cr.currentStep(), 3);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 3);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 4;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    QCOMPARE(cr.currentStep(), 1);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    QCOMPARE(cr.currentStep(), 3);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 3);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 4;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.reset();
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Forward);
}

void ChaserRunner_Test::roundCheckPingPongBackward()
{
    m_chaser->addStep(m_scene1->id()); // Easier to check direction changes with 4 steps
    m_chaser->setDirection(Function::Backward);
    m_chaser->setRunOrder(Function::PingPong);
    m_chaser->setDuration(Function::infiniteSpeed());
    ChaserRunner cr(m_doc, m_chaser);

    QCOMPARE(cr.currentStep(), 3);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_direction, Function::Backward);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    QCOMPARE(cr.currentStep(), 1);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    QCOMPARE(cr.currentStep(), 3);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 3);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 4;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    QCOMPARE(cr.currentStep(), 1);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.reset();
    QCOMPARE(cr.currentStep(), 3);
    QCOMPARE(cr.m_direction, Function::Backward);
}
*/

void ChaserRunner_Test::writeNoSteps()
{
    Chaser chaser(m_doc);
    ChaserRunner cr(m_doc, &chaser);

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    MasterTimerStub timer(m_doc, ua);

    QVERIFY(cr.write(&timer, ua) == false);
}

void ChaserRunner_Test::writeForwardLoopZero()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);
}

void ChaserRunner_Test::writeBackwardLoopZero()
{
    m_chaser->setDirection(Function::Backward);
    m_chaser->setRunOrder(Function::Loop);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);
}

void ChaserRunner_Test::writeForwardSingleShotZero()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::SingleShot);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == false);
    cr.postRun(&timer, QList<Universe*>());
    timer.timerTick();
    QVERIFY(m_scene3->stopped() == true);
    QCOMPARE(timer.m_functionList.size(), 0);
}

void ChaserRunner_Test::writeBackwardSingleShotZero()
{
    m_chaser->setDirection(Function::Backward);
    m_chaser->setRunOrder(Function::SingleShot);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == false);
    cr.postRun(&timer, QList<Universe*>());
    timer.timerTick();
    QVERIFY(m_scene1->stopped() == true);
    QCOMPARE(timer.m_functionList.size(), 0);
}

void ChaserRunner_Test::writeForwardPingPongZero()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::PingPong);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QVERIFY(m_scene1->stopped() == true);
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QVERIFY(m_scene1->stopped() == true);
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);
}

void ChaserRunner_Test::writeBackwardPingPongZero()
{
    m_chaser->setDirection(Function::Backward);
    m_chaser->setRunOrder(Function::PingPong);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);
}

void ChaserRunner_Test::writeForwardLoopFive()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);

    uint dur = MasterTimer::tick() * 5;
    m_chaser->setDuration(dur);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }
}

void ChaserRunner_Test::writeBackwardLoopFive()
{
    m_chaser->setDirection(Function::Backward);
    m_chaser->setRunOrder(Function::Loop);

    uint dur = MasterTimer::tick() * 5;
    m_chaser->setDuration(dur);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }
}

void ChaserRunner_Test::writeForwardSingleShotFive()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::SingleShot);

    uint dur = MasterTimer::tick() * 5;
    m_chaser->setDuration(dur);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    QVERIFY(cr.write(&timer, QList<Universe*>()) == false);
    timer.timerTick();

    cr.postRun(&timer, QList<Universe*>());
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 0);
}

void ChaserRunner_Test::writeBackwardSingleShotFive()
{
    m_chaser->setDirection(Function::Backward);
    m_chaser->setRunOrder(Function::SingleShot);

    uint dur = MasterTimer::tick() * 5;
    m_chaser->setDuration(dur);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    QVERIFY(cr.write(&timer, QList<Universe*>()) == false);
    timer.timerTick();

    cr.postRun(&timer, QList<Universe*>());
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 0);
}

void ChaserRunner_Test::writeForwardPingPongFive()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::PingPong);

    uint dur = MasterTimer::tick() * 5;
    m_chaser->setDuration(dur);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }
}

void ChaserRunner_Test::writeBackwardPingPongFive()
{
    m_chaser->setDirection(Function::Backward);
    m_chaser->setRunOrder(Function::PingPong);

    uint dur = MasterTimer::tick() * 5;
    m_chaser->setDuration(dur);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }
}

void ChaserRunner_Test::writeNoAutoStep()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);

    m_chaser->setDuration(Function::infiniteSpeed());

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    ChaserAction action;
    action.m_action = ChaserNextStep;
    cr.setAction(action);

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    cr.setAction(action);

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    cr.setAction(action);

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    action.m_action = ChaserPreviousStep;
    cr.setAction(action);

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    cr.setAction(action);

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    cr.setAction(action);

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }
}

void ChaserRunner_Test::adjustIntensity()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    cr.adjustStepIntensity(0.5);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);
    QCOMPARE(m_scene1->getAttributeValue(Scene::ParentIntensity), qreal(0.5));
    QCOMPARE(m_scene2->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene3->getAttributeValue(Function::Intensity), qreal(1.0));

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);
    QCOMPARE(m_scene1->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene2->getAttributeValue(Scene::ParentIntensity), qreal(0.5));
    QCOMPARE(m_scene3->getAttributeValue(Function::Intensity), qreal(1.0));

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);
    QCOMPARE(m_scene1->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene2->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene3->getAttributeValue(Scene::ParentIntensity), qreal(0.5));

    cr.adjustStepIntensity(0.7);
    QCOMPARE(m_scene1->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene2->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene3->getAttributeValue(Scene::ParentIntensity), qreal(0.7));

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);
    QCOMPARE(m_scene1->getAttributeValue(Scene::ParentIntensity), qreal(0.7));
    QCOMPARE(m_scene2->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene3->getAttributeValue(Function::Intensity), qreal(1.0));

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);
    QCOMPARE(m_scene1->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene2->getAttributeValue(Scene::ParentIntensity), qreal(0.7));
    QCOMPARE(m_scene3->getAttributeValue(Function::Intensity), qreal(1.0));

    cr.adjustStepIntensity(1.5);
    QCOMPARE(m_scene1->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene2->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene3->getAttributeValue(Function::Intensity), qreal(1.0));

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);
    QCOMPARE(m_scene1->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene2->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(m_scene3->getAttributeValue(Function::Intensity), qreal(1.0));
}

void ChaserRunner_Test::adjustMasterIntensityAcrossRunningCrossfadeSteps()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);
    m_chaser->setDuration(Function::infiniteSpeed());

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer *timer = m_doc->masterTimer();

    cr.adjustStepIntensity(0.75, 0, Chaser::BlendedCrossfade);
    timer->timerTick();
    cr.adjustStepIntensity(0.25, 1, Chaser::BlendedCrossfade);
    timer->timerTick();

    QCOMPARE(cr.runningStepsNumber(), 2);
    QCOMPARE(cr.m_runnerSteps.size(), 2);
    QCOMPARE(cr.m_runnerSteps.at(0)->m_index, 0);
    QCOMPARE(cr.m_runnerSteps.at(1)->m_index, 1);
    QCOMPARE(cr.m_runnerSteps.at(0)->m_masterIntensity, qreal(1.0));
    QCOMPARE(cr.m_runnerSteps.at(1)->m_masterIntensity, qreal(1.0));
    QCOMPARE(m_scene1->getAttributeValue(Function::Intensity), qreal(0.75));
    QCOMPARE(m_scene2->getAttributeValue(Function::Intensity), qreal(0.25));
    QCOMPARE(m_scene1->getAttributeValue(Scene::ParentIntensity), qreal(1.0));
    QCOMPARE(m_scene2->getAttributeValue(Scene::ParentIntensity), qreal(1.0));

    cr.adjustStepIntensity(0.0);

    QCOMPARE(cr.m_runnerSteps.at(0)->m_masterIntensity, qreal(0.0));
    QCOMPARE(cr.m_runnerSteps.at(1)->m_masterIntensity, qreal(0.0));
    QCOMPARE(m_scene1->getAttributeValue(Scene::ParentIntensity), qreal(0.0));
    QCOMPARE(m_scene2->getAttributeValue(Scene::ParentIntensity), qreal(0.0));

    timer->timerTick();

    QList<Universe *> universes = m_doc->inputOutputMap()->claimUniverses();
    universes[0]->processFaders(MasterTimer::tick());
    QCOMPARE(universes[0]->postGMValue(0), uchar(0));
    QCOMPARE(universes[0]->postGMValue(1), uchar(0));
    m_doc->inputOutputMap()->releaseUniverses(false);
}

/* Set up m_chaser as a looping Beats tempo Chaser with steps of $beats
   beats, started by a Show at $origin on $map */
static void setupTempoMapChaser(Chaser *chaser, const TempoMap &map, quint32 origin, uint beats)
{
    chaser->setTempoType(Function::Beats);
    chaser->setRunOrder(Function::Loop);
    chaser->setDirection(Function::Forward);
    chaser->setDurationMode(Chaser::Common);
    chaser->setDuration(beats);
    chaser->m_tempoMapClock = QSharedPointer<const TempoMapClock>(new TempoMapClock(map, origin));
}

void ChaserRunner_Test::tempoMapNoDrift()
{
    TempoMap map;
    map.addSection(TempoSection(0, 600000, 127.5));
    setupTempoMapChaser(m_chaser, map, 0, 1000);
    double beat = 60000.0 / 127.5;

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);
    int lastStep = -1;
    int changes = 0;

    // about 3 minutes: every step change must land on the tick nearest to
    // its beat, however far into the run it is
    for (int i = 0; i < 9400; i++)
    {
        QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
        timer.timerTick();

        if (cr.m_lastRunStepIdx != lastStep)
        {
            if (lastStep != -1)
            {
                changes++;
                double expected = changes * beat;
                QVERIFY2(qAbs(i * 20.0 - expected) <= 10.0,
                         qPrintable(QString("change %1 at %2 ms, expected %3 ms")
                                    .arg(changes).arg(i * 20).arg(expected)));
            }
            lastStep = cr.m_lastRunStepIdx;
        }
    }

    QCOMPARE(changes, int((9400 * 20.0 - 10) / beat));
}

void ChaserRunner_Test::tempoMapOffGridStart()
{
    TempoMap map;
    map.addSection(TempoSection(0, 600000, 120));
    // the item starts at 730 ms, at beat 1.46: the first step ends on beat 2
    setupTempoMapChaser(m_chaser, map, 730, 1000);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    QCOMPARE(cr.m_lastRunStepIdx, 0);
    QCOMPARE(cr.m_runnerSteps.first()->m_endTime, 1000.0);

    int i = 1;
    for (; i < 100 && cr.m_lastRunStepIdx == 0; i++)
        cr.write(&timer, QList<Universe*>());

    // the write at 730 + 13 ticks = 990 ms is the one nearest to 1000 ms
    QCOMPARE(i - 1, 13);
    QCOMPARE(cr.m_lastRunStepIdx, 1);
    QCOMPARE(cr.m_runnerSteps.first()->m_endTime, 1500.0);
}

void ChaserRunner_Test::tempoMapSeek()
{
    TempoMap map;
    map.addSection(TempoSection(0, 600000, 120));
    setupTempoMapChaser(m_chaser, map, 0, 1000);

    // started 3250 ms into the item: 6.5 beats, so the 7th step (index 0 of
    // the third round) is running, and it started at 3000 ms
    ChaserRunner cr(m_doc, m_chaser, 3250);
    MasterTimer timer(m_doc);

    QCOMPARE(cr.m_pendingAction.m_action, ChaserSetStepIndex);
    QCOMPARE(cr.m_pendingAction.m_stepIndex, 0);

    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    QCOMPARE(cr.m_lastRunStepIdx, 0);
    QCOMPARE(cr.m_runnerSteps.first()->m_endTime, 3500.0);

    // started 1250 ms in: step 2, which started at 1000 ms
    ChaserRunner cr2(m_doc, m_chaser, 1250);
    QCOMPARE(cr2.m_pendingAction.m_stepIndex, 2);
    QVERIFY(cr2.write(&timer, QList<Universe*>()) == true);
    QCOMPARE(cr2.m_runnerSteps.first()->m_endTime, 1500.0);
}

void ChaserRunner_Test::tempoMapFades()
{
    TempoMap map;
    map.addSection(TempoSection(0, 10000, 60));
    map.addSection(TempoSection(10000, 10000, 120));
    setupTempoMapChaser(m_chaser, map, 0, 1000);
    m_chaser->setFadeInMode(Chaser::Common);
    m_chaser->setFadeInSpeed(500);
    m_chaser->setFadeOutMode(Chaser::Common);
    m_chaser->setFadeOutSpeed(1000);

    // half a beat and a beat at 60 BPM
    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);
    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);
    QCOMPARE(cr.m_runnerSteps.first()->m_fadeIn, uint(500));
    QCOMPARE(cr.m_runnerSteps.first()->m_fadeOut, uint(1000));

    // and at 120 BPM
    ChaserRunner cr2(m_doc, m_chaser, 10000);
    QVERIFY(cr2.write(&timer, QList<Universe*>()) == true);
    QCOMPARE(cr2.m_runnerSteps.first()->m_fadeIn, uint(250));
    QCOMPARE(cr2.m_runnerSteps.first()->m_fadeOut, uint(500));
}

void ChaserRunner_Test::tempoMapNextStep()
{
    TempoMap map;
    map.addSection(TempoSection(0, 600000, 120));
    setupTempoMapChaser(m_chaser, map, 0, 2000);

    ChaserRunner cr(m_doc, m_chaser);
    MasterTimer timer(m_doc);
    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);

    // 10 ticks in (200 ms), a manual next step starts now and locks to the
    // grid again: 2 beats from beat 0.4 end on beat 2
    for (int i = 0; i < 9; i++)
        cr.write(&timer, QList<Universe*>());

    ChaserAction action;
    action.m_action = ChaserNextStep;
    action.m_masterIntensity = 1.0;
    action.m_stepIntensity = 1.0;
    action.m_fadeMode = Chaser::FromFunction;
    action.m_stepIndex = -1;
    cr.setAction(action);
    QVERIFY(cr.write(&timer, QList<Universe*>()) == true);

    QCOMPARE(cr.m_lastRunStepIdx, 1);
    QCOMPARE(cr.m_runnerSteps.first()->m_endTime, 1000.0);
}

QTEST_APPLESS_MAIN(ChaserRunner_Test)
