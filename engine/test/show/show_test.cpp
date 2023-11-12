/*
  Q Light Controller Plus - Unit test
  show_test.cpp

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

#include "show_test.h"
#include "show.h"

void Show_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void Show_Test::cleanupTestCase()
{
    delete m_doc;
}

void Show_Test::defaults()
{
    Show s(m_doc);

    // check defaults
    QCOMPARE(s.type(), Function::ShowType);
    QCOMPARE(s.id(), Function::invalidId());
    QCOMPARE(s.name(), "New Show");
    QCOMPARE(s.attributes().count(), 0);
}

void Show_Test::timeDivision()
{
    Show s(m_doc);
    QCOMPARE(s.getTimeDivisionType(), Show::Time);
    QCOMPARE(s.getTimeDivisionBPM(), 120);

    s.setTimeDivision(Show::BPM_4_4, 111);
    QCOMPARE(s.getTimeDivisionType(), Show::BPM_4_4);
    QCOMPARE(s.getTimeDivisionBPM(), 111);

    QCOMPARE(s.stringToTempo("Time"), Show::Time);
    QCOMPARE(s.stringToTempo("BPM_4_4"), Show::BPM_4_4);
    QCOMPARE(s.stringToTempo("BPM_3_4"), Show::BPM_3_4);
    QCOMPARE(s.stringToTempo("BPM_2_4"), Show::BPM_2_4);

    QCOMPARE(s.tempoToString(Show::Time), "Time");
    QCOMPARE(s.tempoToString(Show::BPM_4_4), "BPM_4_4");
    QCOMPARE(s.tempoToString(Show::BPM_3_4), "BPM_3_4");
    QCOMPARE(s.tempoToString(Show::BPM_2_4), "BPM_2_4");
}

void Show_Test::tracks()
{
    Show s(m_doc);
    s.setID(123);

    QCOMPARE(s.tracks().count(), 0);

    Track *t = new Track(123);
    t->setName("First track");

    Track *t2 = new Track(321);
    t2->setName("Second track");

    QVERIFY(s.addTrack(t) == true);
    QCOMPARE(s.getTracksCount(), 1);
    QCOMPARE(s.tracks().count(), 1);

    QVERIFY(s.track(456) == NULL);
    QVERIFY(s.getTrackFromSceneID(456) == NULL);

    QVERIFY(s.track(0) == t);
    QVERIFY(s.getTrackFromSceneID(123) == t);

    // check sutomatic ID assignment
    QVERIFY(t->id() == 0);
    QVERIFY(t->showId() == 123);

    // check automatic attribute registration
    QCOMPARE(s.attributes().count(), 1);
    QVERIFY(s.attributes().at(0).m_name == "First track");

    // add a second track and move it up
    QVERIFY(s.addTrack(t2) == true);
    QCOMPARE(s.getTracksCount(), 2);

    s.moveTrack(t2, -1);
    QVERIFY(s.tracks().at(0)->name() == "Second track");
    QVERIFY(s.tracks().at(1)->name() == "First track");

    // no change
    s.moveTrack(t2, -1);
    QVERIFY(s.tracks().at(0)->name() == "Second track");
    QVERIFY(s.tracks().at(1)->name() == "First track");

    // move back as original
    s.moveTrack(t, -1);
    QVERIFY(s.tracks().at(0)->name() == "First track");
    QVERIFY(s.tracks().at(1)->name() == "Second track");

    // check invalid track removal
    QVERIFY(s.removeTrack(456) == false);
    QCOMPARE(s.tracks().count(), 2);

    // check valid track removal
    QVERIFY(s.removeTrack(0) == true);
    QCOMPARE(s.tracks().count(), 1);
    QCOMPARE(s.attributes().count(), 1);

    QVERIFY(s.removeTrack(1) == true);
    QCOMPARE(s.tracks().count(), 0);
    QCOMPARE(s.attributes().count(), 0);
}

void Show_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Show");

    xmlWriter.writeStartElement("TimeDivision");
    xmlWriter.writeAttribute("Type", "BPM_2_4");
    xmlWriter.writeAttribute("BPM", "222");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Track");
    xmlWriter.writeAttribute("ID", "0");
    xmlWriter.writeAttribute("SceneID", "111");
    xmlWriter.writeAttribute("Name", "Read track 1");
    xmlWriter.writeAttribute("isMute", "0");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Track");
    xmlWriter.writeAttribute("ID", "1");
    xmlWriter.writeAttribute("SceneID", "222");
    xmlWriter.writeAttribute("Name", "Read track 2");
    xmlWriter.writeAttribute("isMute", "1");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Show s(m_doc);
    QVERIFY(s.loadXML(xmlReader) == true);

    QCOMPARE(s.getTimeDivisionType(), Show::BPM_2_4);
    QCOMPARE(s.getTimeDivisionBPM(), 222);

    QCOMPARE(s.getTracksCount(), 2);

    Track *t, *t2;
    t = s.track(111);
    QVERIFY(t == NULL);

    t = s.track(0);
    t2 = s.track(1);

    QCOMPARE(t->name(), "Read track 1");
    QVERIFY(t->getSceneID() == 111);
    QCOMPARE(t->isMute(), false);

    QCOMPARE(t2->name(), "Read track 2");
    QVERIFY(t2->getSceneID() == 222);
    QCOMPARE(t2->isMute(), true);
}

void Show_Test::save()
{
    Show s(m_doc);
    s.setID(123);
    s.setName("Test Show");
    s.setTimeDivision(Show::BPM_3_4, 111);

    Track *t = new Track(456);
    t->setName("First track");

    Track *t2 = new Track(789);
    t2->setName("Second track");
    t2->setMute(true);

    s.addTrack(t);
    s.addTrack(t2);

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
    QVERIFY(xmlReader.attributes().value("Type").toString() == "Show");
    QVERIFY(xmlReader.attributes().value("ID").toString() == "123");
    QVERIFY(xmlReader.attributes().value("Name").toString() == "Test Show");

    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "TimeDivision");
    QVERIFY(xmlReader.attributes().value("Type").toString() == "BPM_3_4");
    QVERIFY(xmlReader.attributes().value("BPM").toString() == "111");
    xmlReader.skipCurrentElement();

    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "Track");
    QVERIFY(xmlReader.attributes().value("ID").toString() == "0");
    QVERIFY(xmlReader.attributes().value("SceneID").toString() == "456");
    QVERIFY(xmlReader.attributes().value("Name").toString() == "First track");
    QVERIFY(xmlReader.attributes().value("isMute").toString() == "0");
    xmlReader.skipCurrentElement();

    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "Track");
    QVERIFY(xmlReader.attributes().value("ID").toString() == "1");
    QVERIFY(xmlReader.attributes().value("SceneID").toString() == "789");
    QVERIFY(xmlReader.attributes().value("Name").toString() == "Second track");
    QVERIFY(xmlReader.attributes().value("isMute").toString() == "1");
}


QTEST_APPLESS_MAIN(Show_Test)
