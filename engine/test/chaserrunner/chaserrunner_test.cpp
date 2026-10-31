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

/* Count the writes until $cr moves on from the step it started with */
static int writesToNextStep(ChaserRunner &cr, MasterTimer *timer)
{
    int writes = 1;
    cr.write(timer, QList<Universe*>());
    int firstStep = cr.m_lastRunStepIdx;
    while (writes < 100 && cr.m_lastRunStepIdx == firstStep)
    {
        cr.write(timer, QList<Universe*>());
        writes++;
    }
    return writes;
}

void ChaserRunner_Test::writeStartOffset()
{
    m_chaser->setDirection(Function::Forward);
    m_chaser->setRunOrder(Function::Loop);
    m_chaser->setDurationMode(Chaser::Common);
    m_chaser->setDuration(MasterTimer::tick() * 10);
    MasterTimer timer(m_doc);

    ChaserRunner cr(m_doc, m_chaser);
    int fromStart = writesToNextStep(cr, &timer);
    cr.postRun(&timer, QList<Universe*>());

    // started two ticks into its first step: the step counts the offset
    // and the tick of the write that starts it, like a step started at 0
    ChaserRunner crOffset(m_doc, m_chaser, MasterTimer::tick() * 2);
    QVERIFY(crOffset.write(&timer, QList<Universe*>()) == true);
    QCOMPARE(crOffset.m_runnerSteps.first()->m_elapsed, MasterTimer::tick() * 3);
    crOffset.postRun(&timer, QList<Universe*>());

    // and ends two writes earlier
    ChaserRunner crOffset2(m_doc, m_chaser, MasterTimer::tick() * 2);
    QCOMPARE(writesToNextStep(crOffset2, &timer), fromStart - 2);
    crOffset2.postRun(&timer, QList<Universe*>());
}

/****************************************************************************
 * Beats tempo
 *
 * A beat lasts 50 ticks, so the runner beat timeline (1000 = 1 beat)
 * advances by 20 every tick, whatever the tick duration is.
 ****************************************************************************/

#define BEAT_TICKS 50

static void setupBeatsTimer(MasterTimer &timer)
{
    timer.m_beatTimeDuration = MasterTimer::tick() * BEAT_TICKS;
    timer.m_beatRequested = false;
}

static void setupBeatsChaser(Chaser *chaser, uint duration)
{
    chaser->setDirection(Function::Forward);
    chaser->setRunOrder(Function::Loop);
    chaser->setTempoType(Function::Beats);
    chaser->setDurationMode(Chaser::Common);
    chaser->setDuration(duration);
}

static void addFourthStep(Doc *doc, Chaser *chaser)
{
    Scene *scene4 = new Scene(doc);
    scene4->setName("S4");
    doc->addFunction(scene4);
    chaser->addStep(ChaserStep(scene4->id()));
}

/* Return the list of ticks, from $first to $last included, every $period ticks */
static QList<int> beatTicks(int first, int last, int period = BEAT_TICKS)
{
    QList<int> ticks;
    for (int i = first; i <= last; i += period)
        ticks << i;
    return ticks;
}

/* Write $cr for $ticks times, firing a beat on the writes listed in $beats.
 * Return the index of the running step after each write */
static QVector<int> runBeats(ChaserRunner &cr, MasterTimer &timer, int ticks, const QList<int> &beats)
{
    QVector<int> steps;
    for (int i = 0; i < ticks; i++)
    {
        timer.m_beatRequested = beats.contains(i);
        cr.write(&timer, QList<Universe*>());
        steps.append(cr.m_lastRunStepIdx);
    }
    timer.m_beatRequested = false;
    return steps;
}

/* Build the expected step index for each tick, from a list of
 * (first tick, step index) pairs sorted by tick */
static QVector<int> expectedSteps(int ticks, const QList<QPair<int, int>> &changes)
{
    QVector<int> steps(ticks, -1);
    for (int c = 0; c < changes.count(); c++)
    {
        int end = c + 1 < changes.count() ? changes.at(c + 1).first : ticks;
        for (int i = changes.at(c).first; i < end && i < ticks; i++)
            steps[i] = changes.at(c).second;
    }
    return steps;
}

