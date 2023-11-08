/*
  Q Light Controller Plus - Unit test
  function_test.cpp

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

#include "function_test.h"

#define protected public
#define private public
#include "function_stub.h"
#include "function.h"
#undef private
#undef protected

#include "doc.h"

void Function_Test::initTestCase()
{
}

void Function_Test::initial()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    QCOMPARE(stub->id(), Function::invalidId());
    QCOMPARE(stub->name(), QString());
    QCOMPARE(stub->runOrder(), Function::Loop);
    QCOMPARE(stub->direction(), Function::Forward);
    QCOMPARE(stub->elapsed(), quint32(0));
    QCOMPARE(stub->elapsedBeats(), quint32(0));
    QCOMPARE(stub->stopped(), true);
    QCOMPARE(stub->fadeInSpeed(), uint(0));
    QCOMPARE(stub->fadeOutSpeed(), uint(0));
    QCOMPARE(stub->duration(), uint(0));
    QCOMPARE(stub->totalDuration(), uint(0));
    QCOMPARE(stub->overrideFadeInSpeed(), Function::defaultSpeed());
    QCOMPARE(stub->overrideFadeOutSpeed(), Function::defaultSpeed());
    QCOMPARE(stub->overrideDuration(), Function::defaultSpeed());
    QVERIFY(stub->saveXML(NULL) == false);
    QXmlStreamReader reader;
    QVERIFY(stub->loadXML(reader) == false);
}

void Function_Test::properties()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    doc.addFunction(stub);

    QSignalSpy spy(stub, SIGNAL(changed(quint32)));
    QSignalSpy nameSpy(stub, SIGNAL(nameChanged(quint32)));

    stub->setName("Test");
    QCOMPARE(nameSpy.size(), 1);
    QCOMPARE(nameSpy[0][0].toUInt(), stub->id());
    QCOMPARE(stub->name(), QString("Test"));

    stub->setRunOrder(Function::PingPong);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0][0].toUInt(), stub->id());
    QCOMPARE(stub->runOrder(), Function::PingPong);

    stub->setDirection(Function::Backward);
    QCOMPARE(spy.size(), 2);
    QCOMPARE(spy[1][0].toUInt(), stub->id());
    QCOMPARE(stub->direction(), Function::Backward);

    stub->setFadeInSpeed(14);
    QCOMPARE(spy.size(), 3);
    QCOMPARE(spy[2][0].toUInt(), stub->id());
    QCOMPARE(stub->fadeInSpeed(), uint(14));

    stub->setFadeOutSpeed(42);
    QCOMPARE(spy.size(), 4);
    QCOMPARE(spy[3][0].toUInt(), stub->id());
    QCOMPARE(stub->fadeOutSpeed(), uint(42));

    stub->setDuration(69);
    QCOMPARE(spy.size(), 5);
    QCOMPARE(spy[4][0].toUInt(), stub->id());
    QCOMPARE(stub->duration(), uint(69));

    QVERIFY(stub->isVisible() == true);
    stub->setVisible(false);
    QVERIFY(stub->isVisible() == false);

    QVERIFY(stub->uiStateMap().size() == 0);
    QVERIFY(stub->uiStateValue("foo").isNull() == true);
    stub->setUiStateValue("foo", 42);
    QVERIFY(stub->uiStateMap().size() == 1);
    QVERIFY(stub->uiStateValue("foo").toInt() == 42);
}

void Function_Test::copyFrom()
{
    Doc doc(this);

    Function_Stub* stub1 = new Function_Stub(&doc);
    QVERIFY(stub1->copyFrom(NULL) == false);
    stub1->setName("Stub1");
    stub1->setRunOrder(Function::PingPong);
    stub1->setDirection(Function::Backward);
    stub1->setFadeInSpeed(42);
    stub1->setFadeOutSpeed(69);
    stub1->setDuration(1337);

    Function_Stub* stub2 = new Function_Stub(&doc);
    QSignalSpy spy(stub2, SIGNAL(changed(quint32)));
    stub2->copyFrom(stub1);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0][0].toUInt(), quint32(stub2->id()));
    QCOMPARE(stub2->name(), QString("Stub1"));
    QCOMPARE(stub2->runOrder(), Function::PingPong);
    QCOMPARE(stub2->direction(), Function::Backward);
    QCOMPARE(stub2->fadeInSpeed(), uint(42));
    QCOMPARE(stub2->fadeOutSpeed(), uint(69));
    QCOMPARE(stub2->duration(), uint(1337));
}

void Function_Test::flashUnflash()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    QSignalSpy spy(stub, SIGNAL(flashing(quint32,bool)));

    QVERIFY(stub->flashing() == false);
    stub->flash(NULL, false, false);
    QCOMPARE(spy.size(), 1);
    QVERIFY(stub->flashing() == true);
    stub->flash(NULL, false, false);
    QCOMPARE(spy.size(), 1);
    QVERIFY(stub->flashing() == true);
    stub->unFlash(NULL);
    QCOMPARE(spy.size(), 2);
    QVERIFY(stub->flashing() == false);
}

void Function_Test::elapsed()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    QCOMPARE(stub->elapsed(), MasterTimer::tick() * 0);
    stub->incrementElapsed();
    QCOMPARE(stub->elapsed(), MasterTimer::tick() * 1);
    stub->incrementElapsed();
    QCOMPARE(stub->elapsed(), MasterTimer::tick() * 2);
    stub->incrementElapsed();
    QCOMPARE(stub->elapsed(), MasterTimer::tick() * 3);
    stub->resetElapsed();
    QCOMPARE(stub->elapsed(), MasterTimer::tick() * 0);

    stub->m_elapsed = UINT_MAX;
    stub->incrementElapsed();
    QCOMPARE(stub->elapsed(), quint32(UINT_MAX));
    stub->incrementElapsed();
    QCOMPARE(stub->elapsed(), quint32(UINT_MAX));
}

void Function_Test::preRunPostRun()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    QSignalSpy spyRunning(stub, SIGNAL(running(quint32)));
    stub->preRun(NULL);
    QVERIFY(stub->isRunning() == true);
    QCOMPARE(spyRunning.size(), 1);
    // @todo Check the contents of the signal in spyRunning

    stub->incrementElapsed();

    QSignalSpy spyStopped(stub, SIGNAL(stopped(quint32)));
    stub->postRun(NULL, QList<Universe*>());
    QVERIFY(stub->stopped() == true);
    QVERIFY(stub->isRunning() == false);
    QCOMPARE(stub->elapsed(), quint32(0));
    QCOMPARE(spyRunning.size(), 1);
    QCOMPARE(spyStopped.size(), 1);
    // @todo Check the contents of the signal in spyStopped
}

void Function_Test::stopAndWait()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    stub->preRun(NULL);
    stub->incrementElapsed();

    // @todo Make stopAndWait() return before the 2s watchdog timer
    //QSignalSpy spyStopped(stub, SIGNAL(stopped(quint32)));
    //QVERIFY(stub->stopAndWait() == true);
}

void Function_Test::stopAndWaitFail()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    stub->preRun(NULL);
    stub->incrementElapsed();

    QSignalSpy spyStopped(stub, SIGNAL(stopped(quint32)));
    QVERIFY(stub->stopAndWait() == false);
}

void Function_Test::adjustIntensity()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    QCOMPARE(stub->getAttributeValue(Function::Intensity), qreal(1.0));

    stub->adjustAttribute(0.5, Function::Intensity);
    QCOMPARE(stub->getAttributeValue(Function::Intensity), qreal(0.5));

    stub->adjustAttribute(1.5, Function::Intensity);
    QCOMPARE(stub->getAttributeValue(Function::Intensity), qreal(1.0));

    stub->adjustAttribute(-7.0, Function::Intensity);
    QCOMPARE(stub->getAttributeValue(Function::Intensity), qreal(0));

    stub->resetAttributes();
    QCOMPARE(stub->getAttributeValue(Function::Intensity), qreal(0.0));
}

void Function_Test::slotFixtureRemoved()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    Fixture* fxi = new Fixture(&doc);
    fxi->setID(42);
    QVERIFY(doc.addFixture(fxi, fxi->id()) == true);
    QVERIFY(doc.addFunction(stub) == true);

    QCOMPARE(stub->m_slotFixtureRemovedId, Function::invalidId());
    doc.deleteFixture(42);
    QCOMPARE(stub->m_slotFixtureRemovedId, quint32(42));
}

void Function_Test::invalidId()
{
    QCOMPARE(Function::invalidId(), quint32(UINT_MAX));
}

void Function_Test::typeString()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    QCOMPARE(stub->typeString(), Function::typeToString(Function::Type(31337)));
    stub->m_type = Function::SceneType;
    QCOMPARE(stub->typeString(), Function::typeToString(Function::SceneType));
    stub->m_type = Function::ChaserType;
    QCOMPARE(stub->typeString(), Function::typeToString(Function::ChaserType));
}

void Function_Test::typeToString()
{
    QVERIFY(Function::typeToString(Function::Undefined) == "Undefined");
    QVERIFY(Function::typeToString(Function::SceneType) == "Scene");
    QVERIFY(Function::typeToString(Function::ChaserType) == "Chaser");
    QVERIFY(Function::typeToString(Function::EFXType) == "EFX");
    QVERIFY(Function::typeToString(Function::CollectionType) == "Collection");
    QVERIFY(Function::typeToString(Function::RGBMatrixType) == "RGBMatrix");
    QVERIFY(Function::typeToString(Function::ScriptType) == "Script");
    QVERIFY(Function::typeToString(Function::SequenceType) == "Sequence");
    QVERIFY(Function::typeToString(Function::ShowType) == "Show");
    QVERIFY(Function::typeToString(Function::AudioType) == "Audio");
    QVERIFY(Function::typeToString(Function::VideoType) == "Video");

    QVERIFY(Function::typeToString(Function::Type(42)) == "Undefined");
    QVERIFY(Function::typeToString(Function::Type(31337)) == "Undefined");
}

void Function_Test::stringToType()
{
    QVERIFY(Function::stringToType("Undefined") == Function::Undefined);
    QVERIFY(Function::stringToType("Scene") == Function::SceneType);
    QVERIFY(Function::stringToType("Chaser") == Function::ChaserType);
    QVERIFY(Function::stringToType("EFX") == Function::EFXType);
    QVERIFY(Function::stringToType("Collection") == Function::CollectionType);
    QVERIFY(Function::stringToType("RGBMatrix") == Function::RGBMatrixType);
    QVERIFY(Function::stringToType("Script") == Function::ScriptType);
    QVERIFY(Function::stringToType("Sequence") == Function::SequenceType);
    QVERIFY(Function::stringToType("Show") == Function::ShowType);
    QVERIFY(Function::stringToType("Audio") == Function::AudioType);
    QVERIFY(Function::stringToType("Video") == Function::VideoType);

    QVERIFY(Function::stringToType("Foobar") == Function::Undefined);
    QVERIFY(Function::stringToType("Xyzzy") == Function::Undefined);
}

void Function_Test::runOrderToString()
{
    QVERIFY(Function::runOrderToString(Function::Loop) == "Loop");
    QVERIFY(Function::runOrderToString(Function::SingleShot) == "SingleShot");
    QVERIFY(Function::runOrderToString(Function::PingPong) == "PingPong");
    QVERIFY(Function::runOrderToString(Function::Random) == "Random");

    QVERIFY(Function::runOrderToString(Function::RunOrder(42)) == "Loop");
    QVERIFY(Function::runOrderToString(Function::RunOrder(69)) == "Loop");
}

void Function_Test::stringToRunOrder()
{
    QVERIFY(Function::stringToRunOrder("Loop") == Function::Loop);
    QVERIFY(Function::stringToRunOrder("SingleShot") == Function::SingleShot);
    QVERIFY(Function::stringToRunOrder("PingPong") == Function::PingPong);
    QVERIFY(Function::stringToRunOrder("Random") == Function::Random);

    QVERIFY(Function::stringToRunOrder("Foobar") == Function::Loop);
    QVERIFY(Function::stringToRunOrder("Xyzzy") == Function::Loop);
}

void Function_Test::directionToString()
{
    QVERIFY(Function::directionToString(Function::Forward) == "Forward");
    QVERIFY(Function::directionToString(Function::Backward) == "Backward");

    QVERIFY(Function::directionToString(Function::Direction(42)) == "Forward");
    QVERIFY(Function::directionToString(Function::Direction(69)) == "Forward");
}

void Function_Test::stringToDirection()
{
    QVERIFY(Function::stringToDirection("Forward") == Function::Forward);
    QVERIFY(Function::stringToDirection("Backward") == Function::Backward);

    QVERIFY(Function::stringToDirection("Foobar") == Function::Forward);
    QVERIFY(Function::stringToDirection("Xyzzy") == Function::Forward);
}

void Function_Test::speedToString()
{
    QCOMPARE(Function::speedToString(0), QString("0ms"));
    QCOMPARE(Function::speedToString(1000), QString("1s"));
    QCOMPARE(Function::speedToString(1000 * 60), QString("1m"));
    QCOMPARE(Function::speedToString(1000 * 60 * 60), QString("1h"));

    QCOMPARE(Function::speedToString(990), QString("990ms"));
    QCOMPARE(Function::speedToString(990 + 59 * 1000), QString("59s990ms"));
    QCOMPARE(Function::speedToString(990 + 59 * 1000 + 59 * 1000 * 60), QString("59m59s990ms"));
    QCOMPARE(Function::speedToString(990 + 59 * 1000 + 59 * 1000 * 60 + 99 * 1000 * 60 * 60), QString("99h59m59s990ms"));
    QCOMPARE(Function::speedToString(999 + 59 * 1000 + 59 * 1000 * 60 + 99 * 1000 * 60 * 60), QString("99h59m59s999ms"));
    QCOMPARE(Function::speedToString(1 + 1 * 1000 + 1 * 1000 * 60 + 1 * 1000 * 60 * 60), QString("1h01m01s001ms"));
    QCOMPARE(Function::speedToString(100), QString("100ms"));

    QCOMPARE(Function::speedToString(10), QString("10ms"));
    QCOMPARE(Function::speedToString(Function::infiniteSpeed()), QString(QChar(0x221E)));
}

void Function_Test::stringToSpeed()
{
    QCOMPARE(Function::stringToSpeed(".0"), uint(0));
    QCOMPARE(Function::stringToSpeed(".0."), uint(0));
    QCOMPARE(Function::stringToSpeed("0"), uint(0));
    QCOMPARE(Function::stringToSpeed("0.0"), uint(0));

    QCOMPARE(Function::stringToSpeed(".01"), uint(10));
    QCOMPARE(Function::stringToSpeed(".010"), uint(10));
    QCOMPARE(Function::stringToSpeed(".011"), uint(11));

    QCOMPARE(Function::stringToSpeed(".03"), uint(30));
    QCOMPARE(Function::stringToSpeed(".030"), uint(30));
    QCOMPARE(Function::stringToSpeed(".031"), uint(31));

    QCOMPARE(Function::stringToSpeed(".1"), uint(100));
    QCOMPARE(Function::stringToSpeed(".10"), uint(100));
    QCOMPARE(Function::stringToSpeed(".100"), uint(100));
    QCOMPARE(Function::stringToSpeed(".101"), uint(101));

    QCOMPARE(Function::stringToSpeed("1"), uint(1));
    QCOMPARE(Function::stringToSpeed("1s"), uint(1000));
    QCOMPARE(Function::stringToSpeed("1.000"), uint(1000));
    QCOMPARE(Function::stringToSpeed("1.001"), uint(1001));
    QCOMPARE(Function::stringToSpeed("1s.00"), uint(1000));
    QCOMPARE(Function::stringToSpeed("1ms"), uint(1));

    QCOMPARE(Function::stringToSpeed("1s.01"), uint(10 + 1000));
    QCOMPARE(Function::stringToSpeed("1m1s.01"), uint(10 + 1000 + 1000 * 60));
    QCOMPARE(Function::stringToSpeed("1h1m1s.01"), uint(10 + 1000 + 1000 * 60 + 1000 * 60 * 60));

    QCOMPARE(Function::stringToSpeed("59s12ms"), uint(12 + 59 * 1000));
    QCOMPARE(Function::stringToSpeed("59m59s999ms"), uint(999 + 59 * 1000 + 59 * 1000 * 60));

    // This string is broken, voluntarily ignore ms
    QCOMPARE(Function::stringToSpeed("59m59s.999ms"), uint(/*999 +*/ 59 * 1000 + 59 * 1000 * 60));

    QCOMPARE(Function::stringToSpeed(QString(QChar(0x221E))), Function::infiniteSpeed());
}

