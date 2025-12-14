/*
  Q Light Controller Plus - Unit test
  chaser_test.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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
#define private public
#include "mastertimer_stub.h"
#include "chaserrunner.h"
#include "chaser_test.h"
#include "chaserstep.h"
#include "collection.h"
#include "function.h"
#include "fixture.h"
#include "universe.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "efx.h"
#include "bus.h"
#undef protected
#undef private

#include "qlcchannel.h"
#include "qlcfile.h"

#include "../common/resource_paths.h"

void Chaser_Test::initTestCase()
{
    m_doc = NULL;
}

void Chaser_Test::cleanupTestCase()
{
    delete m_doc;
}

void Chaser_Test::init()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void Chaser_Test::cleanup()
{
    m_doc->clearContents();
}

void Chaser_Test::initial()
{
    Chaser c(m_doc);
    QVERIFY(c.type() == Function::ChaserType);
    QVERIFY(c.name() == "New Chaser");
    QVERIFY(c.steps().size() == 0);
    QVERIFY(c.direction() == Chaser::Forward);
    QVERIFY(c.runOrder() == Chaser::Loop);
    QVERIFY(c.id() == Function::invalidId());
    QVERIFY(c.m_runner == NULL);
    QCOMPARE(c.m_legacyHoldBus, Bus::invalid());
    QCOMPARE(c.fadeInMode(), Chaser::Default);
    QCOMPARE(c.fadeOutMode(), Chaser::Default);
    QCOMPARE(c.durationMode(), Chaser::Common);
}

void Chaser_Test::directionRunOrder()
{
    Chaser c(m_doc);

    QVERIFY(c.direction() == Chaser::Forward);
    QVERIFY(c.runOrder() == Chaser::Loop);

    c.setDirection(Chaser::Backward);
    QVERIFY(c.direction() == Chaser::Backward);

    c.setRunOrder(Chaser::PingPong);
    QVERIFY(c.runOrder() == Chaser::PingPong);

    c.setRunOrder(Chaser::Random);
    QVERIFY(c.runOrder() == Chaser::Random);

    c.setDirection(Chaser::Forward);
    QVERIFY(c.direction() == Chaser::Forward);

    c.setRunOrder(Chaser::SingleShot);
    QVERIFY(c.runOrder() == Chaser::SingleShot);

    c.setDirection(Chaser::Backward);
    QVERIFY(c.direction() == Chaser::Backward);

    c.setRunOrder(Chaser::Loop);
    QVERIFY(c.runOrder() == Chaser::Loop);

    /* Check that invalid direction results in a sane fallback value */
    c.setDirection(Chaser::Direction(15));
    QVERIFY(c.direction() == Chaser::Forward);

    /* Check that invalid run order results in a sane fallback value */
    c.setRunOrder(Chaser::RunOrder(42));
    QVERIFY(c.runOrder() == Chaser::Loop);
}

