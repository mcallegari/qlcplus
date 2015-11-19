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

void ChaserStep_Test::load()
{
    int number = -1;
    ChaserStep step;

    QBuffer buffer;
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    QVERIFY(step.loadXML(xmlReader, number) == false);
    QCOMPARE(number, -1);

    buffer.close();
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "5");
    xmlWriter.writeAttribute("FadeIn", "10");
    xmlWriter.writeAttribute("Hold", "15");
    xmlWriter.writeAttribute("FadeOut", "20");
    xmlWriter.writeAttribute("Duration", "25");
    xmlWriter.writeAttribute("Note", "Fortytwo");
    xmlWriter.writeCharacters("30");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(step.loadXML(xmlReader, number) == true);
    QCOMPARE(number, 5);
    QCOMPARE(step.fadeIn, uint(10));
    QCOMPARE(step.hold, uint(15));
    QCOMPARE(step.fadeOut, uint(20));
    QCOMPARE(step.duration, uint(25));
    QCOMPARE(step.fid, quint32(30));
    QCOMPARE(step.note, QString("Fortytwo"));
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
    //QCOMPARE(tag.attribute("Duration"), QString("4")); // deprecated from version 4.3.2
    QCOMPARE(xmlReader.attributes().value("Number").toString(), QString("5"));
    QCOMPARE(xmlReader.readElementText(), QString("1"));
}

QTEST_APPLESS_MAIN(ChaserStep_Test)