void Function_Test::speedOperations()
{
    QCOMPARE(Function::speedNormalize(-1), Function::infiniteSpeed());
    QCOMPARE(Function::speedNormalize(-10), Function::infiniteSpeed());
    QCOMPARE(Function::speedNormalize(0), uint(0));
    QCOMPARE(Function::speedNormalize(12), uint(12));
    QCOMPARE(Function::speedNormalize(10), uint(10));
    QCOMPARE(Function::speedNormalize(20), uint(20));
    QCOMPARE(Function::speedNormalize(30), uint(30));
    QCOMPARE(Function::speedNormalize(40), uint(40));
    QCOMPARE(Function::speedNormalize(50), uint(50));
    QCOMPARE(Function::speedNormalize(60), uint(60));

    QCOMPARE(Function::speedAdd(10, 10), uint(20));
    QCOMPARE(Function::speedAdd(10, 0), uint(10));
    QCOMPARE(Function::speedAdd(0, 10), uint(10));
    QCOMPARE(Function::speedAdd(15, 15), uint(30));
    QCOMPARE(Function::speedAdd(Function::infiniteSpeed(), 10), Function::infiniteSpeed());
    QCOMPARE(Function::speedAdd(10, Function::infiniteSpeed()), Function::infiniteSpeed());
    QCOMPARE(Function::speedAdd(Function::infiniteSpeed(), Function::infiniteSpeed()), Function::infiniteSpeed());
    QCOMPARE(Function::speedAdd(10, 0), uint(10));
    QCOMPARE(Function::speedAdd(20, 0), uint(20));
    QCOMPARE(Function::speedAdd(30, 0), uint(30));
    QCOMPARE(Function::speedAdd(40, 0), uint(40));
    QCOMPARE(Function::speedAdd(50, 0), uint(50));
    QCOMPARE(Function::speedAdd(60, 0), uint(60));
    QCOMPARE(Function::speedAdd(70, 0), uint(70));

    QCOMPARE(Function::speedSubtract(10, 10), uint(0));
    QCOMPARE(Function::speedSubtract(10, 0), uint(10));
    QCOMPARE(Function::speedSubtract(0, 10), uint(0));
    QCOMPARE(Function::speedSubtract(15, 2), uint(13));
    QCOMPARE(Function::speedSubtract(Function::infiniteSpeed(), 10), Function::infiniteSpeed());
    QCOMPARE(Function::speedSubtract(10, Function::infiniteSpeed()), uint(0));
    QCOMPARE(Function::speedSubtract(Function::infiniteSpeed(), Function::infiniteSpeed()), uint(0));
}

