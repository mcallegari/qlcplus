/*
  Q Light Controller Plus - Unit test
  track_test.cpp

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

#include "track_test.h"
#include "sequence.h"
#include "track.h"

void Track_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_showFunc = new ShowFunction(123, this);
}

void Track_Test::cleanupTestCase()
{
    delete m_showFunc;
}

void Track_Test::defaults()
{
    Track t;

    // check defaults
    QVERIFY(t.id() == Track::invalidId());
    QVERIFY(t.getSceneID() == Scene::invalidId());
    QVERIFY(t.showId() == Function::invalidId());
    QCOMPARE(t.name(), "New Track");

    // set & check base params
    t.setId(321);
    t.setShowId(567);
    t.setSceneID(890);
    t.setName("Foo Track");

    QVERIFY(t.id() == 321);
    QVERIFY(t.showId() == 567);
    QVERIFY(t.getSceneID() == 890);
    QCOMPARE(t.name(), "Foo Track");

    Track t2(123);
    QVERIFY(t2.getSceneID() == 123);
}

void Track_Test::mute()
{
    Track t;
    QCOMPARE(t.isMute(), false);

    t.setMute(true);
    QCOMPARE(t.isMute(), true);

    t.setMute(false);
    QCOMPARE(t.isMute(), false);
}

void Track_Test::showFunctions()
{
    Track t;
    QCOMPARE(t.showFunctions().count(), 0);

    QVERIFY(t.createShowFunction(123) != nullptr);
    QCOMPARE(t.showFunctions().count(), 1);

    QVERIFY(t.addShowFunction(nullptr) == false);
    QVERIFY(t.addShowFunction(m_showFunc) == false);
    QCOMPARE(t.showFunctions().count(), 1);

    m_showFunc->setFunctionID(123);
    QVERIFY(t.addShowFunction(m_showFunc) == true);
    QCOMPARE(t.showFunctions().count(), 2);

    QVERIFY(t.removeShowFunction(nullptr) == false);
    QVERIFY(t.removeShowFunction(m_showFunc, true) == true);
    QCOMPARE(t.showFunctions().count(), 1);
}

void Track_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Track");
    xmlWriter.writeAttribute("ID", "123");
    xmlWriter.writeAttribute("SceneID", "456");
    xmlWriter.writeAttribute("Name", "Sequence Cue");
    xmlWriter.writeAttribute("isMute", "1");

    xmlWriter.writeStartElement("ShowFunction");
    xmlWriter.writeAttribute("ID", "789");
    xmlWriter.writeAttribute("Duration", "112233");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Track t;
    QVERIFY(t.loadXML(xmlReader) == true);

    QVERIFY(t.id() == 123);
    QVERIFY(t.getSceneID() == 456);
    QCOMPARE(t.name(), "Sequence Cue");
    QCOMPARE(t.isMute(), true);

    QVERIFY(t.showFunctions().count() == 1);
    ShowFunction *sf = t.showFunctions().first();

    QVERIFY(sf->functionID() == 789);
    QVERIFY(sf->duration() == 112233);
}

void Track_Test::functions()
{
    Track t;
    t.setId(321);
    t.setShowId(567);
    t.setName("Foo Track");

    Scene *s = new Scene(m_doc);
    m_doc->addFunction(s, 10);

    Sequence *sq = new Sequence(m_doc);
    sq->setBoundSceneID(890);
    m_doc->addFunction(sq, 20);

    // invalid ShowFunction
    ShowFunction *sf1 = new ShowFunction(123);
    sf1->setFunctionID(666);

    // Valid ShowFunction
    ShowFunction *sf2 = new ShowFunction(456);
    sf2->setFunctionID(s->id());
    QVERIFY(sf2->color() == QColor());

    ShowFunction *sf3 = new ShowFunction(789);
    sf3->setFunctionID(sq->id());

    t.addShowFunction(sf1);
    t.addShowFunction(sf2);
    t.addShowFunction(sf3);

    QVERIFY(t.showFunctions().count() == 3);
    QVERIFY(t.showFunction(111) == NULL);
    QVERIFY(t.showFunction(123) == sf1);
    QVERIFY(t.showFunction(456) == sf2);
    QVERIFY(t.showFunction(789) == sf3);
    QVERIFY(t.postLoad(m_doc) == true);

    // invalid ShowFunction has been removed
    QVERIFY(t.showFunctions().count() == 2);
    // invalid color has been fixed
    QVERIFY(sf2->color() == QColor(100, 100, 100));
    // check SceneID set from sequence
    //QVERIFY(t.getSceneID() == 890);

    //QVERIFY(t.contains(m_doc, 890) == true);
    QVERIFY(t.contains(m_doc, 666) == false);
    QVERIFY(t.contains(m_doc, 10) == true);
    QVERIFY(t.contains(m_doc, 20) == true);

    QVERIFY(t.components().count() == 2);
    QVERIFY(t.components().at(0) == 10);
    QVERIFY(t.components().at(1) == 20);
}

void Track_Test::save()
{
    Track t(321);
    t.setId(654);
    t.setName("Audio Cue");
    t.setMute(true);

    m_showFunc = new ShowFunction(123, this);
    m_showFunc->setFunctionID(987);

    t.addShowFunction(m_showFunc);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(t.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "Track");

    QVERIFY(xmlReader.attributes().value("ID").toString() == "654");
    QVERIFY(xmlReader.attributes().value("SceneID").toString() == "321");
    QVERIFY(xmlReader.attributes().value("Name").toString() == "Audio Cue");
    QVERIFY(xmlReader.attributes().value("isMute").toString() == "1");

    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "ShowFunction");
    QVERIFY(xmlReader.attributes().value("ID").toString() == "987");
}


QTEST_APPLESS_MAIN(Track_Test)
