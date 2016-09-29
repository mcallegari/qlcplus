/*
  Q Light Controller Plus - Unit test
  functionspeeds_test.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari
                David Garyga

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

#include "functionspeeds_test.h"

#define protected public
#define private public
#include "functionspeeds.h"
#undef private
#undef protected

#include "doc.h"

void FunctionSpeeds_Test::initTestCase()
{
}

void FunctionSpeeds_Test::copyFrom()
{
    FunctionSpeeds t1(0, 1, 2);
    FunctionSpeeds t2 = t1;
    QCOMPARE(t2.fadeIn(), quint32(0));
    QCOMPARE(t2.hold(), quint32(1));
    QCOMPARE(t2.fadeOut(), quint32(2));
}

void FunctionSpeeds_Test::msToString()
{
    QCOMPARE(Speed::msToString(0), QString("0ms"));
    QCOMPARE(Speed::msToString(1000), QString("1s"));
    QCOMPARE(Speed::msToString(1000 * 60), QString("1m"));
    QCOMPARE(Speed::msToString(1000 * 60 * 60), QString("1h"));

    QCOMPARE(Speed::msToString(990), QString("990ms"));
    QCOMPARE(Speed::msToString(990 + 59 * 1000), QString("59s990ms"));
    QCOMPARE(Speed::msToString(990 + 59 * 1000 + 59 * 1000 * 60), QString("59m59s990ms"));
    QCOMPARE(Speed::msToString(990 + 59 * 1000 + 59 * 1000 * 60 + 99 * 1000 * 60 * 60), QString("99h59m59s990ms"));
    QCOMPARE(Speed::msToString(999 + 59 * 1000 + 59 * 1000 * 60 + 99 * 1000 * 60 * 60), QString("99h59m59s999ms"));
    QCOMPARE(Speed::msToString(1 + 1 * 1000 + 1 * 1000 * 60 + 1 * 1000 * 60 * 60), QString("1h01m01s001ms"));

    QCOMPARE(Speed::msToString(10), QString("10ms"));
    QCOMPARE(Speed::msToString(100), QString("100ms"));
}

void FunctionSpeeds_Test::stringToMs()
{
    QCOMPARE(Speed::stringToMs(".0"), uint(0));
    QCOMPARE(Speed::stringToMs(".0."), uint(0));
    QCOMPARE(Speed::stringToMs("0"), uint(0));
    QCOMPARE(Speed::stringToMs("0.0"), uint(0));

    QCOMPARE(Speed::stringToMs(".01"), uint(10));
    QCOMPARE(Speed::stringToMs(".010"), uint(10));
    QCOMPARE(Speed::stringToMs(".011"), uint(11));

    QCOMPARE(Speed::stringToMs(".03"), uint(30));
    QCOMPARE(Speed::stringToMs(".030"), uint(30));
    QCOMPARE(Speed::stringToMs(".031"), uint(31));

    QCOMPARE(Speed::stringToMs(".1"), uint(100));
    QCOMPARE(Speed::stringToMs(".10"), uint(100));
    QCOMPARE(Speed::stringToMs(".100"), uint(100));
    QCOMPARE(Speed::stringToMs(".101"), uint(101));

    QCOMPARE(Speed::stringToMs("1"), uint(1));
    QCOMPARE(Speed::stringToMs("1s"), uint(1000));
    QCOMPARE(Speed::stringToMs("1.000"), uint(1000));
    QCOMPARE(Speed::stringToMs("1.001"), uint(1001));
    QCOMPARE(Speed::stringToMs("1s.00"), uint(1000));
    QCOMPARE(Speed::stringToMs("1ms"), uint(1));

    QCOMPARE(Speed::stringToMs("1s.01"), uint(10 + 1000));
    QCOMPARE(Speed::stringToMs("1m1s.01"), uint(10 + 1000 + 1000 * 60));
    QCOMPARE(Speed::stringToMs("1h1m1s.01"), uint(10 + 1000 + 1000 * 60 + 1000 * 60 * 60));

    QCOMPARE(Speed::stringToMs("59s12ms"), uint(12 + 59 * 1000));
    QCOMPARE(Speed::stringToMs("59m59s999ms"), uint(999 + 59 * 1000 + 59 * 1000 * 60));

    // This string is broken, voluntarily ignore ms
    QCOMPARE(Speed::stringToMs("59m59s.999ms"), uint(/*999 +*/ 59 * 1000 + 59 * 1000 * 60));
}

void FunctionSpeeds_Test::operations()
{
    QCOMPARE(Speed::normalize(-1), Speed::infiniteValue());
    QCOMPARE(Speed::normalize(-10), Speed::infiniteValue());
    QCOMPARE(Speed::normalize(0), uint(0));
    QCOMPARE(Speed::normalize(12), uint(12));
    QCOMPARE(Speed::normalize(10), uint(10));
    QCOMPARE(Speed::normalize(20), uint(20));
    QCOMPARE(Speed::normalize(30), uint(30));
    QCOMPARE(Speed::normalize(40), uint(40));
    QCOMPARE(Speed::normalize(50), uint(50));
    QCOMPARE(Speed::normalize(60), uint(60));

    QCOMPARE(Speed::add(10, 10), uint(20));
    QCOMPARE(Speed::add(10, 0), uint(10));
    QCOMPARE(Speed::add(0, 10), uint(10));
    QCOMPARE(Speed::add(15, 15), uint(30));
    QCOMPARE(Speed::add(Speed::infiniteValue(), 10), Speed::infiniteValue());
    QCOMPARE(Speed::add(10, Speed::infiniteValue()), Speed::infiniteValue());
    QCOMPARE(Speed::add(Speed::infiniteValue(), Speed::infiniteValue()), Speed::infiniteValue());
    QCOMPARE(Speed::add(10, 0), uint(10));
    QCOMPARE(Speed::add(20, 0), uint(20));
    QCOMPARE(Speed::add(30, 0), uint(30));
    QCOMPARE(Speed::add(40, 0), uint(40));
    QCOMPARE(Speed::add(50, 0), uint(50));
    QCOMPARE(Speed::add(60, 0), uint(60));
    QCOMPARE(Speed::add(70, 0), uint(70));

    QCOMPARE(Speed::sub(10, 10), uint(0));
    QCOMPARE(Speed::sub(10, 0), uint(10));
    QCOMPARE(Speed::sub(0, 10), uint(0));
    QCOMPARE(Speed::sub(15, 2), uint(13));
    QCOMPARE(Speed::sub(Speed::infiniteValue(), 10), Speed::infiniteValue());
    QCOMPARE(Speed::sub(10, Speed::infiniteValue()), uint(0));
    QCOMPARE(Speed::sub(Speed::infiniteValue(), Speed::infiniteValue()), uint(0));
}

void FunctionSpeeds_Test::XML()
{
    FunctionSpeeds speeds(100, 200, 300);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(speeds.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Speed"));
    QCOMPARE(xmlReader.attributes().value("FadeIn").toString(), QString("100"));
    QCOMPARE(xmlReader.attributes().value("Hold").toString(), QString("200"));
    QCOMPARE(xmlReader.attributes().value("FadeOut").toString(), QString("300"));

    speeds.setFadeIn(0);
    speeds.setHold(0);
    speeds.setFadeOut(0);
    QVERIFY(speeds.loadXML(xmlReader) == true);
    QCOMPARE(speeds.fadeIn(), uint(100));
    QCOMPARE(speeds.hold(), uint(200));
    QCOMPARE(speeds.fadeOut(), uint(300));
}

QTEST_APPLESS_MAIN(FunctionSpeeds_Test)