void Function_Test::tempo()
{
    Doc doc(this);
    Function_Stub* stub = new Function_Stub(&doc);

    QVERIFY(stub->tempoType() == Function::Time);

    QVERIFY(Function::tempoTypeToString(Function::Time) == QString("Time"));
    QVERIFY(Function::tempoTypeToString(Function::Beats) == QString("Beats"));

    QVERIFY(Function::stringToTempoType("Time") == Function::Time);
    QVERIFY(Function::stringToTempoType("Beats") == Function::Beats);

    QVERIFY(Function::timeToBeats(0, 0) == 0);
    QVERIFY(Function::timeToBeats(60000, 500) == 120000);

    QVERIFY(Function::beatsToTime(0, 0) == 0);
    QVERIFY(Function::beatsToTime(60000, 500) == 30000);

    /* check that setting the same tempo type does nothing */
    stub->setTempoType(Function::Time);
    QVERIFY(stub->tempoType() == Function::Time);

    stub->setFadeInSpeed(1000);
    stub->setDuration(4000);
    stub->setFadeOutSpeed(2000);

    /* check Time -> Beats switch */
    stub->setTempoType(Function::Beats);
    QVERIFY(stub->fadeInSpeed() == 2000);
    QVERIFY(stub->duration() == 8000);
    QVERIFY(stub->fadeOutSpeed() == 4000);

    /* check Beats -> Time switch */
    stub->setTempoType(Function::Time);
    QVERIFY(stub->fadeInSpeed() == 1000);
    QVERIFY(stub->duration() == 4000);
    QVERIFY(stub->fadeOutSpeed() == 2000);

    QVERIFY(stub->overrideTempoType() == Function::Original);
    stub->setOverrideTempoType(Function::Beats);
    QVERIFY(stub->overrideTempoType() == Function::Beats);
}

