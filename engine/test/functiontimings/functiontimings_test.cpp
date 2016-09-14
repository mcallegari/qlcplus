/*
  Q Light Controller Plus - Unit test
  functiontimings_test.cpp

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

#include "functiontimings_test.h"

#define protected public
#define private public
#include "functiontimings.h"
#undef private
#undef protected

#include "doc.h"

void FunctionTimings_Test::initTestCase()
{
}

void FunctionTimings_Test::copyFrom()
{
    FunctionTimings t1(0, 1, 2);
    FunctionTimings t2 = t1;
    QCOMPARE(t2.fadeIn, quint32(0));
    QCOMPARE(t2.hold, quint32(1));
    QCOMPARE(t2.fadeOut, quint32(2));
}

void FunctionTimings_Test::valueToString()
{
    QCOMPARE(FunctionTimings::valueToString(0), QString("0ms"));
    QCOMPARE(FunctionTimings::valueToString(1000), QString("1s"));
    QCOMPARE(FunctionTimings::valueToString(1000 * 60), QString("1m"));
    QCOMPARE(FunctionTimings::valueToString(1000 * 60 * 60), QString("1h"));

    QCOMPARE(FunctionTimings::valueToString(990), QString("990ms"));
    QCOMPARE(FunctionTimings::valueToString(990 + 59 * 1000), QString("59s990ms"));
    QCOMPARE(FunctionTimings::valueToString(990 + 59 * 1000 + 59 * 1000 * 60), QString("59m59s990ms"));
    QCOMPARE(FunctionTimings::valueToString(990 + 59 * 1000 + 59 * 1000 * 60 + 99 * 1000 * 60 * 60), QString("99h59m59s990ms"));
    QCOMPARE(FunctionTimings::valueToString(999 + 59 * 1000 + 59 * 1000 * 60 + 99 * 1000 * 60 * 60), QString("99h59m59s999ms"));
    QCOMPARE(FunctionTimings::valueToString(1 + 1 * 1000 + 1 * 1000 * 60 + 1 * 1000 * 60 * 60), QString("1h01m01s001ms"));

    QCOMPARE(FunctionTimings::valueToString(10), QString("10ms"));
    QCOMPARE(FunctionTimings::valueToString(100), QString("100ms"));
}

void FunctionTimings_Test::stringToValue()
{
    QCOMPARE(FunctionTimings::stringToValue(".0"), uint(0));
    QCOMPARE(FunctionTimings::stringToValue(".0."), uint(0));
    QCOMPARE(FunctionTimings::stringToValue("0"), uint(0));
    QCOMPARE(FunctionTimings::stringToValue("0.0"), uint(0));

    QCOMPARE(FunctionTimings::stringToValue(".01"), uint(10));
    QCOMPARE(FunctionTimings::stringToValue(".010"), uint(10));
    QCOMPARE(FunctionTimings::stringToValue(".011"), uint(11));

    QCOMPARE(FunctionTimings::stringToValue(".03"), uint(30));
    QCOMPARE(FunctionTimings::stringToValue(".030"), uint(30));
    QCOMPARE(FunctionTimings::stringToValue(".031"), uint(31));

    QCOMPARE(FunctionTimings::stringToValue(".1"), uint(100));
    QCOMPARE(FunctionTimings::stringToValue(".10"), uint(100));
    QCOMPARE(FunctionTimings::stringToValue(".100"), uint(100));
    QCOMPARE(FunctionTimings::stringToValue(".101"), uint(101));

    QCOMPARE(FunctionTimings::stringToValue("1"), uint(1));
    QCOMPARE(FunctionTimings::stringToValue("1s"), uint(1000));
    QCOMPARE(FunctionTimings::stringToValue("1.000"), uint(1000));
    QCOMPARE(FunctionTimings::stringToValue("1.001"), uint(1001));
    QCOMPARE(FunctionTimings::stringToValue("1s.00"), uint(1000));
    QCOMPARE(FunctionTimings::stringToValue("1ms"), uint(1));

    QCOMPARE(FunctionTimings::stringToValue("1s.01"), uint(10 + 1000));
    QCOMPARE(FunctionTimings::stringToValue("1m1s.01"), uint(10 + 1000 + 1000 * 60));
    QCOMPARE(FunctionTimings::stringToValue("1h1m1s.01"), uint(10 + 1000 + 1000 * 60 + 1000 * 60 * 60));

    QCOMPARE(FunctionTimings::stringToValue("59s12ms"), uint(12 + 59 * 1000));
    QCOMPARE(FunctionTimings::stringToValue("59m59s999ms"), uint(999 + 59 * 1000 + 59 * 1000 * 60));

    // This string is broken, voluntarily ignore ms
    QCOMPARE(FunctionTimings::stringToValue("59m59s.999ms"), uint(/*999 +*/ 59 * 1000 + 59 * 1000 * 60));
}