void Chaser_Test::steps()
{
    Chaser c(m_doc);
    c.setID(50);
    QVERIFY(c.steps().size() == 0);

    /* A chaser should not be allowed to be its own member */
    QVERIFY(c.addStep(ChaserStep(50)) == false);
    QVERIFY(c.steps().size() == 0);

    /* Add a function with id "12" to the chaser */
    QVERIFY(c.addStep(12));
    QVERIFY(c.steps().size() == 1);
    QVERIFY(c.steps().at(0) == ChaserStep(12));

    /* Add another function in the middle */
    QVERIFY(c.addStep(ChaserStep(34)));
    QVERIFY(c.steps().size() == 2);
    QVERIFY(c.steps().at(0) == ChaserStep(12));
    QVERIFY(c.steps().at(1) == ChaserStep(34));

    /* Must be able to add the same function multiple times */
    QVERIFY(c.addStep(ChaserStep(12)));
    QVERIFY(c.steps().size() == 3);
    QVERIFY(c.steps().at(0) == ChaserStep(12));
    QVERIFY(c.steps().at(1) == ChaserStep(34));
    QVERIFY(c.steps().at(2) == ChaserStep(12));

    /* Must be able to replace a step to another */
    QVERIFY(c.replaceStep(ChaserStep(42), 1));
    QVERIFY(c.steps().at(0) == ChaserStep(12));
    QVERIFY(c.steps().at(1) == ChaserStep(42));
    QVERIFY(c.steps().at(2) == ChaserStep(12));

    /* Cannot replace a nonexistent step */
    QVERIFY(c.replaceStep(ChaserStep(69), 5) == false);

    /* Removing a non-existent index should make no modifications */
    QVERIFY(c.removeStep(3) == false);
    QVERIFY(c.steps().size() == 3);
    QVERIFY(c.steps().at(0) == ChaserStep(12));
    QVERIFY(c.steps().at(1) == ChaserStep(42));
    QVERIFY(c.steps().at(2) == ChaserStep(12));

    /* Removing the last step should succeed */
    QVERIFY(c.removeStep(2) == true);
    QVERIFY(c.steps().size() == 2);
    QVERIFY(c.steps().at(0) == ChaserStep(12));
    QVERIFY(c.steps().at(1) == ChaserStep(42));

    /* Removing the first step should succeed */
    QVERIFY(c.removeStep(0) == true);
    QVERIFY(c.steps().size() == 1);
    QVERIFY(c.steps().at(0) == ChaserStep(42));

    /* Removing the only step should succeed */
    QVERIFY(c.removeStep(0) == true);
    QVERIFY(c.steps().size() == 0);

    /* Step insertion */
    QVERIFY(c.addStep(ChaserStep(1), 0));
    QVERIFY(c.addStep(ChaserStep(2), 0));
    QVERIFY(c.steps().size() == 2);
    QVERIFY(c.steps().at(0) == ChaserStep(2));
    QVERIFY(c.steps().at(1) == ChaserStep(1));

    /* Move an invalid step will fail */
    QVERIFY(c.moveStep(5, 1) == false);

    QVERIFY(c.moveStep(1, 0) == true);
    QVERIFY(c.steps().size() == 2);
    QVERIFY(c.steps().at(0) == ChaserStep(1));
    QVERIFY(c.steps().at(1) == ChaserStep(2));
}

void Chaser_Test::stepAt()
{
    Chaser c(m_doc);
    c.setID(42);
    QVERIFY(c.addStep(ChaserStep(0, 1000, 5000, 0)) == true);
    QVERIFY(c.stepsCount() == 1);

    QVERIFY(c.stepAt(10) == NULL);
    ChaserStep *cs = c.stepAt(0);
    QVERIFY(cs != NULL);

    QVERIFY(cs->fadeIn == 1000);
    QVERIFY(cs->hold == 5000);

    cs->fadeIn = 500;
    cs->hold = 8000;

    QVERIFY(cs->fadeIn == 500);
    QVERIFY(cs->hold == 8000);
}

void Chaser_Test::functionRemoval()
{
    Chaser c(m_doc);
    c.setID(42);
    QVERIFY(c.steps().size() == 0);

    QVERIFY(c.addStep(ChaserStep(0)) == true);
    QVERIFY(c.addStep(ChaserStep(1)) == true);
    QVERIFY(c.addStep(ChaserStep(2)) == true);
    QVERIFY(c.addStep(ChaserStep(3)) == true);
    QVERIFY(c.steps().size() == 4);

    /* Simulate function removal signal with an uninteresting function id */
    c.slotFunctionRemoved(6);
    QVERIFY(c.steps().size() == 4);

    /* Simulate function removal signal with a function in the chaser */
    c.slotFunctionRemoved(1);
    QVERIFY(c.steps().size() == 3);
    QVERIFY(c.steps().at(0) == ChaserStep(0));
    QVERIFY(c.steps().at(1) == ChaserStep(2));
    QVERIFY(c.steps().at(2) == ChaserStep(3));

    /* Simulate function removal signal with an invalid function id */
    c.slotFunctionRemoved(Function::invalidId());
    QVERIFY(c.steps().size() == 3);
    QVERIFY(c.steps().at(0) == ChaserStep(0));
    QVERIFY(c.steps().at(1) == ChaserStep(2));
    QVERIFY(c.steps().at(2) == ChaserStep(3));

    /* Simulate function removal signal with a function in the chaser */
    c.slotFunctionRemoved(0);
    QVERIFY(c.steps().size() == 2);
    QVERIFY(c.steps().at(0) == ChaserStep(2));
    QVERIFY(c.steps().at(1) == ChaserStep(3));
}