void Function_Test::attributes()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);

    QCOMPARE(stub->attributes().count(), 1); // Intensity is always there
    QCOMPARE(stub->getAttributeValue(Function::Intensity), 1.0);
    stub->adjustAttribute(0.5, Function::Intensity);
    QCOMPARE(stub->getAttributeValue(Function::Intensity), 0.5);

    stub->registerAttribute("Foo", Function::LastWins, 0.0, 1.0, 1.0);
    QCOMPARE(stub->attributes().count(), 2);
    QCOMPARE(stub->getAttributeIndex("Foo"), 1);
    QCOMPARE(stub->getAttributeValue(1), 1.0);
    stub->adjustAttribute(0.7, 1);
    QCOMPARE(stub->getAttributeValue(1), 0.7);

    // check non existent attribute
    QCOMPARE(stub->getAttributeIndex("Bar"), -1);

    stub->resetAttributes();

    QCOMPARE(stub->renameAttribute(1, "Boo"), true);
    QCOMPARE(stub->renameAttribute(5, "Yah"), false);
    QCOMPARE(stub->getAttributeIndex("Boo"), 1);

    stub->unregisterAttribute("Boo");
    QCOMPARE(stub->attributes().count(), 1);

    QCOMPARE(stub->getAttributeValue(1), 0.0);

    int attr = stub->requestAttributeOverride(Function::Intensity, 0.4);
    QCOMPARE(attr, 128);
    QCOMPARE(stub->getAttributeValue(Function::Intensity), 0.2); /* 0.5 * 0.4 */

    stub->adjustAttribute(0.7, attr);
    QCOMPARE(stub->getAttributeValue(Function::Intensity), 0.35);

    stub->releaseAttributeOverride(attr);
    QCOMPARE(stub->getAttributeValue(Function::Intensity), 0.5);

    stub->registerAttribute("Foo", Function::LastWins, 0.0, 1.0, 1.0);
    QCOMPARE(stub->getAttributeValue(1), 1.0);

    attr = stub->requestAttributeOverride(1, 0.3);
    QCOMPARE(attr, 129);
    QCOMPARE(stub->getAttributeValue(1), 0.3); /* Last wins */
}

