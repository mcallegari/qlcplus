/*
  Q Light Controller Plus - Unit test
  scene_test.cpp

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
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "scene_test.h"
#include "qlcchannel.h"
#include "universe.h"
#include "function.h"
#include "fixture.h"
#include "qlcfile.h"
#include "scene.h"
#include "chaser.h"
#include "doc.h"
#include "bus.h"
#undef private
#undef protected

#include "../common/resource_paths.h"

void Scene_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void Scene_Test::cleanupTestCase()
{
    delete m_doc;
}

void Scene_Test::init()
{
}

void Scene_Test::cleanup()
{
    m_doc->clearContents();
}

void Scene_Test::initial()
{
    Scene s(m_doc);
    QVERIFY(s.type() == Function::Scene);
    QVERIFY(s.name() == "New Scene");
    QVERIFY(s.values().size() == 0);
    QVERIFY(s.id() == Function::invalidId());
    QCOMPARE(s.m_legacyFadeBus, Bus::invalid());
}

void Scene_Test::values()
{
    Scene s(m_doc);
    QVERIFY(s.values().size() == 0);

    /* Value 3 to fixture 1's channel number 2 */
    s.setValue(1, 2, 3);
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 3);

    /* Value 6 to fixture 4's channel number 5 */
    SceneValue scv(4, 5, 6);
    s.setValue(scv);
    QVERIFY(s.values().size() == 2);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 3);
    QVERIFY(s.values().at(1).fxi == 4);
    QVERIFY(s.values().at(1).channel == 5);
    QVERIFY(s.values().at(1).value == 6);

    /* Replace previous value 3 with 15 for fixture 1's channel number 2 */
    s.setValue(1, 2, 15);
    QVERIFY(s.values().size() == 2);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 15);
    QVERIFY(s.values().at(1).fxi == 4);
    QVERIFY(s.values().at(1).channel == 5);
    QVERIFY(s.values().at(1).value == 6);

    QVERIFY(s.value(1, 2) == 15);
    QVERIFY(s.value(3, 2) == 0); // No such channel
    QVERIFY(s.value(4, 5) == 6);

    /* No channel 5 for fixture 1 in the scene, unset shouldn't happen */
    s.unsetValue(1, 5);
    QVERIFY(s.values().size() == 2);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 15);
    QVERIFY(s.values().at(1).fxi == 4);
    QVERIFY(s.values().at(1).channel == 5);
    QVERIFY(s.values().at(1).value == 6);

    /* Remove fixture 1's channel 2 from the scene */
    s.unsetValue(1, 2);
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 4);
    QVERIFY(s.values().at(0).channel == 5);
    QVERIFY(s.values().at(0).value == 6);

    /* No fixture 1 anymore */
    s.unsetValue(1, 2);
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 4);
    QVERIFY(s.values().at(0).channel == 5);
    QVERIFY(s.values().at(0).value == 6);

    /* Remove fixture 4's channel 5 from the scene */
    s.unsetValue(4, 5);
    QVERIFY(s.values().size() == 0);

    s.setValue(1, 1, 255);
    s.setValue(2, 2, 255);
    s.setValue(4, 3, 255);
    s.setValue(1, 4, 255);
    QVERIFY(s.values().size() == 4);

    s.clear();
    QVERIFY(s.values().size() == 0);
}

void Scene_Test::fixtureRemoval()
{
    Scene s(m_doc);
    QVERIFY(s.values().size() == 0);

    s.setValue(1, 2, 3);
    s.setValue(4, 5, 6);
    QVERIFY(s.values().size() == 2);

    /* Simulate fixture removal signal with an uninteresting fixture id */
    s.slotFixtureRemoved(6);
    QVERIFY(s.values().size() == 2);

    /* Simulate fixture removal signal with a fixture in the scene */
    s.slotFixtureRemoved(4);
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 3);

    /* Simulate fixture removal signal with an invalid fixture id */
    s.slotFixtureRemoved(Fixture::invalidId());
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 3);

    /* Simulate fixture removal signal with a fixture in the scene */
    s.slotFixtureRemoved(1);
    QVERIFY(s.values().size() == 0);
}

