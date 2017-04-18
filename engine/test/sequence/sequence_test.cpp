/*
  Q Light Controller Plus - Unit test
  sequence_test.cpp

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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#define protected public
#include "mastertimer_stub.h"
#include "sequence_test.h"
#include "chaserstep.h"
#include "sequence.h"
#include "universe.h"
#include "function.h"
#include "fixture.h"
#include "qlcfile.h"
#include "scene.h"
#include "doc.h"
#undef protected

void Sequence_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void Sequence_Test::cleanupTestCase()
{
    delete m_doc;
}

void Sequence_Test::init()
{
}

void Sequence_Test::cleanup()
{
    m_doc->clearContents();
}

void Sequence_Test::initial()
{
    Sequence s(m_doc);
    QVERIFY(s.type() == Function::SequenceType);
    QVERIFY(s.name() == "New Sequence");
    QCOMPARE(s.id(), Function::invalidId());
    QCOMPARE(s.boundSceneID(), Function::invalidId());
    QCOMPARE(s.steps().size(), 0);
    QCOMPARE(s.direction(), Sequence::Forward);
    QCOMPARE(s.runOrder(), Sequence::Loop);
    QCOMPARE(s.fadeInMode(), Sequence::Default);
    QCOMPARE(s.fadeOutMode(), Sequence::Default);
    QCOMPARE(s.durationMode(), Sequence::Common);
}

void Sequence_Test::createCopy()
{
    Doc doc(this);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(5);
    m_doc->addFixture(fxi);

    Scene *scene = new Scene(m_doc);
    scene->setValue(SceneValue(0, 0, 0));
    scene->setValue(SceneValue(0, 1, 0));
    scene->setValue(SceneValue(0, 2, 0));

    doc.addFunction(scene);
    QVERIFY(scene->id() != Function::invalidId());

    Sequence* seq = new Sequence(m_doc);
    seq->setName("First");
    seq->setFadeInSpeed(42);
    seq->setFadeOutSpeed(69);
    seq->setDuration(1337);
    seq->setDirection(Sequence::Backward);
    seq->setRunOrder(Sequence::SingleShot);
    seq->setBoundSceneID(scene->id());

    ChaserStep step;
    step.fid = scene->id();
    step.values.append(SceneValue(0, 0, 100));
    step.values.append(SceneValue(0, 1, 50));
    step.values.append(SceneValue(0, 2, 25));
    seq->addStep(step);
    step.values.replace(1, SceneValue(0, 1, 80));
    step.values.replace(2, SceneValue(0, 2, 200));
    seq->addStep(step);
    step.values.replace(0, SceneValue(0, 1, 180));
    step.values.replace(2, SceneValue(0, 2, 0));
    seq->addStep(step);
    QCOMPARE(seq->steps().size(), 3);


    doc.addFunction(seq);
    QVERIFY(seq->id() != Function::invalidId());

    Function* f = seq->createCopy(&doc);
    QVERIFY(f != NULL);
    QVERIFY(f != seq);
    QVERIFY(f->id() != seq->id());

    Sequence* copy = qobject_cast<Sequence*> (f);
    QVERIFY(copy != NULL);
    QVERIFY(copy->fadeInSpeed() == 42);
    QVERIFY(copy->fadeOutSpeed() == 69);
    QVERIFY(copy->duration() == 1337);
    QVERIFY(copy->direction() == Sequence::Backward);
    QVERIFY(copy->runOrder() == Sequence::SingleShot);
    QVERIFY(copy->boundSceneID() == scene->id());

    QCOMPARE(copy->steps().size(), 3);
    QVERIFY(copy->steps().at(0) == seq->steps().at(0));
    QVERIFY(copy->steps().at(1) == seq->steps().at(1));
    QVERIFY(copy->steps().at(2) == seq->steps().at(2));
}

QTEST_APPLESS_MAIN(Sequence_Test)