void Function_Test::blendMode()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);

    QCOMPARE(stub->blendMode(), Universe::NormalBlend);

    stub->setBlendMode(Universe::AdditiveBlend);

    QCOMPARE(stub->blendMode(), Universe::AdditiveBlend);
}

void Function_Test::loaderWrongRoot()
{
    Doc d(this);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Scene");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(Function::loader(xmlReader, &d) == false);
    QVERIFY(d.functions().size() == 0);
}

void Function_Test::loaderWrongID()
{
    Doc d(this);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("ID", QString("%1").arg(Function::invalidId()));

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    QVERIFY(Function::loader(xmlReader, &d) == false);
    QVERIFY(d.functions().size() == 0);

    // reset the data buffer
    buffer.setData(QByteArray());
    buffer.close();

    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWriter.setDevice(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("ID", "-4");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(Function::loader(xmlReader, &d) == false);
    QVERIFY(d.functions().size() == 0);
}

void Function_Test::loaderScene()
{
    Doc d(this);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Scene");
    xmlWriter.writeAttribute("ID", "15");
    xmlWriter.writeAttribute("Name", "Lipton");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Fade");
    xmlWriter.writeCharacters("5");
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

    /* Just verify that a Scene function gets loaded. The rest of Scene
       loading is tested in Scene_test. */
    QVERIFY(Function::loader(xmlReader, &d) == true);
    QVERIFY(d.functions().size() == 1);
    QVERIFY(d.function(15) != NULL);
    QVERIFY(d.function(15)->type() == Function::SceneType);
    QVERIFY(d.function(15)->name() == QString("Lipton"));
}

void Function_Test::loaderChaser()
{
    Doc d(this);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Chaser");
    xmlWriter.writeAttribute("ID", "1");
    xmlWriter.writeAttribute("Name", "Malarkey");

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

    /* Just verify that a Chaser function gets loaded. The rest of Chaser
       loading is tested in Chaser_test. */
    QVERIFY(Function::loader(xmlReader, &d) == true);
    QVERIFY(d.functions().size() == 1);
    QVERIFY(d.function(1) != NULL);
    QVERIFY(d.function(1)->type() == Function::ChaserType);
    QVERIFY(d.function(1)->name() == QString("Malarkey"));
}

void Function_Test::loaderCollection()
{
    Doc d(this);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Collection");
    xmlWriter.writeAttribute("ID","120");
    xmlWriter.writeAttribute("Name", "Spiers");

    xmlWriter.writeTextElement("Step", "87");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    /* Just verify that a Chaser function gets loaded. The rest of Chaser
       loading is tested in Chaser_test. */
    QVERIFY(Function::loader(xmlReader, &d) == true);
    QVERIFY(d.functions().size() == 1);
    QVERIFY(d.function(120) != NULL);
    QVERIFY(d.function(120)->type() == Function::CollectionType);
    QVERIFY(d.function(120)->name() == QString("Spiers"));
}

void Function_Test::loaderEFX()
{
    Doc d(this);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "EFX");
    xmlWriter.writeAttribute("Name", "Guarnere");
    xmlWriter.writeAttribute("ID","0");

    xmlWriter.writeTextElement("PropagationMode", "Serial");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Fade");
    xmlWriter.writeCharacters("12");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Forward");
    xmlWriter.writeTextElement("RunOrder", "Loop");
    xmlWriter.writeTextElement("Algorithm", "Diamond");
    xmlWriter.writeTextElement("Width", "100");
    xmlWriter.writeTextElement("Height", "90");
    xmlWriter.writeTextElement("Rotation", "310");

    xmlWriter.writeStartElement("StartScene");
    xmlWriter.writeAttribute("Enabled", "True");
    xmlWriter.writeCharacters("13");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("StopScene");
    xmlWriter.writeAttribute("Enabled", "True");
    xmlWriter.writeCharacters("77");
    xmlWriter.writeEndElement();

    /* X Axis */
    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("Name", "X");

    xmlWriter.writeTextElement("Offset", "10");
    xmlWriter.writeTextElement("Frequency", "2");
    xmlWriter.writeTextElement("Phase", "270");
    xmlWriter.writeEndElement();

    /* Y Axis */
    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("Name", "Y");

    xmlWriter.writeTextElement("Offset", "20");
    xmlWriter.writeTextElement("Frequency", "3");
    xmlWriter.writeTextElement("Phase", "80");
    xmlWriter.writeEndElement();

    /* Fixture 1 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "33");
    xmlWriter.writeTextElement("Direction", "Forward");
    xmlWriter.writeEndElement();

    /* Fixture 2 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "11");
    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeEndElement();

    /* Fixture 3 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "45");
    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    /* Just verify that a Chaser function gets loaded. The rest of Chaser
       loading is tested in Chaser_test. */
    QVERIFY(Function::loader(xmlReader, &d) == true);
    QVERIFY(d.functions().size() == 1);
    QVERIFY(d.function(0) != NULL);
    QVERIFY(d.function(0)->type() == Function::EFXType);
    QVERIFY(d.function(0)->name() == QString("Guarnere"));
}

