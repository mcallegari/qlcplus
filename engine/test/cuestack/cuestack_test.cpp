/*
  Q Light Controller - Unit test
  cuestack_test.cpp

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

#include "cuestack_test.h"

#define private public
#define protected public
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "universe.h"
#include "cuestack.h"
#include "fixture.h"
#include "qlcfile.h"
#include "doc.h"
#undef protected
#undef private

#include "../common/resource_paths.h"

void CueStack_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
}

void CueStack_Test::cleanupTestCase()
{
    delete m_doc;
}

void CueStack_Test::init()
{
    m_doc->clearContents();
}

void CueStack_Test::initial()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.name(), QString());
    QCOMPARE(cs.fadeInSpeed(), uint(0));
    QCOMPARE(cs.fadeOutSpeed(), uint(0));
    QCOMPARE(cs.duration(), uint(UINT_MAX));
    QCOMPARE(cs.cues().size(), 0);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.intensity(), qreal(1.0));
    QCOMPARE(cs.isRunning(), false);
    QCOMPARE(cs.isStarted(), false);
    QCOMPARE(cs.isFlashing(), false);
    QCOMPARE(cs.m_fader, (GenericFader*) NULL);
    QCOMPARE(cs.m_elapsed, uint(0));
    QCOMPARE(cs.m_previous, false);
    QCOMPARE(cs.m_next, false);
    QCOMPARE(cs.doc(), m_doc);
}

void CueStack_Test::name()
{
    CueStack cs(m_doc);
    QSignalSpy spy(&cs, SIGNAL(changed(int)));

    cs.setName("Foo");
    QCOMPARE(cs.name(), QString("Foo"));
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0].size(), 1);
    QCOMPARE(spy[0][0].toInt(), -1);
}

void CueStack_Test::speeds()
{
    CueStack cs(m_doc);

    cs.setFadeInSpeed(100);
    cs.setFadeOutSpeed(200);
    cs.setDuration(300);
    QCOMPARE(cs.fadeInSpeed(), uint(100));
    QCOMPARE(cs.fadeOutSpeed(), uint(200));
    QCOMPARE(cs.duration(), uint(300));
}

void CueStack_Test::appendCue()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.cues().size(), 0);

    cs.appendCue(Cue("One"));
    QCOMPARE(cs.cues().size(), 1);
    QCOMPARE(cs.cues().at(0).name(), QString("One"));

    cs.appendCue(Cue("Two"));
    QCOMPARE(cs.cues().size(), 2);
    QCOMPARE(cs.cues().at(0).name(), QString("One"));
    QCOMPARE(cs.cues().at(1).name(), QString("Two"));

    cs.appendCue(Cue("Three"));
    QCOMPARE(cs.cues().size(), 3);
    QCOMPARE(cs.cues().at(0).name(), QString("One"));
    QCOMPARE(cs.cues().at(1).name(), QString("Two"));
    QCOMPARE(cs.cues().at(2).name(), QString("Three"));

    cs.appendCue(Cue("Four"));
    QCOMPARE(cs.cues().size(), 4);
    QCOMPARE(cs.cues().at(0).name(), QString("One"));
    QCOMPARE(cs.cues().at(1).name(), QString("Two"));
    QCOMPARE(cs.cues().at(2).name(), QString("Three"));
    QCOMPARE(cs.cues().at(3).name(), QString("Four"));
}

void CueStack_Test::insertCue()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.cues().size(), 0);

    QSignalSpy spy(&cs, SIGNAL(currentCueChanged(int)));
    cs.insertCue(0, Cue("One"));
    QCOMPARE(cs.cues().size(), 1);
    QCOMPARE(cs.cues()[0].name(), QString("One"));
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(spy.size(), 0);

    cs.insertCue(0, Cue("Two"));
    QCOMPARE(cs.cues().size(), 2);
    QCOMPARE(cs.cues()[0].name(), QString("Two"));
    QCOMPARE(cs.cues()[1].name(), QString("One"));
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(spy.size(), 0);

    cs.setCurrentIndex(1);
    cs.insertCue(2, Cue("Three"));
    QCOMPARE(cs.cues().size(), 3);
    QCOMPARE(cs.cues()[0].name(), QString("Two"));
    QCOMPARE(cs.cues()[1].name(), QString("One"));
    QCOMPARE(cs.cues()[2].name(), QString("Three"));
    QCOMPARE(cs.currentIndex(), 1);
    QCOMPARE(spy.size(), 0);

    cs.setCurrentIndex(1);
    cs.insertCue(20, Cue("Four"));
    QCOMPARE(cs.cues().size(), 4);
    QCOMPARE(cs.cues()[0].name(), QString("Two"));
    QCOMPARE(cs.cues()[1].name(), QString("One"));
    QCOMPARE(cs.cues()[2].name(), QString("Three"));
    QCOMPARE(cs.cues()[3].name(), QString("Four"));
    QCOMPARE(cs.currentIndex(), 1);
    QCOMPARE(spy.size(), 0);

    cs.setCurrentIndex(1);
    cs.insertCue(1, Cue("Five"));
    QCOMPARE(cs.cues().size(), 5);
    QCOMPARE(cs.cues()[0].name(), QString("Two"));
    QCOMPARE(cs.cues()[1].name(), QString("Five"));
    QCOMPARE(cs.cues()[2].name(), QString("One"));
    QCOMPARE(cs.cues()[3].name(), QString("Three"));
    QCOMPARE(cs.cues()[4].name(), QString("Four"));
    QCOMPARE(cs.currentIndex(), 2);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 2);

    cs.insertCue(-1, Cue("Six"));
    QCOMPARE(cs.cues().size(), 6);
    QCOMPARE(cs.cues()[0].name(), QString("Two"));
    QCOMPARE(cs.cues()[1].name(), QString("Five"));
    QCOMPARE(cs.cues()[2].name(), QString("One"));
    QCOMPARE(cs.cues()[3].name(), QString("Three"));
    QCOMPARE(cs.cues()[4].name(), QString("Four"));
    QCOMPARE(cs.cues()[5].name(), QString("Six"));
    QCOMPARE(cs.currentIndex(), 2);
    QCOMPARE(spy.size(), 1);
}

void CueStack_Test::replaceCue()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.cues().size(), 0);
    cs.appendCue(Cue("One"));
    cs.appendCue(Cue("Two"));
    cs.appendCue(Cue("Three"));
    cs.appendCue(Cue("Four"));
    cs.appendCue(Cue("Five"));
    QCOMPARE(cs.cues().size(), 5);

    cs.replaceCue(0, Cue("Six"));
    QCOMPARE(cs.cues().size(), 5);
    QCOMPARE(cs.cues()[0].name(), QString("Six"));
    QCOMPARE(cs.cues()[1].name(), QString("Two"));
    QCOMPARE(cs.cues()[2].name(), QString("Three"));
    QCOMPARE(cs.cues()[3].name(), QString("Four"));
    QCOMPARE(cs.cues()[4].name(), QString("Five"));

    cs.replaceCue(-1, Cue("Seven"));
    QCOMPARE(cs.cues().size(), 6);
    QCOMPARE(cs.cues()[0].name(), QString("Six"));
    QCOMPARE(cs.cues()[1].name(), QString("Two"));
    QCOMPARE(cs.cues()[2].name(), QString("Three"));
    QCOMPARE(cs.cues()[3].name(), QString("Four"));
    QCOMPARE(cs.cues()[4].name(), QString("Five"));
    QCOMPARE(cs.cues()[5].name(), QString("Seven"));

    cs.replaceCue(20, Cue("Eight"));
    QCOMPARE(cs.cues().size(), 7);
    QCOMPARE(cs.cues()[0].name(), QString("Six"));
    QCOMPARE(cs.cues()[1].name(), QString("Two"));
    QCOMPARE(cs.cues()[2].name(), QString("Three"));
    QCOMPARE(cs.cues()[3].name(), QString("Four"));
    QCOMPARE(cs.cues()[4].name(), QString("Five"));
    QCOMPARE(cs.cues()[5].name(), QString("Seven"));
    QCOMPARE(cs.cues()[6].name(), QString("Eight"));

    cs.replaceCue(5, Cue("Nine"));
    QCOMPARE(cs.cues().size(), 7);
    QCOMPARE(cs.cues()[0].name(), QString("Six"));
    QCOMPARE(cs.cues()[1].name(), QString("Two"));
    QCOMPARE(cs.cues()[2].name(), QString("Three"));
    QCOMPARE(cs.cues()[3].name(), QString("Four"));
    QCOMPARE(cs.cues()[4].name(), QString("Five"));
    QCOMPARE(cs.cues()[5].name(), QString("Nine"));
    QCOMPARE(cs.cues()[6].name(), QString("Eight"));
}

void CueStack_Test::currentIndex()
{
    CueStack cs(m_doc);
    cs.appendCue(Cue("One"));
    cs.appendCue(Cue("Two"));
    cs.appendCue(Cue("Three"));
    cs.appendCue(Cue("Four"));
    cs.appendCue(Cue("Five"));

    cs.setCurrentIndex(-1);
    QCOMPARE(cs.currentIndex(), -1);

    cs.setCurrentIndex(-2);
    QCOMPARE(cs.currentIndex(), -1);

    cs.setCurrentIndex(1);
    QCOMPARE(cs.currentIndex(), 1);

    cs.setCurrentIndex(2);
    QCOMPARE(cs.currentIndex(), 2);

    cs.setCurrentIndex(4);
    QCOMPARE(cs.currentIndex(), 4);

    cs.setCurrentIndex(3);
    QCOMPARE(cs.currentIndex(), 3);

    cs.setCurrentIndex(5);
    QCOMPARE(cs.currentIndex(), 4);

    cs.setCurrentIndex(-1);
    QCOMPARE(cs.currentIndex(), -1);

    cs.nextCue();
    QCOMPARE(cs.m_previous, false);
    QCOMPARE(cs.m_next, true);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.isRunning(), true);
    QCOMPARE(cs.isStarted(), false);

    cs.nextCue();
    QCOMPARE(cs.m_previous, false);
    QCOMPARE(cs.m_next, true);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.isRunning(), true);
    QCOMPARE(cs.isStarted(), false);

    cs.m_running = false;
    cs.m_next = false;

    cs.previousCue();
    QCOMPARE(cs.m_previous, true);
    QCOMPARE(cs.m_next, false);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.isRunning(), true);
    QCOMPARE(cs.isStarted(), false);

    cs.previousCue();
    QCOMPARE(cs.m_previous, true);
    QCOMPARE(cs.m_next, false);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.isRunning(), true);
    QCOMPARE(cs.isStarted(), false);
}

void CueStack_Test::removeCue()
{
    CueStack cs(m_doc);
    cs.appendCue(Cue("One"));
    cs.appendCue(Cue("Two"));
    cs.appendCue(Cue("Three"));
    cs.appendCue(Cue("Four"));
    cs.appendCue(Cue("Five"));
    QCOMPARE(cs.cues().size(), 5);
    cs.setCurrentIndex(4);

    cs.removeCue(-1);
    QCOMPARE(cs.cues().size(), 5);
    QCOMPARE(cs.currentIndex(), 4);

    cs.removeCue(5);
    QCOMPARE(cs.cues().size(), 5);
    QCOMPARE(cs.currentIndex(), 4);

    cs.removeCue(3);
    QCOMPARE(cs.cues().size(), 4);
    QCOMPARE(cs.currentIndex(), 3); // currentIndex-- because a cue before it was removed
    QCOMPARE(cs.cues().at(0).name(), QString("One"));
    QCOMPARE(cs.cues().at(1).name(), QString("Two"));
    QCOMPARE(cs.cues().at(2).name(), QString("Three"));
    QCOMPARE(cs.cues().at(3).name(), QString("Five"));

    cs.removeCue(0);
    QCOMPARE(cs.cues().size(), 3);
    QCOMPARE(cs.currentIndex(), 2); // currentIndex-- because a cue before it was removed
    QCOMPARE(cs.cues().at(0).name(), QString("Two"));
    QCOMPARE(cs.cues().at(1).name(), QString("Three"));
    QCOMPARE(cs.cues().at(2).name(), QString("Five"));

    cs.setCurrentIndex(0);

    cs.removeCue(2);
    QCOMPARE(cs.cues().size(), 2);
    QCOMPARE(cs.currentIndex(), 0); // no change because a cue AFTER it was removed
    QCOMPARE(cs.cues().at(0).name(), QString("Two"));
    QCOMPARE(cs.cues().at(1).name(), QString("Three"));

    cs.removeCue(2);
    QCOMPARE(cs.cues().size(), 2);
    QCOMPARE(cs.currentIndex(), 0); // no change because nothing was removed
    QCOMPARE(cs.cues().at(0).name(), QString("Two"));
    QCOMPARE(cs.cues().at(1).name(), QString("Three"));

    cs.removeCue(0);
    QCOMPARE(cs.cues().size(), 1);
    QCOMPARE(cs.currentIndex(), 0); // currentIndex was removed -> next cue becomes current
    QCOMPARE(cs.cues().at(0).name(), QString("Three"));

    cs.removeCue(0);
    QCOMPARE(cs.cues().size(), 0);

    cs.removeCue(0);
    QCOMPARE(cs.cues().size(), 0);
}

void CueStack_Test::removeCues()
{
    CueStack cs(m_doc);
    cs.appendCue(Cue("One"));
    cs.appendCue(Cue("Two"));
    cs.appendCue(Cue("Three"));
    cs.appendCue(Cue("Four"));
    cs.appendCue(Cue("Five"));
    QCOMPARE(cs.cues().size(), 5);
    cs.setCurrentIndex(4);

    cs.removeCues(QList <int>());
    QCOMPARE(cs.cues().size(), 5);
    QCOMPARE(cs.currentIndex(), 4);

    cs.removeCues(QList <int>() << 5);
    QCOMPARE(cs.cues().size(), 5);
    QCOMPARE(cs.currentIndex(), 4);

    cs.removeCues(QList <int>() << 3);
    QCOMPARE(cs.cues().size(), 4);
    QCOMPARE(cs.currentIndex(), 3); // currentIndex-- because a cue before it was removed
    QCOMPARE(cs.cues().at(0).name(), QString("One"));
    QCOMPARE(cs.cues().at(1).name(), QString("Two"));
    QCOMPARE(cs.cues().at(2).name(), QString("Three"));
    QCOMPARE(cs.cues().at(3).name(), QString("Five"));

    cs.removeCues(QList <int>() << 2 << 0);
    QCOMPARE(cs.cues().size(), 2);
    QCOMPARE(cs.currentIndex(), 1); // currentIndex-- times two because two cue before it were removed
    QCOMPARE(cs.cues().at(0).name(), QString("Two"));
    QCOMPARE(cs.cues().at(1).name(), QString("Five"));

    cs.removeCues(QList <int>() << 0 << 1);
    QCOMPARE(cs.cues().size(), 0);
    QCOMPARE(cs.currentIndex(), 0);

    cs.removeCues(QList <int>() << 0 << 100 << 1000);
    QCOMPARE(cs.cues().size(), 0);
}

void CueStack_Test::load()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("CueStack");
    doc.appendChild(root);

    CueStack cs(m_doc);
    QCOMPARE(CueStack::loadXMLID(root), UINT_MAX);
    root.setAttribute("ID", 15);
    QCOMPARE(CueStack::loadXMLID(root), uint(15));
    QCOMPARE(cs.loadXML(root), true);
    QCOMPARE(cs.cues().size(), 0);

    QDomElement speed = doc.createElement("Speed");
    speed.setAttribute("FadeIn", 100);
    speed.setAttribute("FadeOut", 200);
    speed.setAttribute("Duration", 300);
    root.appendChild(speed);

    QDomElement cue = doc.createElement("Cue");
    cue.setAttribute("Name", "First");
    root.appendChild(cue);

    cue = doc.createElement("Cue");
    cue.setAttribute("Name", "Second");
    root.appendChild(cue);

    cue = doc.createElement("Cue");
    cue.setAttribute("Name", "Third");
    root.appendChild(cue);

    // Extra garbage
    QDomElement foo = doc.createElement("Foo");
    root.appendChild(foo);

    QCOMPARE(CueStack::loadXMLID(root), uint(15));
    QCOMPARE(cs.loadXML(root), true);
    QCOMPARE(cs.cues().size(), 3);
    QCOMPARE(cs.cues().at(0).name(), QString("First"));
    QCOMPARE(cs.cues().at(1).name(), QString("Second"));
    QCOMPARE(cs.cues().at(2).name(), QString("Third"));

    QCOMPARE(CueStack::loadXMLID(foo), UINT_MAX);
    QCOMPARE(cs.loadXML(foo), false);
}

void CueStack_Test::save()
{
    CueStack cs(m_doc);
    cs.appendCue(Cue("One"));
    cs.appendCue(Cue("Two"));
    cs.appendCue(Cue("Three"));
    cs.setFadeInSpeed(200);
    cs.setFadeOutSpeed(300);
    cs.setDuration(400);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    doc.appendChild(root);

    QCOMPARE(cs.saveXML(&doc, &root, 42), true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("CueStack"));

    int cue = 0, speed = 0;

    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Speed")
        {
            speed++;
            QCOMPARE(tag.attribute("FadeIn"), QString("200"));
            QCOMPARE(tag.attribute("FadeOut"), QString("300"));
            QCOMPARE(tag.attribute("Duration"), QString("400"));
        }
        else if (tag.tagName() == "Cue")
        {
            // The contents of a Cue tag are tested in Cue tests
            cue++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(cue, 3);
    QCOMPARE(speed, 1);
}

void CueStack_Test::preRun()
{
    CueStack cs(m_doc);
    QVERIFY(cs.m_fader == NULL);
    QCOMPARE(cs.isStarted(), false);

    cs.m_elapsed = 500;
    QSignalSpy spy(&cs, SIGNAL(started()));
    cs.preRun();
    QVERIFY(cs.m_fader != NULL);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(cs.m_elapsed, uint(0));
    QCOMPARE(cs.m_fader->intensity(), qreal(1.0));
    QCOMPARE(cs.isStarted(), true);

    MasterTimer mt(m_doc);
    cs.postRun(&mt);
}

void CueStack_Test::flash()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.isFlashing(), false);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 0);

    // Disable flashing when it's already disabled -> fail
    cs.setFlashing(false);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 0);
    QCOMPARE(cs.isFlashing(), false);
    cs.setFlashing(false);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 0);
    QCOMPARE(cs.isFlashing(), false);

    // Enable flashing without cues -> fail
    cs.setFlashing(true);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 0);
    QCOMPARE(cs.isFlashing(), false);

    Cue cue;
    cue.setValue(0, 255);
    cue.setValue(128, 42);
    cs.appendCue(cue);

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    ua.at(0)->setChannelCapability(0, QLCChannel::Intensity);
    ua.at(0)->setChannelCapability(128, QLCChannel::Intensity);
    cs.setFlashing(true);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 1);
    QCOMPARE(cs.isFlashing(), true);
    cs.writeDMX(m_doc->masterTimer(), ua);
    QCOMPARE(ua[0]->preGMValues()[0], (char) 255);
    QCOMPARE(ua[0]->preGMValues()[128], (char) 42);
    ua[0]->zeroIntensityChannels();

    cs.setFlashing(true);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 1);
    cs.writeDMX(m_doc->masterTimer(), ua);
    QCOMPARE(ua[0]->preGMValues()[0], (char) 255);
    QCOMPARE(ua[0]->preGMValues()[128], (char) 42);
    ua[0]->zeroIntensityChannels();

    cs.setFlashing(false);
    QCOMPARE(cs.isFlashing(), false);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 0);
    cs.writeDMX(m_doc->masterTimer(), ua);
    QCOMPARE(ua[0]->preGMValues()[0], (char) 0);
    QCOMPARE(ua[0]->preGMValues()[128], (char) 0);
    ua[0]->zeroIntensityChannels();
}

void CueStack_Test::startStop()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.isRunning(), false);
    cs.start();
    QCOMPARE(cs.isRunning(), true);
    cs.stop();
    QCOMPARE(cs.isRunning(), false);
    cs.start();
    QCOMPARE(cs.isRunning(), true);
}

void CueStack_Test::intensity()
{
    CueStack cs(m_doc);

    cs.adjustIntensity(0.5);
    QCOMPARE(cs.intensity(), qreal(0.5));

    cs.preRun();
    QCOMPARE(cs.intensity(), qreal(0.5));
    QCOMPARE(cs.m_fader->intensity(), qreal(0.5));
    MasterTimer mt(m_doc);
    cs.postRun(&mt);
}

void CueStack_Test::nextPrevious()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.previous(), -1);
    QCOMPARE(cs.next(), -1);

    cs.appendCue(Cue("One"));
    QCOMPARE(cs.previous(), 0);
    QCOMPARE(cs.next(), 0);

    cs.appendCue(Cue("Two"));
    QCOMPARE(cs.previous(), 1);
    QCOMPARE(cs.next(), 0);
    QCOMPARE(cs.next(), 1);
    QCOMPARE(cs.next(), 0);
    QCOMPARE(cs.previous(), 1);
    QCOMPARE(cs.previous(), 0);

    cs.appendCue(Cue("Three"));
    QCOMPARE(cs.next(), 1);
    QCOMPARE(cs.next(), 2);
    QCOMPARE(cs.next(), 0);
    QCOMPARE(cs.previous(), 2);
    QCOMPARE(cs.previous(), 1);
    QCOMPARE(cs.previous(), 0);
}

void CueStack_Test::insertStartValue()
{
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    ua.at(0)->setChannelCapability(0, QLCChannel::Pan);
    CueStack cs(m_doc);
    cs.preRun();

    FadeChannel fc;
    fc.setChannel(0);
    fc.setStart(0);
    fc.setTarget(255);
    fc.setCurrent(127);

    cs.m_fader->add(fc);

    fc.setTarget(64);
    cs.insertStartValue(fc, ua);
    QCOMPARE(fc.start(), uchar(127));
    QCOMPARE(fc.current(), uchar(127));

    cs.m_fader->remove(fc);

    // HTP channel in universes
    ua[0]->write(0, 192);
    cs.insertStartValue(fc, ua);
    QCOMPARE(fc.start(), uchar(0));
    QCOMPARE(fc.current(), uchar(0));

    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Scanner");
    fxi->setAddress(0);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);

    // LTP channel (Pan) in universes
    ua[0]->write(0, 192);
    cs.insertStartValue(fc, ua);
    QCOMPARE(fc.start(), uchar(192));
    QCOMPARE(fc.current(), uchar(192));

    MasterTimer mt(m_doc);
    cs.postRun(&mt);
}

void CueStack_Test::switchCue()
{
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Scanner");
    fxi->setAddress(10);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    CueStack cs(m_doc);
    cs.setFadeInSpeed(100);
    cs.setFadeOutSpeed(200);
    cs.setDuration(300);

    Cue cue;
    cue.setName("One");
    cue.setValue(0, 255);
    cue.setValue(1, 255);
    cue.setValue(500, 255);
    cue.setValue(10, 255); // LTP
    cue.setValue(11, 255); // LTP
    cue.setFadeInSpeed(20);
    cue.setFadeOutSpeed(40);
    cs.appendCue(cue);

    cue = Cue();

    cue.setName("Two");
    cue.setValue(500, 255);
    cue.setValue(3, 255);
    cue.setValue(4, 255);
    cue.setValue(11, 255); // LTP
    cue.setFadeInSpeed(60);
    cue.setFadeOutSpeed(80);
    cs.appendCue(cue);

    cs.preRun();

    // Do nothing with invalid cue indices
    cs.switchCue(-1, -1, ua);
    QCOMPARE(cs.m_fader->channels().size(), 0);
    cs.switchCue(-1, 3, ua);
    QCOMPARE(cs.m_fader->channels().size(), 0);

    // Switch to cue one
    cs.switchCue(3, 0, ua);
    QCOMPARE(cs.m_fader->channels().size(), 5);

    FadeChannel fc;
    fc.setChannel(0);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(0));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(20));

    fc.setChannel(1);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(1));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(20));

    fc.setChannel(10);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(10));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(20));

    fc.setChannel(11);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(11));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(20));

    fc.setChannel(500);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(500));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(20));

    fc.setChannel(3);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), QLCChannel::invalid());
    fc.setChannel(4);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), QLCChannel::invalid());

    fc.setChannel(0);
    cs.m_fader->m_channels[fc].setCurrent(127);
    fc.setChannel(1);
    cs.m_fader->m_channels[fc].setCurrent(127);
    fc.setChannel(10);
    cs.m_fader->m_channels[fc].setCurrent(127);
    fc.setChannel(11);
    cs.m_fader->m_channels[fc].setCurrent(127);
    fc.setChannel(500);
    cs.m_fader->m_channels[fc].setCurrent(127);

    // Switch to cue two
    cs.switchCue(0, 1, ua);
    QCOMPARE(cs.m_fader->channels().size(), 7);

    fc.setChannel(0);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(0));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(40));

    fc.setChannel(1);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(1));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(40));

    fc.setChannel(11); // LTP channel also in the next cue
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(11));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(60));

    fc.setChannel(500);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(500));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(60));

    fc.setChannel(3);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(3));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(60));

    fc.setChannel(4);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(4));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), uint(60));

    // Stop
    cs.switchCue(1, -1, ua);
    QCOMPARE(cs.m_fader->channels().size(), 7);

    MasterTimer mt(m_doc);
    cs.postRun(&mt);
}

void CueStack_Test::postRun()
{
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Scanner");
    fxi->setAddress(10);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);

    MasterTimer mt(m_doc);
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    CueStack cs(m_doc);
    cs.setFadeInSpeed(100);
    cs.setFadeOutSpeed(200);
    cs.setDuration(300);

    Cue cue;
    cue.setName("One");
    cue.setValue(0, 255);
    cue.setValue(1, 255);
    cue.setValue(500, 255);
    cue.setValue(10, 255); // LTP
    cue.setValue(11, 255); // LTP
    cs.appendCue(cue);

    cue = Cue();

    cue.setName("Two");
    cue.setValue(500, 255);
    cue.setValue(3, 255);
    cue.setValue(4, 255);
    cue.setValue(11, 255); // LTP
    cs.appendCue(cue);

    cs.preRun();

    // Switch to cue one
    cs.switchCue(-1, 0, ua);
    QCOMPARE(cs.m_fader->channels().size(), 5);

    QSignalSpy cueSpy(&cs, SIGNAL(currentCueChanged(int)));
    QSignalSpy stopSpy(&cs, SIGNAL(stopped()));

    cs.postRun(&mt);
    QCOMPARE(cs.m_fader, (GenericFader*) NULL);
    QCOMPARE(cs.m_currentIndex, -1);
    QCOMPARE(cueSpy.size(), 1);
    QCOMPARE(cueSpy.at(0).size(), 1);
    QCOMPARE(cueSpy.at(0).at(0).toInt(), -1);
    QCOMPARE(stopSpy.size(), 1);

    // Only HTP channels go to MasterTimer's GenericFader
    QCOMPARE(mt.m_fader->channels().size(), 3);
    FadeChannel fc;
    fc.setChannel(0);
    QCOMPARE(mt.m_fader->channels().contains(fc), true);
    fc.setChannel(1);
    QCOMPARE(mt.m_fader->channels().contains(fc), true);
    fc.setChannel(500);
    QCOMPARE(mt.m_fader->channels().contains(fc), true);
}

void CueStack_Test::write()
{
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));

    CueStack cs(m_doc);

    Cue cue("One");
    cue.setValue(0, 255);
    cue.setFadeInSpeed(100);
    cue.setFadeOutSpeed(200);
    cue.setDuration(300);
    cs.appendCue(cue);

    cue = Cue("Two");
    cue.setValue(1, 255);
    cue.setFadeInSpeed(100);
    cue.setFadeOutSpeed(200);
    cue.setDuration(300);
    cs.appendCue(cue);

    cs.preRun();
    QVERIFY(cs.m_fader != NULL);

    cs.write(ua);
    QCOMPARE(cs.currentIndex(), -1);

    cs.start();
    cs.write(ua);
    QCOMPARE(cs.currentIndex(), -1);

    cs.nextCue();
    QCOMPARE(cs.currentIndex(), -1);
    cs.write(ua);
    QCOMPARE(cs.currentIndex(), 0);
    QCOMPARE(cs.m_fader->channels().size(), 1);
    FadeChannel fc;
    fc.setChannel(0);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));

    cs.previousCue();
    QCOMPARE(cs.currentIndex(), 0);
    cs.write(ua);
    QCOMPARE(cs.currentIndex(), 1);

    fc.setChannel(0);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    fc.setChannel(1);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(1));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));

    MasterTimer mt(m_doc);
    cs.postRun(&mt);
}

QTEST_APPLESS_MAIN(CueStack_Test)
