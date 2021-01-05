/*
  Q Light Controller Plus - Unit tests
  chaserstep_test.cpp

  Copyright (C) Heikki Junnila
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

#include "chaserstep_test.h"
#include "chaserstep.h"
#include "scene.h"
#include "doc.h"

void ChaserStep_Test::initial()
{
    ChaserStep step;
    QCOMPARE(step.fid, Function::invalidId());
    QCOMPARE(step.fadeIn, uint(0));
    QCOMPARE(step.fadeOut, uint(0));
    QCOMPARE(step.duration, uint(0));
    QCOMPARE(step.note, QString());

    step = ChaserStep(1, 2, 3, 4);
    QCOMPARE(step.fid, quint32(1));
    QCOMPARE(step.fadeIn, uint(2));
    QCOMPARE(step.hold, uint(3));
    QCOMPARE(step.fadeOut, uint(4));
    QCOMPARE(step.duration, uint(5));
    QCOMPARE(step.note, QString());
}

void ChaserStep_Test::comparison()
{
    ChaserStep step1(0, 1, 2, 3);
    ChaserStep step2(0, 1, 2, 3);
    QVERIFY(step1 == step2);

    step2.fid = 1;
    QVERIFY((step1 == step2) == false);
}

void ChaserStep_Test::resolveFunction()
{
    ChaserStep step(0);
    QVERIFY(step.resolveFunction(NULL) == NULL);

    Doc doc(this);
    QVERIFY(step.resolveFunction(&doc) == NULL);

    Scene* scene = new Scene(&doc);
    doc.addFunction(scene);
    QVERIFY(step.resolveFunction(&doc) == scene);
}

void ChaserStep_Test::variant()
{
    ChaserStep step(1, 2, 3, 4);
    QVariant var = step.toVariant();
    QList <QVariant> list(var.toList());
    QCOMPARE(list.size(), 6);
    QCOMPARE(list[0].toUInt(), uint(1));
    QCOMPARE(list[1].toUInt(), uint(2));
    QCOMPARE(list[2].toUInt(), uint(3));
    QCOMPARE(list[3].toUInt(), uint(4));
    QCOMPARE(list[4].toUInt(), uint(5));

    ChaserStep pets = ChaserStep::fromVariant(var);
    QCOMPARE(pets.fid, uint(1));
    QCOMPARE(pets.fadeIn, uint(2));
    QCOMPARE(pets.hold, uint(3));
    QCOMPARE(pets.fadeOut, uint(4));
    QCOMPARE(pets.duration, uint(5));
}

#include <QDebug>

void ChaserStep_Test::values()
{
    ChaserStep step(1, 2, 3, 4);
    QVERIFY(step.values.count() == 0);

    /* append a new value */
    QVERIFY(step.setValue(SceneValue(5, 6, 7)) == 0);
    QVERIFY(step.values.count() == 1);

    /* insert a new value at position */
    QVERIFY(step.setValue(SceneValue(5, 7, 8), 0) == 0);
    QVERIFY(step.values.count() == 2);
    QVERIFY(step.values.at(0) == SceneValue(5, 7, 8));
    QVERIFY(step.values.at(1) == SceneValue(5, 6, 7));

    /* insert a new value in the middle */
    QVERIFY(step.setValue(SceneValue(5, 8, 9), 1) == 1);
    QVERIFY(step.values.count() == 3);
    QVERIFY(step.values.at(0) == SceneValue(5, 7, 8));
    QVERIFY(step.values.at(1) == SceneValue(5, 8, 9));
    QVERIFY(step.values.at(2) == SceneValue(5, 6, 7));

    /* replace an existing value */
    QVERIFY(step.setValue(SceneValue(5, 8, 19), 1) == 1);
    QVERIFY(step.values.count() == 3);
    QVERIFY(step.values.at(1) == SceneValue(5, 8, 19));

    bool created = false;
    /* check created on new value */
    QVERIFY(step.setValue(SceneValue(6, 7, 8), -1, &created) == 3);
    QVERIFY(created == true);
    QVERIFY(step.values.count() == 4);

    /* check that the previous operation sorted the values */
    QVERIFY(step.values.at(0) == SceneValue(5, 6, 7));
    QVERIFY(step.values.at(1) == SceneValue(5, 7, 8));
    QVERIFY(step.values.at(2) == SceneValue(5, 8, 19));
    QVERIFY(step.values.at(3) == SceneValue(6, 7, 8));

    /* check created on existing value */
    QVERIFY(step.setValue(SceneValue(5, 8, 9), 2, &created) == 2);
    QVERIFY(created == false);
    QVERIFY(step.values.at(2) == SceneValue(5, 8, 9));
    QVERIFY(step.values.count() == 4);

    /* check creating an invalid value */
    QVERIFY(step.setValue(SceneValue(7, 8, 9), 42, &created) == -1);
    QVERIFY(created == false);

    /* check new value append */
    QVERIFY(step.setValue(SceneValue(7, 8, 9), 4, &created) == 4);
    QVERIFY(created == true);
    QVERIFY(step.values.count() == 5);

    /* unset an invalid value */
    QVERIFY(step.unSetValue(SceneValue(10, 20, 30)) == -1);

    /* unset a valid value */
    QVERIFY(step.unSetValue(SceneValue(5, 8, 9), 1) == 1);
    QVERIFY(step.values.count() == 4);
}