void Function_Test::loaderUnknownType()
{
    Doc d(this);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Major");
    xmlWriter.writeAttribute("ID", "15");
    xmlWriter.writeAttribute("Name", "Winters");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    /* Just verify that a Scene function gets loaded. The rest of Scene
       loading is tested in Scene_test. */
    QVERIFY(Function::loader(xmlReader, &d) == false);
    QVERIFY(d.functions().size() == 0);
    QVERIFY(d.function(15) == NULL);
}

void Function_Test::runOrderXML()
{
    Doc d(this);
    Function_Stub stub(&d);
    stub.setRunOrder(Function::SingleShot);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(stub.saveXMLRunOrder(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("RunOrder"));
    QCOMPARE(xmlReader.readElementText(), QString("SingleShot"));

    stub.setRunOrder(Function::Loop);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();
    QVERIFY(stub.loadXMLRunOrder(xmlReader) == true);
    QCOMPARE(stub.runOrder(), Function::SingleShot);

    buffer.close();
    buffer.setData(QByteArray());
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWriter.setDevice(&buffer);

    stub.setRunOrder(Function::Loop);
    QVERIFY(stub.saveXMLRunOrder(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer);

    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("RunOrder"));
    QCOMPARE(xmlReader.readElementText(), QString("Loop"));

    stub.setRunOrder(Function::SingleShot);

    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();
    QVERIFY(stub.loadXMLRunOrder(xmlReader) == true);
    QCOMPARE(stub.runOrder(), Function::Loop);

    buffer.close();
    buffer.setData(QByteArray());
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWriter.setDevice(&buffer);

    stub.setRunOrder(Function::PingPong);
    QVERIFY(stub.saveXMLRunOrder(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer);

    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("RunOrder"));
    QCOMPARE(xmlReader.readElementText(), QString("PingPong"));
    stub.setRunOrder(Function::Loop);

    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();
    QVERIFY(stub.loadXMLRunOrder(xmlReader) == true);
    QCOMPARE(stub.runOrder(), Function::PingPong);

    //QVERIFY(stub.loadXMLRunOrder(root) == false);
}

void Function_Test::directionXML()
{
    Doc d(this);
    Function_Stub stub(&d);
    stub.setDirection(Function::Backward);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(stub.saveXMLDirection(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("Direction"));
    QCOMPARE(xmlReader.readElementText(), QString("Backward"));
    stub.setDirection(Function::Forward);

    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();
    QVERIFY(stub.loadXMLDirection(xmlReader) == true);
    QCOMPARE(stub.direction(), Function::Backward);

    QBuffer buffer2;
    buffer2.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWriter.setDevice(&buffer2);

    stub.setDirection(Function::Forward);
    QVERIFY(stub.saveXMLDirection(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer2.close();

    buffer2.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer2);

    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("Direction"));
    QCOMPARE(xmlReader.readElementText(), QString("Forward"));
    stub.setDirection(Function::Backward);

    buffer2.seek(0);
    xmlReader.setDevice(&buffer2);
    xmlReader.readNextStartElement();
    QVERIFY(stub.loadXMLDirection(xmlReader) == true);
    QCOMPARE(stub.direction(), Function::Forward);

    //QVERIFY(stub.loadXMLDirection(root) == false);
}

void Function_Test::speedXML()
{
    Doc d(this);
    Function_Stub stub(&d);
    stub.setFadeInSpeed(500);
    stub.setFadeOutSpeed(1000);
    stub.setDuration(1500);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(stub.saveXMLSpeed(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Speed"));
    QCOMPARE(xmlReader.attributes().value("FadeIn").toString(), QString("500"));
    QCOMPARE(xmlReader.attributes().value("FadeOut").toString(), QString("1000"));
    QCOMPARE(xmlReader.attributes().value("Duration").toString(), QString("1500"));

    stub.setFadeInSpeed(0);
    stub.setFadeOutSpeed(0);
    stub.setDuration(0);
    QVERIFY(stub.loadXMLSpeed(xmlReader) == true);
    QCOMPARE(stub.fadeInSpeed(), uint(500));
    QCOMPARE(stub.fadeOutSpeed(), uint(1000));
    QCOMPARE(stub.duration(), uint(1500));

    //QVERIFY(stub.loadXMLSpeed(root) == false);
}

QTEST_APPLESS_MAIN(Function_Test)
