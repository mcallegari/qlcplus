/*
  Q Light Controller - Unit test
  function_test.cpp

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
#include <QtXml>

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
    QCOMPARE(stub->name(), QString());
    QCOMPARE(stub->runOrder(), Function::Loop);
    QCOMPARE(stub->direction(), Function::Forward);
    QCOMPARE(stub->elapsed(), quint32(0));
    QCOMPARE(stub->stopped(), true);
    QCOMPARE(stub->fadeInSpeed(), uint(0));
    QCOMPARE(stub->fadeOutSpeed(), uint(0));
    QCOMPARE(stub->duration(), uint(0));
}

void Function_Test::properties()
{
    Doc doc(this);

    Function_Stub* stub = new Function_Stub(&doc);
    doc.addFunction(stub);

    QSignalSpy spy(stub, SIGNAL(changed(quint32)));

    stub->setName("Test");
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0][0].toUInt(), stub->id());
    QCOMPARE(stub->name(), QString("Test"));

    stub->setRunOrder(Function::PingPong);
    QCOMPARE(spy.size(), 2);
    QCOMPARE(spy[1][0].toUInt(), stub->id());
    QCOMPARE(stub->runOrder(), Function::PingPong);

    stub->setDirection(Function::Backward);
    QCOMPARE(spy.size(), 3);
    QCOMPARE(spy[2][0].toUInt(), stub->id());
    QCOMPARE(stub->direction(), Function::Backward);

    stub->setFadeInSpeed(14);
    QCOMPARE(spy.size(), 4);
    QCOMPARE(spy[3][0].toUInt(), stub->id());
    QCOMPARE(stub->fadeInSpeed(), uint(14));

    stub->setFadeOutSpeed(42);
    QCOMPARE(spy.size(), 5);
    QCOMPARE(spy[4][0].toUInt(), stub->id());
    QCOMPARE(stub->fadeOutSpeed(), uint(42));

    stub->setDuration(69);
    QCOMPARE(spy.size(), 6);
    QCOMPARE(spy[5][0].toUInt(), stub->id());
    QCOMPARE(stub->duration(), uint(69));
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
    stub->flash(NULL);
    QCOMPARE(spy.size(), 1);
    QVERIFY(stub->flashing() == true);
    stub->flash(NULL);
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
    stub->postRun(NULL, NULL);
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
    QCOMPARE(stub->getAttributeValue(Function::Intensity), qreal(1.0));
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
    stub->m_type = Function::Scene;
    QCOMPARE(stub->typeString(), Function::typeToString(Function::Scene));
    stub->m_type = Function::Chaser;
    QCOMPARE(stub->typeString(), Function::typeToString(Function::Chaser));
}

void Function_Test::typeToString()
{
    QVERIFY(Function::typeToString(Function::Undefined) == "Undefined");
    QVERIFY(Function::typeToString(Function::Scene) == "Scene");
    QVERIFY(Function::typeToString(Function::Chaser) == "Chaser");
    QVERIFY(Function::typeToString(Function::EFX) == "EFX");
    QVERIFY(Function::typeToString(Function::Collection) == "Collection");

    QVERIFY(Function::typeToString(Function::Type(42)) == "Undefined");
    QVERIFY(Function::typeToString(Function::Type(31337)) == "Undefined");
}

void Function_Test::stringToType()
{
    QVERIFY(Function::stringToType("Undefined") == Function::Undefined);
    QVERIFY(Function::stringToType("Scene") == Function::Scene);
    QVERIFY(Function::stringToType("Chaser") == Function::Chaser);
    QVERIFY(Function::stringToType("EFX") == Function::EFX);
    QVERIFY(Function::stringToType("Collection") == Function::Collection);

    QVERIFY(Function::stringToType("Foobar") == Function::Undefined);
    QVERIFY(Function::stringToType("Xyzzy") == Function::Undefined);
}

void Function_Test::runOrderToString()
{
    QVERIFY(Function::runOrderToString(Function::Loop) == "Loop");
    QVERIFY(Function::runOrderToString(Function::SingleShot) == "SingleShot");
    QVERIFY(Function::runOrderToString(Function::PingPong) == "PingPong");

    QVERIFY(Function::runOrderToString(Function::RunOrder(42)) == "Loop");
    QVERIFY(Function::runOrderToString(Function::RunOrder(69)) == "Loop");
}

void Function_Test::stringToRunOrder()
{
    QVERIFY(Function::stringToRunOrder("Loop") == Function::Loop);
    QVERIFY(Function::stringToRunOrder("SingleShot") == Function::SingleShot);
    QVERIFY(Function::stringToRunOrder("PingPong") == Function::PingPong);

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

void Function_Test::loaderWrongRoot()
{
    Doc d(this);

    QDomDocument doc;
    QDomElement root = doc.createElement("Scene");

    QVERIFY(Function::loader(root, &d) == false);
    QVERIFY(d.functions().size() == 0);
}

void Function_Test::loaderWrongID()
{
    Doc d(this);

    QDomDocument doc;
    QDomElement root = doc.createElement("Function");
    root.setAttribute("ID", QString("%1").arg(Function::invalidId()));

    QVERIFY(Function::loader(root, &d) == false);
    QVERIFY(d.functions().size() == 0);

    root.setAttribute("ID", "-4");
    QVERIFY(Function::loader(root, &d) == false);
    QVERIFY(d.functions().size() == 0);
}

void Function_Test::loaderScene()
{
    Doc d(this);

    QDomDocument doc;
    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Scene");
    root.setAttribute("ID", 15);
    root.setAttribute("Name", "Lipton");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Fade");
    QDomText busText = doc.createTextNode("5");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement v2 = doc.createElement("Value");
    v2.setAttribute("Fixture", 133);
    v2.setAttribute("Channel", 4);
    QDomText v2Text = doc.createTextNode("59");
    v2.appendChild(v2Text);
    root.appendChild(v2);

    /* Just verify that a Scene function gets loaded. The rest of Scene
       loading is tested in Scene_test. */
    QVERIFY(Function::loader(root, &d) == true);
    QVERIFY(d.functions().size() == 1);
    QVERIFY(d.function(15) != NULL);
    QVERIFY(d.function(15)->type() == Function::Scene);
    QVERIFY(d.function(15)->name() == QString("Lipton"));
}