void Scene_Test::loadSuccess()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Scene");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Fade");
    xmlWriter.writeCharacters("5");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Speed");
    xmlWriter.writeAttribute("FadeIn", "500");
    xmlWriter.writeAttribute("FadeOut", "5000");
    xmlWriter.writeAttribute("Duration", "50000");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Fixture", "5");
    xmlWriter.writeAttribute("Channel", "60");
    xmlWriter.writeCharacters("100");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Fixture", "133");
    xmlWriter.writeAttribute("Channel", "4");
    xmlWriter.writeCharacters("59");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeAttribute("Fixture", "133");
    xmlWriter.writeAttribute("Channel", "4");
    xmlWriter.writeCharacters("59");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Scene s(m_doc);
    QVERIFY(s.loadXML(xmlReader) == true);
    QVERIFY(s.fadeInSpeed() == 500);
    QVERIFY(s.fadeOutSpeed() == 5000);
    QVERIFY(s.duration() == 50000);
    QVERIFY(s.values().size() == 2);
    QVERIFY(s.value(5, 60) == 100);
    QVERIFY(s.value(133, 4) == 59);
}

void Scene_Test::loadWrongType()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Chaser");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Fade");
    xmlWriter.writeCharacters("5");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Fixture", "5");
    xmlWriter.writeAttribute("Channel", "60");
    xmlWriter.writeCharacters("100");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Fixture", "133");
    xmlWriter.writeAttribute("Channel", "4");
    xmlWriter.writeCharacters("59");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Scene s(m_doc);
    QVERIFY(s.loadXML(xmlReader) == false);
}

void Scene_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Scene");
    xmlWriter.writeAttribute("Type", "Scene");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Fade");
    xmlWriter.writeCharacters("5");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Fixture", "5");
    xmlWriter.writeAttribute("Channel", "60");
    xmlWriter.writeCharacters("100");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Fixture", "133");
    xmlWriter.writeAttribute("Channel", "4");
    xmlWriter.writeCharacters("59");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Scene s(m_doc);
    QVERIFY(s.loadXML(xmlReader) == false);
}