void Chaser_Test::copyFrom()
{
    Chaser c1(m_doc);
    c1.setName("First");
    c1.setDirection(Chaser::Backward);
    c1.setRunOrder(Chaser::PingPong);
    c1.setFadeInSpeed(42);
    c1.setFadeOutSpeed(69);
    c1.setDuration(1337);
    c1.addStep(ChaserStep(2));
    c1.addStep(ChaserStep(0));
    c1.addStep(ChaserStep(1));
    c1.addStep(ChaserStep(25));

    /* Verify that chaser contents are copied */
    Chaser c2(m_doc);
    QSignalSpy spy(&c2, SIGNAL(changed(quint32)));
    QVERIFY(c2.copyFrom(&c1) == true);
    QCOMPARE(spy.size(), 1);
    QVERIFY(c2.name() == c1.name());
    QVERIFY(c2.fadeInSpeed() == 42);
    QVERIFY(c2.fadeOutSpeed() == 69);
    QVERIFY(c2.duration() == 1337);
    QVERIFY(c2.direction() == Chaser::Backward);
    QVERIFY(c2.runOrder() == Chaser::PingPong);
    QVERIFY(c2.steps().size() == 4);
    QVERIFY(c2.steps().at(0) == ChaserStep(2));
    QVERIFY(c2.steps().at(1) == ChaserStep(0));
    QVERIFY(c2.steps().at(2) == ChaserStep(1));
    QVERIFY(c2.steps().at(3) == ChaserStep(25));

    /* Verify that a Chaser gets a copy only from another Chaser */
    Scene s(m_doc);
    QVERIFY(c2.copyFrom(&s) == false);

    /* Make a third Chaser */
    Chaser c3(m_doc);
    c3.setName("Third");
    c3.setFadeInSpeed(142);
    c3.setFadeOutSpeed(169);
    c3.setDuration(11337);
    c3.setDirection(Chaser::Forward);
    c3.setRunOrder(Chaser::Loop);
    c3.addStep(ChaserStep(15));
    c3.addStep(ChaserStep(94));
    c3.addStep(ChaserStep(3));

    /* Verify that copying TO the same Chaser a second time succeeds and
       that steps are not appended but replaced completely. */
    QVERIFY(c2.copyFrom(&c3) == true);
    QVERIFY(c2.name() == c3.name());
    QVERIFY(c2.fadeInSpeed() == 142);
    QVERIFY(c2.fadeOutSpeed() == 169);
    QVERIFY(c2.duration() == 11337);
    QVERIFY(c2.direction() == Chaser::Forward);
    QVERIFY(c2.runOrder() == Chaser::Loop);
    QVERIFY(c2.steps().size() == 3);
    QVERIFY(c2.steps().at(0) == ChaserStep(15));
    QVERIFY(c2.steps().at(1) == ChaserStep(94));
    QVERIFY(c2.steps().at(2) == ChaserStep(3));
}

void Chaser_Test::createCopy()
{
    Doc doc(this);

    Chaser* c1 = new Chaser(m_doc);
    c1->setName("First");
    c1->setFadeInSpeed(42);
    c1->setFadeOutSpeed(69);
    c1->setDuration(1337);
    c1->setDirection(Chaser::Backward);
    c1->setRunOrder(Chaser::SingleShot);
    c1->addStep(ChaserStep(20));
    c1->addStep(ChaserStep(30));
    c1->addStep(ChaserStep(40));

    doc.addFunction(c1);
    QVERIFY(c1->id() != Function::invalidId());

    Function* f = c1->createCopy(&doc);
    QVERIFY(f != NULL);
    QVERIFY(f != c1);
    QVERIFY(f->id() != c1->id());

    Chaser* copy = qobject_cast<Chaser*> (f);
    QVERIFY(copy != NULL);
    QVERIFY(copy->fadeInSpeed() == 42);
    QVERIFY(copy->fadeOutSpeed() == 69);
    QVERIFY(copy->duration() == 1337);
    QVERIFY(copy->direction() == Chaser::Backward);
    QVERIFY(copy->runOrder() == Chaser::SingleShot);
    QVERIFY(copy->steps().size() == 3);
    QVERIFY(copy->steps().at(0) == ChaserStep(20));
    QVERIFY(copy->steps().at(1) == ChaserStep(30));
    QVERIFY(copy->steps().at(2) == ChaserStep(40));
}

