/*
  Q Light Controller - Unit tests
  chaserstep_test.cpp

  Copyright (C) Heikki Junnila

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

#include <QDomDocument>
#include <QDomElement>
#include <QtTest>

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

    QVERIFY(step.loadXML(QDomElement(), number) == false);
    QCOMPARE(number, -1);

    QDomDocument doc;
    QDomElement root = doc.createElement("Step");
    root.setAttribute("Number", 5);
    root.setAttribute("FadeIn", 10);
    root.setAttribute("Hold", 15);
    root.setAttribute("FadeOut", 20);
    root.setAttribute("Duration", 25);
    QDomText text = doc.createTextNode("30");
    root.appendChild(text);

    QVERIFY(step.loadXML(root, number) == true);
    QCOMPARE(number, 5);
    QCOMPARE(step.fadeIn, uint(10));
    QCOMPARE(step.hold, uint(15));
    QCOMPARE(step.fadeOut, uint(20));
    QCOMPARE(step.duration, uint(25));
    QCOMPARE(step.fid, quint32(30));
}

void ChaserStep_Test::save()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");

    ChaserStep step(1, 2, 3, 4);
    QVERIFY(step.saveXML(&doc, &root, 5, false) == true);
    QDomElement tag = root.firstChild().toElement();
    QCOMPARE(tag.tagName(), QString("Step"));
    QCOMPARE(tag.text(), QString("1"));
    QCOMPARE(tag.attribute("FadeIn"), QString("2"));
    QCOMPARE(tag.attribute("Hold"), QString("3"));
    QCOMPARE(tag.attribute("FadeOut"), QString("4"));
    //QCOMPARE(tag.attribute("Duration"), QString("4")); // deprecated from version 4.3.2
    QCOMPARE(tag.attribute("Number"), QString("5"));
}

QTEST_APPLESS_MAIN(ChaserStep_Test)