void Scene_Test::save()
{
    Scene s(m_doc);
    s.setFadeInSpeed(100);
    s.setFadeOutSpeed(1000);
    s.setDuration(10000);
    s.setValue(0, 0, 100);
    s.setValue(3, 0, 150);
    s.setValue(3, 3, 10);
    s.setValue(3, 5, 100);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(s.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(xmlReader.name().toString() == "Function");
    QVERIFY(xmlReader.attributes().value("Type").toString() == "Scene");

    xmlReader.readNextStartElement();

    QVERIFY(xmlReader.name().toString() == "Speed");
    QVERIFY(xmlReader.attributes().value("FadeIn").toString() == "100");
    QVERIFY(xmlReader.attributes().value("FadeOut").toString() == "1000");
    QVERIFY(xmlReader.attributes().value("Duration").toString() == "10000");

    xmlReader.skipCurrentElement();
    xmlReader.readNextStartElement();

    QVERIFY(xmlReader.name().toString() == "FixtureVal");
    QVERIFY(xmlReader.attributes().value("ID").toString() == "0");
    QVERIFY(xmlReader.readElementText() == "0,100");

    xmlReader.readNextStartElement();

    QVERIFY(xmlReader.name().toString() == "FixtureVal");
    QVERIFY(xmlReader.attributes().value("ID").toString() == "3");
    QVERIFY(xmlReader.readElementText() == "0,150,3,10,5,100");
}

void Scene_Test::copyFrom()
{
    Scene s1(m_doc);
    s1.setName("First");
    s1.setFadeInSpeed(100);
    s1.setFadeOutSpeed(1000);
    s1.setDuration(10000);
    s1.setValue(1, 2, 3);
    s1.setValue(4, 5, 6);
    s1.setValue(7, 8, 9);

    /* Verify that scene contents are copied */
    Scene s2(m_doc);
    QSignalSpy spy(&s2, SIGNAL(changed(quint32)));
    QVERIFY(s2.copyFrom(&s1) == true);
    QCOMPARE(spy.size(), 1);
    QVERIFY(s2.name() == s1.name());
    QVERIFY(s2.fadeInSpeed() == 100);
    QVERIFY(s2.fadeOutSpeed() == 1000);
    QVERIFY(s2.duration() == 10000);
    QVERIFY(s2.value(1, 2) == 3);
    QVERIFY(s2.value(4, 5) == 6);
    QVERIFY(s2.value(7, 8) == 9);

    /* Verify that a Scene gets a copy only from another Scene */
    Chaser c(m_doc);
    QVERIFY(s2.copyFrom(&c) == false);

    /* Make a third Scene */
    Scene s3(m_doc);
    s3.setName("Third");
    s3.setFadeInSpeed(200);
    s3.setFadeOutSpeed(2000);
    s3.setDuration(20000);
    s3.setValue(3, 1, 2);
    s3.setValue(6, 4, 5);
    s3.setValue(9, 7, 8);

    /* Verify that copying TO the same Scene a second time succeeds */
    QVERIFY(s2.copyFrom(&s3) == true);
    QVERIFY(s2.name() == s3.name());
    QVERIFY(s2.fadeInSpeed() == 200);
    QVERIFY(s2.fadeOutSpeed() == 2000);
    QVERIFY(s2.duration() == 20000);
    QVERIFY(s2.value(3, 1) == 2);
    QVERIFY(s2.value(6, 4) == 5);
    QVERIFY(s2.value(9, 7) == 8);
}

void Scene_Test::createCopy()
{
    Doc doc(this);

    Scene* s1 = new Scene(m_doc);
    s1->setName("First");
    s1->setFadeInSpeed(200);
    s1->setFadeOutSpeed(2000);
    s1->setDuration(20000);
    s1->setValue(1, 2, 3);
    s1->setValue(4, 5, 6);
    s1->setValue(7, 8, 9);

    doc.addFunction(s1);
    QVERIFY(s1->id() != Function::invalidId());

    Function* f = s1->createCopy(&doc);
    QVERIFY(f != NULL);
    QVERIFY(f != s1);
    QVERIFY(f->id() != s1->id());

    Scene* copy = qobject_cast<Scene*> (f);
    QVERIFY(copy != NULL);
    QVERIFY(copy->fadeInSpeed() == 200);
    QVERIFY(copy->fadeOutSpeed() == 2000);
    QVERIFY(copy->duration() == 20000);
    QVERIFY(copy->values().size() == 3);
    QVERIFY(copy->value(1, 2) == 3);
    QVERIFY(copy->value(4, 5) == 6);
    QVERIFY(copy->value(7, 8) == 9);
}

void Scene_Test::preRunPostRun()
{
    Doc* doc = new Doc(this);
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    MasterTimerStub timer(m_doc, ua);

    Fixture* fxi = new Fixture(doc);
    fxi->setName("Test Fixture");
    fxi->setAddress(15);
    fxi->setUniverse(3);
    fxi->setChannels(10);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, 123);
    s1->setValue(fxi->id(), 7, 45);
    s1->setValue(fxi->id(), 3, 67);
    doc->addFunction(s1);

    QVERIFY(s1->m_fader == NULL);
    s1->preRun(&timer);
    QVERIFY(s1->m_fader != NULL);

    s1->postRun(&timer, ua);

    delete doc;
}

void Scene_Test::flashUnflash()
{
    Doc* doc = new Doc(this);
    QList<Universe*> ua;
    MasterTimer timer(doc);

    Fixture* fxi = new Fixture(doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(10);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, 123);
    s1->setValue(fxi->id(), 1, 45);
    s1->setValue(fxi->id(), 2, 67);
    doc->addFunction(s1);

    QVERIFY(timer.m_dmxSourceList.size() == 0);

    s1->flash(&timer);
    QVERIFY(timer.m_dmxSourceList.size() == 1);
    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->flashing() == true);

    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[0] == char(123));
    QVERIFY(ua[0]->preGMValues()[1] == char(45));
    QVERIFY(ua[0]->preGMValues()[2] == char(67));
    doc->inputOutputMap()->releaseUniverses(false);

    s1->flash(&timer);
    QVERIFY(timer.m_dmxSourceList.size() == 1);
    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->flashing() == true);

    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[0] == char(123));
    QVERIFY(ua[0]->preGMValues()[1] == char(45));
    QVERIFY(ua[0]->preGMValues()[2] == char(67));
    doc->inputOutputMap()->releaseUniverses(false);

    s1->unFlash(&timer);
    QVERIFY(timer.m_dmxSourceList.size() == 1);
    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->flashing() == false);

    timer.timerTick();
    QVERIFY(timer.m_dmxSourceList.size() == 0);

    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[0] == char(0));
    QVERIFY(ua[0]->preGMValues()[1] == char(0));
    QVERIFY(ua[0]->preGMValues()[2] == char(0));
    doc->inputOutputMap()->releaseUniverses(false);
}