void Function_Test::loaderChaser()
{
    Doc d(this);

    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Chaser");
    root.setAttribute("ID", 1);
    root.setAttribute("Name", "Malarkey");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Hold");
    QDomText busText = doc.createTextNode("16");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement run = doc.createElement("RunOrder");
    QDomText runText = doc.createTextNode("SingleShot");
    run.appendChild(runText);
    root.appendChild(run);

    QDomElement s1 = doc.createElement("Step");
    s1.setAttribute("Number", 1);
    QDomText s1Text = doc.createTextNode("50");
    s1.appendChild(s1Text);
    root.appendChild(s1);

    QDomElement s2 = doc.createElement("Step");
    s2.setAttribute("Number", 2);
    QDomText s2Text = doc.createTextNode("12");
    s2.appendChild(s2Text);
    root.appendChild(s2);

    QDomElement s3 = doc.createElement("Step");
    s3.setAttribute("Number", 0);
    QDomText s3Text = doc.createTextNode("87");
    s3.appendChild(s3Text);
    root.appendChild(s3);

    /* Just verify that a Chaser function gets loaded. The rest of Chaser
       loading is tested in Chaser_test. */
    QVERIFY(Function::loader(root, &d) == true);
    QVERIFY(d.functions().size() == 1);
    QVERIFY(d.function(1) != NULL);
    QVERIFY(d.function(1)->type() == Function::Chaser);
    QVERIFY(d.function(1)->name() == QString("Malarkey"));
}

void Function_Test::loaderCollection()
{
    Doc d(this);

    QDomDocument doc;
    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Collection");
    root.setAttribute("ID", "120");
    root.setAttribute("Name", "Spiers");

    QDomElement s3 = doc.createElement("Step");
    QDomText s3Text = doc.createTextNode("87");
    s3.appendChild(s3Text);
    root.appendChild(s3);

    /* Just verify that a Chaser function gets loaded. The rest of Chaser
       loading is tested in Chaser_test. */
    QVERIFY(Function::loader(root, &d) == true);
    QVERIFY(d.functions().size() == 1);
    QVERIFY(d.function(120) != NULL);
    QVERIFY(d.function(120)->type() == Function::Collection);
    QVERIFY(d.function(120)->name() == QString("Spiers"));
}