void Chaser_Test::speedModes()
{
    Chaser c(m_doc);

    c.setFadeInMode(Chaser::Default);
    QCOMPARE(c.fadeInMode(), Chaser::Default);
    c.setFadeInMode(Chaser::Common);
    QCOMPARE(c.fadeInMode(), Chaser::Common);
    c.setFadeInMode(Chaser::PerStep);
    QCOMPARE(c.fadeInMode(), Chaser::PerStep);

    c.setFadeOutMode(Chaser::Default);
    QCOMPARE(c.fadeOutMode(), Chaser::Default);
    c.setFadeOutMode(Chaser::Common);
    QCOMPARE(c.fadeOutMode(), Chaser::Common);
    c.setFadeOutMode(Chaser::PerStep);
    QCOMPARE(c.fadeOutMode(), Chaser::PerStep);

    c.setDurationMode(Chaser::Default);
    QCOMPARE(c.durationMode(), Chaser::Default);
    c.setDurationMode(Chaser::Common);
    QCOMPARE(c.durationMode(), Chaser::Common);
    c.setDurationMode(Chaser::PerStep);
    QCOMPARE(c.durationMode(), Chaser::PerStep);

    QCOMPARE(Chaser::speedModeToString(Chaser::Default), QString("Default"));
    QCOMPARE(Chaser::speedModeToString(Chaser::Common), QString("Common"));
    QCOMPARE(Chaser::speedModeToString(Chaser::PerStep), QString("PerStep"));
    QCOMPARE(Chaser::speedModeToString(Chaser::SpeedMode(12345)), QString("Default"));

    QCOMPARE(Chaser::stringToSpeedMode("Default"), Chaser::Default);
    QCOMPARE(Chaser::stringToSpeedMode("Common"), Chaser::Common);
    QCOMPARE(Chaser::stringToSpeedMode("PerStep"), Chaser::PerStep);
    QCOMPARE(Chaser::stringToSpeedMode("Foobar"), Chaser::Default);
}

void Chaser_Test::loadSuccessLegacy()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Chaser");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Hold");
    xmlWriter.writeCharacters("16");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeTextElement("RunOrder", "SingleShot");

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters("50");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "2");
    xmlWriter.writeCharacters("12");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "0");
    xmlWriter.writeCharacters("87");
    xmlWriter.writeEndElement();

    // Unknown tag
    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeAttribute("Number", "3");
    xmlWriter.writeCharacters("1");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Bus::instance()->setValue(16, MasterTimer::frequency());

    Chaser c(m_doc);
    QVERIFY(c.loadXML(xmlReader) == true);
    QVERIFY(c.direction() == Chaser::Backward);
    QVERIFY(c.runOrder() == Chaser::SingleShot);
    QCOMPARE(c.steps().size(), 3);
    QVERIFY(c.steps().at(0) == ChaserStep(87));
    QVERIFY(c.steps().at(1) == ChaserStep(50));
    QVERIFY(c.steps().at(2) == ChaserStep(12));

    // postLoad() removes nonexistent functions so let's check this here
    c.postLoad();
    QCOMPARE(c.duration(), MasterTimer::frequency() * MasterTimer::tick());
}