void Scene_Test::writeHTPZeroTicks()
{
    Doc* doc = new Doc(this);
    MasterTimer timer(doc);
    QList<Universe*> ua;

    Fixture* fxi = new Fixture(doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(10);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setFadeInSpeed(0);
    s1->setFadeOutSpeed(0);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, 255);
    s1->setValue(fxi->id(), 1, 127);
    s1->setValue(fxi->id(), 2, 0);
    doc->addFunction(s1);

    s1->start(&timer, Function::Source(Function::Source::God, 0));
    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[0] == (char) 255);
    QVERIFY(ua[0]->preGMValues()[1] == (char) 127);
    QVERIFY(ua[0]->preGMValues()[2] == (char) 0);
    QVERIFY(s1->stopped() == false);
    doc->inputOutputMap()->releaseUniverses(false);

    s1->stop(Function::Source(Function::Source::God, 0));
    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->isRunning() == true); // postRun has not been run yet, but..
    timer.timerTick();                // ..now it has.
    QVERIFY(s1->isRunning() == false);
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[0] == (char) 0);
    QVERIFY(ua[0]->preGMValues()[1] == (char) 0);
    QVERIFY(ua[0]->preGMValues()[2] == (char) 0);
    doc->inputOutputMap()->releaseUniverses(false);
}

void Scene_Test::writeHTPTwoTicks()
{
    Doc* doc = new Doc(this);
    MasterTimer timer(doc);
    QList<Universe*> ua;

    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(doc);
    fxi->setFixtureDefinition(def, mode);
    QCOMPARE(fxi->channels(), quint32(6));
    fxi->setAddress(0);
    fxi->setUniverse(0);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setFadeInSpeed(MasterTimer::tick() * 2);
    s1->setFadeOutSpeed(MasterTimer::tick() * 2);
    s1->setValue(fxi->id(), 5, 250); // HTP
    s1->setValue(fxi->id(), 0, 100); // LTP
    doc->addFunction(s1);

    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 0);
    QVERIFY(ua[0]->preGMValues()[0] == (char) 0);
    doc->inputOutputMap()->releaseUniverses(false);

    QVERIFY(s1->stopped() == true);
    s1->start(&timer, Function::Source(Function::Source::God, 0));
    timer.timerTick();

    QVERIFY(s1->stopped() == false);
    QVERIFY(s1->isRunning() == true);
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 125);
    QVERIFY(ua[0]->preGMValues()[0] == (char) 50);
    doc->inputOutputMap()->releaseUniverses(false);

    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 250);
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100);
    QVERIFY(s1->stopped() == false);
    doc->inputOutputMap()->releaseUniverses(false);

    // Values stay up
    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 250);
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100);
    QVERIFY(s1->stopped() == false);
    ua[0]->write(5, 255); // Overridden in the next round
    ua[0]->write(0, 42);  // Not overridden in the next round
    doc->inputOutputMap()->releaseUniverses(false);

    // Values stay up, LTP is not written anymore
    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 250);
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100);
    QVERIFY(s1->stopped() == false);
    doc->inputOutputMap()->releaseUniverses(false);

    s1->stop(Function::Source(Function::Source::God, 0));
    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->isRunning() == true);

    timer.timerTick();
    QVERIFY(s1->isRunning() == false);
    QVERIFY(s1->stopped() == true);
    // Now, the channels are inside MasterTimer's GenericFader to be zeroed out
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 125); // HTP fades out
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100);  // LTP doesn't
    doc->inputOutputMap()->releaseUniverses(false);

    // Now, the channels are inside MasterTimer's GenericFader to be zeroed out
    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 0);  // HTP fades out
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100); // LTP doesn't
    doc->inputOutputMap()->releaseUniverses(false);
}