/* Return a description of the first tick where the steps differ, or an empty string */
static QString stepsMismatch(const QVector<int> &actual, const QVector<int> &expected)
{
    for (int i = 0; i < expected.count(); i++)
    {
        if (i >= actual.count() || actual.at(i) != expected.at(i))
            return QString("tick %1: step %2, expected %3").arg(i)
                .arg(i < actual.count() ? actual.at(i) : -1).arg(expected.at(i));
    }
    return QString();
}

/* 4 steps of 1/4 beat with a beat every BEAT_TICKS: the steps change at
 * 250, 500 and 750 (ticks 13, 25 and 38) and the first step on the beat */
static QList<QPair<int, int>> quarterCycle(int beatTick)
{
    return QList<QPair<int, int>>()
           << qMakePair(beatTick, 0) << qMakePair(beatTick + 13, 1)
           << qMakePair(beatTick + 25, 2) << qMakePair(beatTick + 38, 3);
}

void ChaserRunner_Test::writeBeatsWhole()
{
    setupBeatsChaser(m_chaser, 1000);
    MasterTimer timer(m_doc);
    setupBeatsTimer(timer);

    // steps change only on beats. With no beats after tick 150,
    // the step started on that beat never ends
    ChaserRunner cr(m_doc, m_chaser);
    QVector<int> steps = runBeats(cr, timer, 230, beatTicks(0, 150));

    QVector<int> expected = expectedSteps(230, QList<QPair<int, int>>()
        << qMakePair(0, 0) << qMakePair(50, 1) << qMakePair(100, 2) << qMakePair(150, 0));

    QCOMPARE(stepsMismatch(steps, expected), QString());
    cr.postRun(&timer, QList<Universe*>());
}

void ChaserRunner_Test::writeBeatsQuarterLocked()
{
    addFourthStep(m_doc, m_chaser);
    setupBeatsChaser(m_chaser, 250);
    MasterTimer timer(m_doc);
    setupBeatsTimer(timer);

    // the first step starts exactly on every beat
    ChaserRunner cr(m_doc, m_chaser);
    QVector<int> steps = runBeats(cr, timer, 200, beatTicks(0, 150));

    QList<QPair<int, int>> changes;
    for (int beat = 0; beat < 200; beat += BEAT_TICKS)
        changes << quarterCycle(beat);

    QCOMPARE(stepsMismatch(steps, expectedSteps(200, changes)), QString());
    cr.postRun(&timer, QList<Universe*>());
}

void ChaserRunner_Test::writeBeatsLateBeat()
{
    addFourthStep(m_doc, m_chaser);
    setupBeatsChaser(m_chaser, 250);
    MasterTimer timer(m_doc);
    setupBeatsTimer(timer);

    // the second beat is 5 ticks late: the last step waits for it,
    // and the first step starts on it
    ChaserRunner cr(m_doc, m_chaser);
    QVector<int> steps = runBeats(cr, timer, 120, QList<int>() << 0 << 55 << 105);

    QList<QPair<int, int>> changes;
    changes << quarterCycle(0) << quarterCycle(55) << quarterCycle(105);

    QCOMPARE(stepsMismatch(steps, expectedSteps(120, changes)), QString());
    cr.postRun(&timer, QList<Universe*>());
}

void ChaserRunner_Test::writeBeatsEarlyBeat()
{
    addFourthStep(m_doc, m_chaser);
    setupBeatsChaser(m_chaser, 250);
    MasterTimer timer(m_doc);
    setupBeatsTimer(timer);

    // the second beat is 5 ticks early: the last step is cut short
    // (but not skipped) and the first step starts on the beat
    ChaserRunner cr(m_doc, m_chaser);
    QVector<int> steps = runBeats(cr, timer, 110, QList<int>() << 0 << 45 << 95);

    QList<QPair<int, int>> changes;
    changes << quarterCycle(0) << quarterCycle(45) << quarterCycle(95);

    QCOMPARE(stepsMismatch(steps, expectedSteps(110, changes)), QString());
    QCOMPARE(steps.at(44), 3);
    cr.postRun(&timer, QList<Universe*>());
}

void ChaserRunner_Test::writeBeatsOneAndHalf()
{
    setupBeatsChaser(m_chaser, 1500);
    MasterTimer timer(m_doc);
    setupBeatsTimer(timer);

    // steps change every 1.5 beats: half way between beats (in milliseconds)
    // or on the beat, when the whole steps time is a whole number of beats
    ChaserRunner cr(m_doc, m_chaser);
    QVector<int> steps = runBeats(cr, timer, 310, beatTicks(0, 300));

    QVector<int> expected = expectedSteps(310, QList<QPair<int, int>>()
        << qMakePair(0, 0) << qMakePair(75, 1) << qMakePair(150, 2)
        << qMakePair(225, 0) << qMakePair(300, 1));

    QCOMPARE(stepsMismatch(steps, expected), QString());
    cr.postRun(&timer, QList<Universe*>());
}