void Function_Test::loaderEFX()
{
    Doc d(this);

    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "EFX");
    root.setAttribute("Name", "Guarnere");
    root.setAttribute("ID", "0");

    QDomElement prop = doc.createElement("PropagationMode");
    QDomText propText = doc.createTextNode("Serial");
    prop.appendChild(propText);
    root.appendChild(prop);

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Fade");
    QDomText busText = doc.createTextNode("12");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Forward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement run = doc.createElement("RunOrder");
    QDomText runText = doc.createTextNode("Loop");
    run.appendChild(runText);
    root.appendChild(run);

    QDomElement algo = doc.createElement("Algorithm");
    QDomText algoText = doc.createTextNode("Diamond");
    algo.appendChild(algoText);
    root.appendChild(algo);

    QDomElement w = doc.createElement("Width");
    QDomText wText = doc.createTextNode("100");
    w.appendChild(wText);
    root.appendChild(w);

    QDomElement h = doc.createElement("Height");
    QDomText hText = doc.createTextNode("90");
    h.appendChild(hText);
    root.appendChild(h);

    QDomElement rot = doc.createElement("Rotation");
    QDomText rotText = doc.createTextNode("310");
    rot.appendChild(rotText);
    root.appendChild(rot);

    QDomElement stas = doc.createElement("StartScene");
    stas.setAttribute("Enabled", "True");
    QDomText stasText = doc.createTextNode("13");
    stas.appendChild(stasText);
    root.appendChild(stas);

    QDomElement stos = doc.createElement("StopScene");
    stos.setAttribute("Enabled", "True");
    QDomText stosText = doc.createTextNode("77");
    stos.appendChild(stosText);
    root.appendChild(stos);

    /* X Axis */
    QDomElement xax = doc.createElement("Axis");
    xax.setAttribute("Name", "X");
    root.appendChild(xax);

    QDomElement xoff = doc.createElement("Offset");
    QDomText xoffText = doc.createTextNode("10");
    xoff.appendChild(xoffText);
    xax.appendChild(xoff);

    QDomElement xfreq = doc.createElement("Frequency");
    QDomText xfreqText = doc.createTextNode("2");
    xfreq.appendChild(xfreqText);
    xax.appendChild(xfreq);

    QDomElement xpha = doc.createElement("Phase");
    QDomText xphaText = doc.createTextNode("270");
    xpha.appendChild(xphaText);
    xax.appendChild(xpha);

    /* Y Axis */
    QDomElement yax = doc.createElement("Axis");
    yax.setAttribute("Name", "Y");
    root.appendChild(yax);

    QDomElement yoff = doc.createElement("Offset");
    QDomText yoffText = doc.createTextNode("20");
    yoff.appendChild(yoffText);
    yax.appendChild(yoff);

    QDomElement yfreq = doc.createElement("Frequency");
    QDomText yfreqText = doc.createTextNode("3");
    yfreq.appendChild(yfreqText);
    yax.appendChild(yfreq);

    QDomElement ypha = doc.createElement("Phase");
    QDomText yphaText = doc.createTextNode("80");
    ypha.appendChild(yphaText);
    yax.appendChild(ypha);

    /* Fixture 1 */
    QDomElement ef1 = doc.createElement("Fixture");
    root.appendChild(ef1);

    QDomElement ef1ID = doc.createElement("ID");
    QDomText ef1IDText = doc.createTextNode("33");
    ef1ID.appendChild(ef1IDText);
    ef1.appendChild(ef1ID);

    QDomElement ef1dir = doc.createElement("Direction");
    QDomText ef1dirText = doc.createTextNode("Forward");
    ef1dir.appendChild(ef1dirText);
    ef1.appendChild(ef1dir);

    /* Fixture 2 */
    QDomElement ef2 = doc.createElement("Fixture");
    root.appendChild(ef2);

    QDomElement ef2ID = doc.createElement("ID");
    QDomText ef2IDText = doc.createTextNode("11");
    ef2ID.appendChild(ef2IDText);
    ef2.appendChild(ef2ID);

    QDomElement ef2dir = doc.createElement("Direction");
    QDomText ef2dirText = doc.createTextNode("Backward");
    ef2dir.appendChild(ef2dirText);
    ef2.appendChild(ef2dir);

    /* Fixture 3 */
    QDomElement ef3 = doc.createElement("Fixture");
    root.appendChild(ef3);

    QDomElement ef3ID = doc.createElement("ID");
    QDomText ef3IDText = doc.createTextNode("45");
    ef3ID.appendChild(ef3IDText);
    ef3.appendChild(ef3ID);

    QDomElement ef3dir = doc.createElement("Direction");
    QDomText ef3dirText = doc.createTextNode("Backward");
    ef3dir.appendChild(ef3dirText);
    ef3.appendChild(ef3dir);

    /* Just verify that a Chaser function gets loaded. The rest of Chaser
       loading is tested in Chaser_test. */
    QVERIFY(Function::loader(root, &d) == true);
    QVERIFY(d.functions().size() == 1);
    QVERIFY(d.function(0) != NULL);
    QVERIFY(d.function(0)->type() == Function::EFX);
    QVERIFY(d.function(0)->name() == QString("Guarnere"));
}