void Scene_Test::writeHTPTwoTicksIntensity()
{
    Doc* doc = new Doc(this);
    MasterTimer timer(doc);
    QList<Universe*> ua;

    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(doc);
    fxi->setFixtureDefinition(def, mode);
    QCOMPARE(fxi->channels(), quint32(6));
    fxi->setAddress(0);
    fxi->setUniverse(0);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setFadeInSpeed(MasterTimer::tick() * 2);
    s1->setFadeOutSpeed(MasterTimer::tick() * 2);
    s1->setValue(fxi->id(), 5, 250); // HTP
    s1->setValue(fxi->id(), 0, 100); // LTP
    doc->addFunction(s1);

    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 0);
    QVERIFY(ua[0]->preGMValues()[0] == (char) 0);
    doc->inputOutputMap()->releaseUniverses(false);

    s1->adjustAttribute(0.5, Function::Intensity);

    QVERIFY(s1->stopped() == true);
    s1->start(&timer, Function::Source(Function::Source::God, 0));
    timer.timerTick();

    QVERIFY(s1->stopped() == false);
    QVERIFY(s1->isRunning() == true);
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) floor((qreal(125) * qreal(0.5)) + 0.5));
    QVERIFY(ua[0]->preGMValues()[0] == (char) 50); // Intensity affects only HTP channels
    doc->inputOutputMap()->releaseUniverses(false);

    s1->adjustAttribute(1.0, Function::Intensity);

    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 250);
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100);
    QVERIFY(s1->stopped() == false);
    doc->inputOutputMap()->releaseUniverses(false);

    s1->adjustAttribute(0.2, Function::Intensity);

    // Values stay up
    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) floor((qreal(250) * qreal(0.2)) + 0.5));
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100);
    QVERIFY(s1->stopped() == false);
    ua[0]->write(5, 255); // Overridden in the next round
    ua[0]->write(0, 42);  // Not overridden in the next round
    doc->inputOutputMap()->releaseUniverses(false);

    // Values stay up, LTP is not written anymore
    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) floor((qreal(250) * qreal(0.2)) + 0.5));
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100);
    QVERIFY(s1->stopped() == false);
    doc->inputOutputMap()->releaseUniverses(false);

    s1->stop(Function::Source(Function::Source::God, 0));
    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->isRunning() == true);

    timer.timerTick();
    QVERIFY(s1->isRunning() == false);
    QVERIFY(s1->stopped() == true);
    // Now, the channels are inside MasterTimer's GenericFader to be zeroed out
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) floor((qreal(125) * qreal(0.2)) + 0.5)); // HTP fades out
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100);  // LTP doesn't
    doc->inputOutputMap()->releaseUniverses(false);

    // Now, the channels are inside MasterTimer's GenericFader to be zeroed out
    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[5] == (char) 0);  // HTP fades out
    QVERIFY(ua[0]->preGMValues()[0] == (char) 100); // LTP doesn't
    doc->inputOutputMap()->releaseUniverses(false);
}

void Scene_Test::writeLTPReady()
{
    Doc* doc = new Doc(this);
    MasterTimer timer(doc);
    QList<Universe*> ua;

    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(doc);
    fxi->setFixtureDefinition(def, mode);
    QCOMPARE(fxi->channels(), quint32(6));
    fxi->setAddress(0);
    fxi->setUniverse(0);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setFadeInSpeed(MasterTimer::tick() * 2);
    s1->setFadeOutSpeed(MasterTimer::tick() * 2);
    s1->setValue(fxi->id(), 0, 250); // LTP
    s1->setValue(fxi->id(), 1, 200); // LTP
    s1->setValue(fxi->id(), 2, 100); // LTP
    doc->addFunction(s1);

    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->isRunning() == false);
    s1->start(&timer, Function::Source(Function::Source::God, 0));

    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[0] == (char) 125);
    QVERIFY(ua[0]->preGMValues()[1] == (char) 100);
    QVERIFY(ua[0]->preGMValues()[2] == (char) 50);
    doc->inputOutputMap()->releaseUniverses(false);

    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[0] == (char) 250);
    QVERIFY(ua[0]->preGMValues()[1] == (char) 200);
    QVERIFY(ua[0]->preGMValues()[2] == (char) 100);
    doc->inputOutputMap()->releaseUniverses(false);

    QVERIFY(s1->stopped() == false);
    QVERIFY(s1->isRunning() == true);

    // LTP values stay on
    timer.timerTick();
    ua = doc->inputOutputMap()->claimUniverses();
    QVERIFY(ua[0]->preGMValues()[0] == (char) 250);
    QVERIFY(ua[0]->preGMValues()[1] == (char) 200);
    QVERIFY(ua[0]->preGMValues()[2] == (char) 100);
    doc->inputOutputMap()->releaseUniverses(false);

    QVERIFY(s1->stopped() == false);
    QVERIFY(s1->isRunning() == true);
}

QTEST_APPLESS_MAIN(Scene_Test)