void FunctionTimings_Test::operations()
{
    QCOMPARE(FunctionTimings::normalize(-1), FunctionTimings::infiniteValue());
    QCOMPARE(FunctionTimings::normalize(-10), FunctionTimings::infiniteValue());
    QCOMPARE(FunctionTimings::normalize(0), uint(0));
    QCOMPARE(FunctionTimings::normalize(12), uint(12));
    QCOMPARE(FunctionTimings::normalize(10), uint(10));
    QCOMPARE(FunctionTimings::normalize(20), uint(20));
    QCOMPARE(FunctionTimings::normalize(30), uint(30));
    QCOMPARE(FunctionTimings::normalize(40), uint(40));
    QCOMPARE(FunctionTimings::normalize(50), uint(50));
    QCOMPARE(FunctionTimings::normalize(60), uint(60));

    QCOMPARE(FunctionTimings::add(10, 10), uint(20));
    QCOMPARE(FunctionTimings::add(10, 0), uint(10));
    QCOMPARE(FunctionTimings::add(0, 10), uint(10));
    QCOMPARE(FunctionTimings::add(15, 15), uint(30));
    QCOMPARE(FunctionTimings::add(FunctionTimings::infiniteValue(), 10), FunctionTimings::infiniteValue());
    QCOMPARE(FunctionTimings::add(10, FunctionTimings::infiniteValue()), FunctionTimings::infiniteValue());
    QCOMPARE(FunctionTimings::add(FunctionTimings::infiniteValue(), FunctionTimings::infiniteValue()), FunctionTimings::infiniteValue());
    QCOMPARE(FunctionTimings::add(10, 0), uint(10));
    QCOMPARE(FunctionTimings::add(20, 0), uint(20));
    QCOMPARE(FunctionTimings::add(30, 0), uint(30));
    QCOMPARE(FunctionTimings::add(40, 0), uint(40));
    QCOMPARE(FunctionTimings::add(50, 0), uint(50));
    QCOMPARE(FunctionTimings::add(60, 0), uint(60));
    QCOMPARE(FunctionTimings::add(70, 0), uint(70));

    QCOMPARE(FunctionTimings::subtract(10, 10), uint(0));
    QCOMPARE(FunctionTimings::subtract(10, 0), uint(10));
    QCOMPARE(FunctionTimings::subtract(0, 10), uint(0));
    QCOMPARE(FunctionTimings::subtract(15, 2), uint(13));
    QCOMPARE(FunctionTimings::subtract(FunctionTimings::infiniteValue(), 10), FunctionTimings::infiniteValue());
    QCOMPARE(FunctionTimings::subtract(10, FunctionTimings::infiniteValue()), uint(0));
    QCOMPARE(FunctionTimings::subtract(FunctionTimings::infiniteValue(), FunctionTimings::infiniteValue()), uint(0));
}

void FunctionTimings_Test::XML()
{
    FunctionTimings timings(100, 200, 300);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(timings.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Timings"));
    QCOMPARE(xmlReader.attributes().value("FadeIn").toString(), QString("100"));
    QCOMPARE(xmlReader.attributes().value("Hold").toString(), QString("200"));
    QCOMPARE(xmlReader.attributes().value("FadeOut").toString(), QString("300"));

    timings.fadeIn = 0;
    timings.hold = 0;
    timings.fadeOut = 0;
    QVERIFY(timings.loadXML(xmlReader) == true);
    QCOMPARE(timings.fadeIn, uint(100));
    QCOMPARE(timings.hold, uint(200));
    QCOMPARE(timings.fadeOut, uint(300));
}

QTEST_APPLESS_MAIN(FunctionTimings_Test)