void Function_Test::loaderUnknownType()
{
    Doc d(this);

    QDomDocument doc;
    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Major");
    root.setAttribute("ID", 15);
    root.setAttribute("Name", "Winters");

    /* Just verify that a Scene function gets loaded. The rest of Scene
       loading is tested in Scene_test. */
    QVERIFY(Function::loader(root, &d) == false);
    QVERIFY(d.functions().size() == 0);
    QVERIFY(d.function(15) == NULL);
}

void Function_Test::runOrderXML()
{
    Doc d(this);
    Function_Stub stub(&d);
    stub.setRunOrder(Function::SingleShot);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    QVERIFY(stub.saveXMLRunOrder(&doc, &root) == true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("RunOrder"));
    QCOMPARE(root.firstChild().toElement().text(), QString("SingleShot"));
    stub.setRunOrder(Function::Loop);
    QVERIFY(stub.loadXMLRunOrder(root.firstChild().toElement()) == true);
    QCOMPARE(stub.runOrder(), Function::SingleShot);

    root = doc.createElement("Foo");
    stub.setRunOrder(Function::Loop);
    QVERIFY(stub.saveXMLRunOrder(&doc, &root) == true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("RunOrder"));
    QCOMPARE(root.firstChild().toElement().text(), QString("Loop"));
    stub.setRunOrder(Function::SingleShot);
    QVERIFY(stub.loadXMLRunOrder(root.firstChild().toElement()) == true);
    QCOMPARE(stub.runOrder(), Function::Loop);

    root = doc.createElement("Foo");
    stub.setRunOrder(Function::PingPong);
    QVERIFY(stub.saveXMLRunOrder(&doc, &root) == true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("RunOrder"));
    QCOMPARE(root.firstChild().toElement().text(), QString("PingPong"));
    stub.setRunOrder(Function::Loop);
    QVERIFY(stub.loadXMLRunOrder(root.firstChild().toElement()) == true);
    QCOMPARE(stub.runOrder(), Function::PingPong);

    QVERIFY(stub.loadXMLRunOrder(root) == false);
}

void Function_Test::directionXML()
{
    Doc d(this);
    Function_Stub stub(&d);
    stub.setDirection(Function::Backward);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    QVERIFY(stub.saveXMLDirection(&doc, &root) == true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("Direction"));
    QCOMPARE(root.firstChild().toElement().text(), QString("Backward"));
    stub.setDirection(Function::Forward);
    QVERIFY(stub.loadXMLDirection(root.firstChild().toElement()) == true);
    QCOMPARE(stub.direction(), Function::Backward);

    root = doc.createElement("Foo");
    stub.setDirection(Function::Forward);
    QVERIFY(stub.saveXMLDirection(&doc, &root) == true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("Direction"));
    QCOMPARE(root.firstChild().toElement().text(), QString("Forward"));
    stub.setDirection(Function::Backward);
    QVERIFY(stub.loadXMLDirection(root.firstChild().toElement()) == true);
    QCOMPARE(stub.direction(), Function::Forward);

    QVERIFY(stub.loadXMLDirection(root) == false);
}

void Function_Test::speedXML()
{
    Doc d(this);
    Function_Stub stub(&d);
    stub.setFadeInSpeed(500);
    stub.setFadeOutSpeed(1000);
    stub.setDuration(1500);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    QVERIFY(stub.saveXMLSpeed(&doc, &root) == true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("Speed"));
    QCOMPARE(root.firstChild().toElement().attribute("FadeIn"), QString("500"));
    QCOMPARE(root.firstChild().toElement().attribute("FadeOut"), QString("1000"));
    QCOMPARE(root.firstChild().toElement().attribute("Duration"), QString("1500"));

    stub.setFadeInSpeed(0);
    stub.setFadeOutSpeed(0);
    stub.setDuration(0);
    QVERIFY(stub.loadXMLSpeed(root.firstChild().toElement()) == true);
    QCOMPARE(stub.fadeInSpeed(), uint(500));
    QCOMPARE(stub.fadeOutSpeed(), uint(1000));
    QCOMPARE(stub.duration(), uint(1500));

    QVERIFY(stub.loadXMLSpeed(root) == false);
}

QTEST_APPLESS_MAIN(Function_Test)