void Chaser_Test::loadSuccess()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Chaser");

    xmlWriter.writeStartElement("Speed");
    xmlWriter.writeAttribute("FadeIn", "42");
    xmlWriter.writeAttribute("FadeOut", "69");
    xmlWriter.writeAttribute("Duration", "1337");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeTextElement("RunOrder", "SingleShot");

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeAttribute("FadeIn", "600");
    xmlWriter.writeAttribute("FadeOut", "700");
    xmlWriter.writeAttribute("Duration", "800");
    xmlWriter.writeCharacters("50");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "2");
    xmlWriter.writeAttribute("FadeIn", "1600");
    xmlWriter.writeAttribute("FadeOut", "1700");
    xmlWriter.writeAttribute("Duration", "1800");
    xmlWriter.writeCharacters("12");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "0");
    // Let's leave these out from this step just for test's sake
    //s3.setAttribute("FadeIn", 2600);
    //s3.setAttribute("FadeOut", 2700);
    //s3.setAttribute("Duration", 2800);
    xmlWriter.writeCharacters("87");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("SpeedModes");
    xmlWriter.writeAttribute("FadeIn", "Common");
    xmlWriter.writeAttribute("FadeOut", "Default");
    xmlWriter.writeAttribute("Duration", "PerStep");
    xmlWriter.writeEndElement();

    // Unknown tag
    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeAttribute("Number", "3");
    xmlWriter.writeCharacters("1");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Chaser c(m_doc);
    QVERIFY(c.loadXML(xmlReader) == true);
    QVERIFY(c.fadeInSpeed() == 42);
    QVERIFY(c.fadeOutSpeed() == 69);
    QVERIFY(c.duration() == 1337);
    QCOMPARE(c.fadeInMode(), Chaser::Common);
    QCOMPARE(c.fadeOutMode(), Chaser::Default);
    QCOMPARE(c.durationMode(), Chaser::PerStep);
    QVERIFY(c.direction() == Chaser::Backward);
    QVERIFY(c.runOrder() == Chaser::SingleShot);
    QVERIFY(c.steps().size() == 3);

    QVERIFY(c.steps().at(0) == ChaserStep(87));
    QCOMPARE(c.steps().at(0).fadeIn, uint(0));
    QCOMPARE(c.steps().at(0).fadeOut, uint(0));
    QCOMPARE(c.steps().at(0).duration, uint(0));

    QVERIFY(c.steps().at(1) == ChaserStep(50));
    QCOMPARE(c.steps().at(1).fadeIn, uint(600));
    QCOMPARE(c.steps().at(1).fadeOut, uint(700));
    QCOMPARE(c.steps().at(1).duration, uint(800));

    QVERIFY(c.steps().at(2) == ChaserStep(12));
    QCOMPARE(c.steps().at(2).fadeIn, uint(1600));
    QCOMPARE(c.steps().at(2).fadeOut, uint(1700));
    QCOMPARE(c.steps().at(2).duration, uint(1800));
}

void Chaser_Test::loadWrongType()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Scene");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Hold");
    xmlWriter.writeCharacters("16");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeTextElement("RunOrder", "SingleShot");

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters("50");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "2");
    xmlWriter.writeCharacters("12");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "0");
    xmlWriter.writeCharacters("87");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Chaser c(m_doc);
    QVERIFY(c.loadXML(xmlReader) == false);
}

void Chaser_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Chaser");
    xmlWriter.writeAttribute("Type", "Chaser");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Hold");
    xmlWriter.writeCharacters("16");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeTextElement("RunOrder", "SingleShot");

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters("50");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "2");
    xmlWriter.writeCharacters("12");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "0");
    xmlWriter.writeCharacters("87");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Chaser c(m_doc);
    QVERIFY(c.loadXML(xmlReader) == false);
}