void ChaserStep_Test::load()
{
    int number = -1;
    ChaserStep step;

    QBuffer buffer;
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    QVERIFY(step.loadXML(xmlReader, number, NULL) == false);
    QCOMPARE(number, -1);

    buffer.close();
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "5");
    xmlWriter.writeAttribute("FadeIn", "10");
    xmlWriter.writeAttribute("Hold", "15");
    xmlWriter.writeAttribute("FadeOut", "20");
    xmlWriter.writeAttribute("Note", "Fortytwo");
    xmlWriter.writeCharacters("30");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(step.loadXML(xmlReader, number, NULL) == true);
    QCOMPARE(number, 5);
    QCOMPARE(step.fadeIn, uint(10));
    QCOMPARE(step.hold, uint(15));
    QCOMPARE(step.fadeOut, uint(20));
    QCOMPARE(step.duration, uint(25));
    QCOMPARE(step.fid, quint32(30));
    QCOMPARE(step.note, QString("Fortytwo"));
}

void ChaserStep_Test::load_sequence()
{
    int number = -1;
    ChaserStep step;

    QBuffer buffer;
    QXmlStreamReader xmlReader(&buffer);
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeAttribute("FadeIn", "10");
    xmlWriter.writeAttribute("Hold", "15");
    xmlWriter.writeAttribute("FadeOut", "20");
    xmlWriter.writeAttribute("Values", "30");
    xmlWriter.writeCharacters("5:0,150,2,100,3,75:7:10,100,15,200");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(step.loadXML(xmlReader, number, NULL) == true);
    QCOMPARE(number, 1);
    QCOMPARE(step.fadeIn, uint(10));
    QCOMPARE(step.hold, uint(15));
    QCOMPARE(step.fadeOut, uint(20));
    QCOMPARE(step.duration, uint(25));
    QCOMPARE(step.fid, Function::invalidId());
    QCOMPARE(step.note, QString(""));

    QList<SceneValue> values = step.values;

    QCOMPARE(step.values.count(), 5);
    QCOMPARE(values.at(0), SceneValue(5, 0, 150));
    QCOMPARE(values.at(1), SceneValue(5, 2, 100));
    QCOMPARE(values.at(2), SceneValue(5, 3, 75));
    QCOMPARE(values.at(3), SceneValue(7, 10, 100));
    QCOMPARE(values.at(4), SceneValue(7, 15, 200));
}

void ChaserStep_Test::load_legacy_sequence()
{
    int number = -1;
    ChaserStep step;

    QBuffer buffer;
    QXmlStreamReader xmlReader(&buffer);
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeAttribute("FadeIn", "10");
    xmlWriter.writeAttribute("Hold", "15");
    xmlWriter.writeAttribute("FadeOut", "20");
    xmlWriter.writeAttribute("Values", "30");
    xmlWriter.writeCharacters("5,0,150,5,2,100,5,3,75,7,10,100,7,15,200");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(step.loadXML(xmlReader, number, NULL) == true);
    QCOMPARE(number, 1);
    QCOMPARE(step.fadeIn, uint(10));
    QCOMPARE(step.hold, uint(15));
    QCOMPARE(step.fadeOut, uint(20));
    QCOMPARE(step.duration, uint(25));
    QCOMPARE(step.fid, Function::invalidId());
    QCOMPARE(step.note, QString(""));

    /* check that no values have been loaded */
    QCOMPARE(step.values.count(), 0);
}

void ChaserStep_Test::save()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    xmlWriter.writeStartElement("Foo");

    ChaserStep step(1, 2, 3, 4);
    QVERIFY(step.saveXML(&xmlWriter, 5, false) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("Foo"));
    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("Step"));
    QCOMPARE(xmlReader.attributes().value("FadeIn").toString(), QString("2"));
    QCOMPARE(xmlReader.attributes().value("Hold").toString(), QString("3"));
    QCOMPARE(xmlReader.attributes().value("FadeOut").toString(), QString("4"));
    QCOMPARE(xmlReader.attributes().value("Number").toString(), QString("5"));
    QCOMPARE(xmlReader.readElementText(), QString("1"));
}

void ChaserStep_Test::save_sequence()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    xmlWriter.writeStartElement("Foo");

    ChaserStep step(1, 2, 3, 4);
    QList<SceneValue> values;
    values.append(SceneValue(1, 0, 255));
    values.append(SceneValue(1, 1, 0));
    values.append(SceneValue(1, 2, 100));
    values.append(SceneValue(1, 3, 200));

    values.append(SceneValue(3, 0, 255));
    values.append(SceneValue(3, 1, 100));
    values.append(SceneValue(3, 2, 0));
    values.append(SceneValue(3, 3, 0));

    step.values = values;

    QVERIFY(step.saveXML(&xmlWriter, 1, true) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("Foo"));
    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("Step"));
    QCOMPARE(xmlReader.attributes().value("FadeIn").toString(), QString("2"));
    QCOMPARE(xmlReader.attributes().value("Hold").toString(), QString("3"));
    QCOMPARE(xmlReader.attributes().value("FadeOut").toString(), QString("4"));
    QCOMPARE(xmlReader.attributes().value("Number").toString(), QString("1"));
    QCOMPARE(xmlReader.attributes().value("Values").toString(), QString("8"));

    QString valText = xmlReader.readElementText();
    QCOMPARE(valText, QString("1:0,255,2,100,3,200:3:0,255,1,100"));

}

QTEST_APPLESS_MAIN(ChaserStep_Test)