void ChaserRunner_Test::writeBeatsStartEarlyInBeat()
{
    addFourthStep(m_doc, m_chaser);
    setupBeatsChaser(m_chaser, 250);
    MasterTimer timer(m_doc);
    setupBeatsTimer(timer);

    // start 2 ticks after a beat (position 40): the first step is anchored
    // to that beat, so it is shorter and the next beat starts the first step
    ChaserRunner cr(m_doc, m_chaser);
    cr.m_beatClockStarted = true;
    cr.m_msSinceBeat = MasterTimer::tick();

    QVector<int> steps = runBeats(cr, timer, 75, QList<int>() << 48);

    QVector<int> expected = expectedSteps(75, QList<QPair<int, int>>()
        << qMakePair(0, 0) << qMakePair(11, 1) << qMakePair(23, 2) << qMakePair(36, 3)
        << quarterCycle(48));

    QCOMPARE(stepsMismatch(steps, expected), QString());
    cr.postRun(&timer, QList<Universe*>());
}

void ChaserRunner_Test::writeBeatsStartLateInBeat()
{
    addFourthStep(m_doc, m_chaser);
    setupBeatsChaser(m_chaser, 250);
    MasterTimer timer(m_doc);
    setupBeatsTimer(timer);

    // start 30 ticks after a beat (position 600): the first step lasts until
    // the next beat plus its own duration, then the steps follow the beat grid
    ChaserRunner cr(m_doc, m_chaser);
    cr.m_beatClockStarted = true;
    cr.m_msSinceBeat = MasterTimer::tick() * 29;

    QVector<int> steps = runBeats(cr, timer, 80, QList<int>() << 20 << 70);

    QVector<int> expected = expectedSteps(80, QList<QPair<int, int>>()
        << qMakePair(0, 0) << qMakePair(33, 1) << qMakePair(45, 2) << qMakePair(58, 3)
        << quarterCycle(70));

    QCOMPARE(stepsMismatch(steps, expected), QString());
    cr.postRun(&timer, QList<Universe*>());
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

void ChaserRunner_Test::randomSingleStep()
{
    Chaser *chaser = new Chaser(m_doc);
    chaser->addStep(ChaserStep(m_scene1->id()));
    chaser->setRunOrder(Function::Random);

    ChaserRunner cr(m_doc, chaser);
    cr.m_lastRunStepIdx = 0;

    // with a single step, the only valid next step is the same one
    QCOMPARE(cr.getNextStepIndex(), 0);
    QCOMPARE(cr.computeNextStep(0), 0);

    delete chaser;
}

void ChaserRunner_Test::randomBackwardRound()
{
    m_chaser->setRunOrder(Function::Random);
    m_chaser->setDirection(Function::Backward);

    for (int i = 0; i < 100; i++)
    {
        ChaserRunner cr(m_doc, m_chaser);

        // the last step of a backward round has been played
        cr.m_lastRunStepIdx = cr.m_order.at(0);

        int next = cr.getNextStepIndex();
        QVERIFY(next >= 0 && next < m_chaser->stepsCount());
        QVERIFY(next != cr.m_lastRunStepIdx);

        next = cr.computeNextStep(cr.m_order.at(0));
        QVERIFY(next >= 0 && next < m_chaser->stepsCount());
    }

    m_chaser->setRunOrder(Function::Loop);
    m_chaser->setDirection(Function::Forward);
}

void ChaserRunner_Test::randomOrderFollowsSteps()
{
    m_chaser->setRunOrder(Function::Random);
    ChaserRunner cr(m_doc, m_chaser);
    QCOMPARE(cr.m_order.size(), 3);

    // a step added while running must become part of the random order
    m_chaser->addStep(ChaserStep(m_scene1->id()));

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    MasterTimerStub timer(m_doc, ua);
    cr.write(&timer, ua);
    QCOMPARE(cr.m_order.size(), 4);

    cr.postRun(&timer, ua);
    m_chaser->setRunOrder(Function::Loop);
}

QTEST_APPLESS_MAIN(ChaserRunner_Test)