void Chaser_Test::postLoad()
{
    Scene* sc1 = new Scene(m_doc);
    m_doc->addFunction(sc1);

    Scene* sc2 = new Scene(m_doc);
    m_doc->addFunction(sc2);

    Chaser* ch1 = new Chaser(m_doc);
    m_doc->addFunction(ch1);

    Chaser* ch2 = new Chaser(m_doc);
    m_doc->addFunction(ch2);

    Collection* co1 = new Collection(m_doc);
    m_doc->addFunction(co1);

    Collection* co2 = new Collection(m_doc);
    m_doc->addFunction(co2);

    EFX* ef1 = new EFX(m_doc);
    m_doc->addFunction(ef1);

    EFX* ef2 = new EFX(m_doc);
    m_doc->addFunction(ef2);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Chaser");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Hold");
    xmlWriter.writeCharacters("16");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeTextElement("RunOrder", "SingleShot");

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "0");
    xmlWriter.writeCharacters(QString::number(sc1->id()));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters(QString::number(sc2->id()));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "2");
    xmlWriter.writeCharacters(QString::number(ch1->id()));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "3");
    xmlWriter.writeCharacters(QString::number(ch2->id()));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "4");
    xmlWriter.writeCharacters(QString::number(co1->id()));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "5");
    xmlWriter.writeCharacters(QString::number(co2->id()));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "6");
    xmlWriter.writeCharacters(QString::number(ef1->id()));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "7");
    xmlWriter.writeCharacters(QString::number(ef2->id()));
    xmlWriter.writeEndElement();

    // Nonexistent function
    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "8");
    xmlWriter.writeCharacters(QString::number(INT_MAX - 1));
    xmlWriter.writeEndElement();

    // Unknown tag
    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeAttribute("Number", "3");
    xmlWriter.writeCharacters("1");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Chaser c(m_doc);
    QCOMPARE(c.loadXML(xmlReader), true);
    QCOMPARE(c.direction(), Chaser::Backward);
    QCOMPARE(c.runOrder(), Chaser::SingleShot);
    QCOMPARE(c.steps().size(), 9);

    c.postLoad();
    QCOMPARE(c.steps().size(), 8);
    QCOMPARE(c.steps().at(0), ChaserStep(sc1->id()));
    QCOMPARE(c.steps().at(1), ChaserStep(sc2->id()));
    QCOMPARE(c.steps().at(2), ChaserStep(ch1->id()));
    QCOMPARE(c.steps().at(3), ChaserStep(ch2->id()));
    QCOMPARE(c.steps().at(4), ChaserStep(co1->id()));
    QCOMPARE(c.steps().at(5), ChaserStep(co2->id()));
    QCOMPARE(c.steps().at(6), ChaserStep(ef1->id()));
    QCOMPARE(c.steps().at(7), ChaserStep(ef2->id()));
}

void Chaser_Test::save()
{
    Chaser c(m_doc);
    c.setDirection(Chaser::Backward);
    c.setRunOrder(Chaser::SingleShot);
    c.setFadeInSpeed(42);
    c.setFadeOutSpeed(69);
    c.setDuration(1337);
    c.addStep(ChaserStep(3));
    c.addStep(ChaserStep(1));
    c.addStep(ChaserStep(0));
    c.addStep(ChaserStep(2));
    c.setFadeInMode(Chaser::Default);
    c.setFadeOutMode(Chaser::PerStep);
    c.setDurationMode(Chaser::Common);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(c.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(xmlReader.name().toString() == "Function");
    QVERIFY(xmlReader.attributes().value("Type").toString() == "Chaser");

    int run = 0, dir = 0, speed = 0, fids = 0, speedmodes = 0;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name() == QString("Bus"))
        {
            QFAIL("Chaser must not write a Bus tag anymore!");
        }
        else if (xmlReader.name() == QString("Direction"))
        {
            QVERIFY(xmlReader.readElementText() == "Backward");
            dir++;
        }
        else if (xmlReader.name() == QString("RunOrder"))
        {
            QVERIFY(xmlReader.readElementText() == "SingleShot");
            run++;
        }
        else if (xmlReader.name() == QString("Step"))
        {
            quint32 fid = xmlReader.readElementText().toUInt();
            QVERIFY(fid == 0 || fid == 1 || fid == 2 || fid == 3);
            fids++;
        }
        else if (xmlReader.name() == QString("Speed"))
        {
            speed++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name() == QString("SpeedModes"))
        {
            QCOMPARE(xmlReader.attributes().value("FadeIn").toString(), QString("Default"));
            QCOMPARE(xmlReader.attributes().value("FadeOut").toString(), QString("PerStep"));
            QCOMPARE(xmlReader.attributes().value("Duration").toString(), QString("Common"));
            speedmodes++;
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL("Unhandled XML tag.");
            xmlReader.skipCurrentElement();
        }
    }

    QCOMPARE(speed, 1);
    QCOMPARE(speedmodes, 1);
    QCOMPARE(dir, 1);
    QCOMPARE(run, 1);
    QCOMPARE(fids, 4);
}

void Chaser_Test::tap()
{
    Scene* s1 = new Scene(m_doc);
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    m_doc->addFunction(s3);

    Scene* s4 = new Scene(m_doc);
    m_doc->addFunction(s4);

    Chaser* c = new Chaser(m_doc);
    c->addStep(s1->id());
    c->addStep(s2->id());
    c->addStep(s3->id());
    c->addStep(s4->id());
    m_doc->addFunction(c);

    c->preRun(m_doc->masterTimer());
    QVERIFY(c->m_runner != NULL);
    QCOMPARE(c->duration(), uint(0));
    c->write(m_doc->masterTimer(), QList<Universe*>());
    QCOMPARE(c->m_runner->m_pendingAction.m_action, ChaserNoAction);
    c->tap();
    QTest::qWait(MasterTimer::tick());
    c->tap();
    QCOMPARE(c->m_runner->m_pendingAction.m_action, ChaserNextStep);
}

void Chaser_Test::preRun()
{
    Chaser* c = new Chaser(m_doc);
    m_doc->addFunction(c);

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    MasterTimerStub timer(m_doc, ua);

    c->m_stop = true;

    c->preRun(&timer);
    QVERIFY(c->m_runner != NULL);
    QCOMPARE(c->isRunning(), true); // Make sure Function::preRun() is called
    //QCOMPARE(c->m_runner->m_elapsed, uint(0)); // Make sure ChaserRunner::reset() is called
    c->postRun(&timer, ua);
}

void Chaser_Test::writeHTP()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    Chaser* c = new Chaser(m_doc);
    c->setDuration(MasterTimer::tick() * 10);
    m_doc->addFunction(c);

    Scene* s1 = new Scene(m_doc);
    s1->setValue(fxi->id(), 0, 255);
    m_doc->addFunction(s1);
    c->addStep(s1->id());

    Scene* s2 = new Scene(m_doc);
    s2->setValue(fxi->id(), 0, 127);
    m_doc->addFunction(s2);
    c->addStep(s2->id());

    MasterTimer timer(m_doc);

    QVERIFY(c->isRunning() == false);
    QVERIFY(c->stopped() == true);
    c->start(&timer, FunctionParent::master());

    timer.timerTick();

    // check step 1
    for (uint i = MasterTimer::tick(); i < c->duration(); i += MasterTimer::tick())
    {
        timer.timerTick();
        QVERIFY(c->isRunning() == true);
        QVERIFY(c->stopped() == false);
        QVERIFY(s1->isRunning() == true);
        QVERIFY(s2->isRunning() == false);
    }

    // check step 2
    for (uint i = 0; i < c->duration(); i += MasterTimer::tick())
    {
        timer.timerTick();
        QVERIFY(c->isRunning() == true);
        QVERIFY(c->stopped() == false);
        QVERIFY(s1->isRunning() == false);
        QVERIFY(s2->isRunning() == true);
    }
}

void Chaser_Test::writeLTP()
{
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Clay Paky", "Sharpy");
    QVERIFY(def != NULL);

    QLCFixtureMode* mode = def->mode("Standard");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    QCOMPARE(fxi->channels(), quint32(16));
    m_doc->addFixture(fxi);

    Chaser* c = new Chaser(m_doc);
    c->setDuration(MasterTimer::tick() * 10);
    m_doc->addFunction(c);

    Scene* s1 = new Scene(m_doc);
    s1->setValue(fxi->id(), 0, 20);
    m_doc->addFunction(s1);
    c->addStep(s1->id());

    Scene* s2 = new Scene(m_doc);
    s2->setValue(fxi->id(), 0, 142);
    m_doc->addFunction(s2);
    c->addStep(s2->id());

    MasterTimer timer(m_doc);
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));

    QVERIFY(c->isRunning() == false);
    QVERIFY(c->stopped() == true);
    c->start(&timer, FunctionParent::master());

    timer.timerTick();

    // check step 1
    for (uint i = MasterTimer::tick(); i < c->duration(); i += MasterTimer::tick())
    {
        timer.timerTick();
        QVERIFY(c->isRunning() == true);
        QVERIFY(c->stopped() == false);
        QVERIFY(s1->isRunning() == true);
        QVERIFY(s2->isRunning() == false);
        ua = m_doc->inputOutputMap()->claimUniverses();
        ua[0]->processFaders();
        QVERIFY(ua[0]->preGMValues()[0] == (char)20);
        m_doc->inputOutputMap()->releaseUniverses(false);
    }

    // check step 2
    for (uint i = 0; i < c->duration(); i += MasterTimer::tick())
    {
        timer.timerTick();
        QVERIFY(c->isRunning() == true);
        QVERIFY(c->stopped() == false);
        QVERIFY(s1->isRunning() == false);
        QVERIFY(s2->isRunning() == true);
        ua = m_doc->inputOutputMap()->claimUniverses();
        ua[0]->processFaders();
        QVERIFY(ua[0]->preGMValues()[0] == (char)142);
        m_doc->inputOutputMap()->releaseUniverses(false);
    }

    // check step 1 again
    for (uint i = 0; i < c->duration(); i += MasterTimer::tick())
    {
        timer.timerTick();
        QVERIFY(c->isRunning() == true);
        QVERIFY(c->stopped() == false);
        QVERIFY(s1->isRunning() == true);
        QVERIFY(s2->isRunning() == false);
        ua = m_doc->inputOutputMap()->claimUniverses();
        ua[0]->processFaders();
        QVERIFY(ua[0]->preGMValues()[0] == (char)20);
        m_doc->inputOutputMap()->releaseUniverses(false);
    }
}

void Chaser_Test::postRun()
{
    Chaser* c = new Chaser(m_doc);
    m_doc->addFunction(c);

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    MasterTimerStub timer(m_doc, ua);

    c->preRun(&timer);
    QCOMPARE(c->isRunning(), true);

    // The chaser has no steps so ChaserRunner::postrun() shouldn't do much
    c->postRun(&timer, ua);
    QCOMPARE(c->isRunning(), false); // Make sure Function::postRun() is called
}

void Chaser_Test::adjustIntensity()
{
    Chaser* c = new Chaser(m_doc);
    m_doc->addFunction(c);

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    MasterTimerStub timer(m_doc, ua);

    c->preRun(&timer);
    c->adjustAttribute(0.5, Function::Intensity);
    QCOMPARE(c->m_runner->m_pendingAction.m_masterIntensity, qreal(0.5));
    c->adjustAttribute(0.8, Function::Intensity);
    QCOMPARE(c->m_runner->m_pendingAction.m_masterIntensity, qreal(0.8));
    c->adjustAttribute(1.5, Function::Intensity);
    QCOMPARE(c->m_runner->m_pendingAction.m_masterIntensity, qreal(1.0));
    c->adjustAttribute(-0.1, Function::Intensity);
    QCOMPARE(c->m_runner->m_pendingAction.m_masterIntensity, qreal(0.0));
    c->postRun(&timer, ua);

    // Mustn't crash after postRun
    c->adjustAttribute(1.0, Function::Intensity);
}

void Chaser_Test::quickChaser()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    Chaser* c = new Chaser(m_doc);
    // A really quick chaser
    c->setDuration(0);
    m_doc->addFunction(c);

    Scene* s1 = new Scene(m_doc);
    s1->setValue(fxi->id(), 0, 255);
    m_doc->addFunction(s1);
    c->addStep(s1->id());

    Scene* s2 = new Scene(m_doc);
    s2->setValue(fxi->id(), 0, 127);
    m_doc->addFunction(s2);
    c->addStep(s2->id());

    MasterTimer timer(m_doc);

    QVERIFY(c->isRunning() == false);
    QVERIFY(c->stopped() == true);
    c->start(&timer, FunctionParent::master());

    timer.timerTick();
    for (uint i = 0; i < 12; ++i)
    {
        timer.timerTick();
        QVERIFY(c->isRunning() == true);
        QVERIFY(c->stopped() == false);
        // always one function running while the other is not
        QVERIFY(s1->isRunning() == true || s2->isRunning() == true);
        QVERIFY(s1->stopped() == true || s2->stopped() == true);
    }

    c->stop(FunctionParent::master());

    timer.timerTick();

    QVERIFY(c->isRunning() == false);
    QVERIFY(c->stopped() == true);
    QVERIFY(s1->isRunning() == false);
    QVERIFY(s1->stopped() == true);
    QVERIFY(s2->isRunning() == false);
    QVERIFY(s2->stopped() == true);
}

QTEST_MAIN(Chaser_Test)
