/*
  Q Light Controller - Unit tests
  qlccapability_test.cpp

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

#include <QtTest>
#include <QtXml>

#include "qlccapability_test.h"
#include "qlccapability.h"

void QLCCapability_Test::initial()
{
    QLCCapability cap;
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
    QVERIFY(cap.name().isEmpty());
}

void QLCCapability_Test::min_data()
{
    QTest::addColumn<uchar> ("value");
    for (uchar i = 0; i < UCHAR_MAX; i++)
        QTest::newRow("foo") << i;
    QTest::newRow("foo") << uchar(UCHAR_MAX);
}

void QLCCapability_Test::min()
{
    QLCCapability cap;
    QVERIFY(cap.min() == 0);

    QFETCH(uchar, value);

    cap.setMin(value);
    QCOMPARE(cap.min(), value);
}

void QLCCapability_Test::max_data()
{
    QTest::addColumn<uchar> ("value");
    for (uchar i = 0; i < UCHAR_MAX; i++)
        QTest::newRow("foo") << i;
    QTest::newRow("foo") << uchar(UCHAR_MAX);
}

void QLCCapability_Test::max()
{
    QLCCapability cap;
    QVERIFY(cap.max() == UCHAR_MAX);

    QFETCH(uchar, value);

    cap.setMax(value);
    QCOMPARE(cap.max(), value);
}

void QLCCapability_Test::middle()
{
    QLCCapability cap;
    QVERIFY(cap.max() == UCHAR_MAX);
    QCOMPARE(cap.middle(), uchar(127));
    cap.setMin(100);
    cap.setMax(200);
    QCOMPARE(cap.middle(), uchar(150));
}

void QLCCapability_Test::name()
{
    QLCCapability cap;
    QVERIFY(cap.name().isEmpty());

    cap.setName("Foobar");
    QVERIFY(cap.name() == "Foobar");
}

void QLCCapability_Test::overlaps()
{
    QLCCapability cap1;
    QLCCapability cap2;
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2 contains cap1 completely */
    cap1.setMin(10);
    cap1.setMax(245);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2's max overlaps cap1 */
    cap2.setMin(0);
    cap2.setMax(10);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2's max overlaps cap1 */
    cap2.setMin(0);
    cap2.setMax(15);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2's min overlaps cap1 */
    cap2.setMin(245);
    cap2.setMax(UCHAR_MAX);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2's min overlaps cap1 */
    cap2.setMin(240);
    cap2.setMax(UCHAR_MAX);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap1 contains cap2 completely */
    cap2.setMin(20);
    cap2.setMax(235);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    cap2.setMin(0);
    cap2.setMax(9);
    QVERIFY(cap1.overlaps(&cap2) == false);
    QVERIFY(cap2.overlaps(&cap1) == false);
}

void QLCCapability_Test::copy()
{
    QLCCapability cap1;
    QVERIFY(cap1.min() == 0);
    QVERIFY(cap1.max() == UCHAR_MAX);
    QVERIFY(cap1.name().isEmpty());

    cap1.setMin(5);
    cap1.setMax(15);
    cap1.setName("Foobar");

    QLCCapability *cap2 = cap1.createCopy();
    QVERIFY(cap2->min() == 5);
    QVERIFY(cap2->max() == 15);
    QVERIFY(cap2->name() == "Foobar");
}

void QLCCapability_Test::load()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Capability");
    doc.appendChild(root);

    root.setAttribute("Min", 13);
    root.setAttribute("Max", 19);

    QDomText name = doc.createTextNode("Test1");
    root.appendChild(name);

    QLCCapability cap;
    QVERIFY(cap.loadXML(root) == true);
    QVERIFY(cap.name() == "Test1");
    QVERIFY(cap.min() == 13);
    QVERIFY(cap.max() == 19);
}

void QLCCapability_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("apability");
    doc.appendChild(root);

    root.setAttribute("Min", 13);
    root.setAttribute("Max", 19);

    QDomText name = doc.createTextNode("Test1");
    root.appendChild(name);

    QLCCapability cap;
    QVERIFY(cap.loadXML(root) == false);
    QVERIFY(cap.name().isEmpty());
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
}

void QLCCapability_Test::loadNoMin()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Capability");
    doc.appendChild(root);

    root.setAttribute("Max", 19);

    QDomText name = doc.createTextNode("Test1");
    root.appendChild(name);

    QLCCapability cap;
    QVERIFY(cap.loadXML(root) == false);
    QVERIFY(cap.name().isEmpty());
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
}

void QLCCapability_Test::loadNoMax()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Capability");
    doc.appendChild(root);

    root.setAttribute("Min", 13);

    QDomText name = doc.createTextNode("Test1");
    root.appendChild(name);

    QLCCapability cap;
    QVERIFY(cap.loadXML(root) == false);
    QVERIFY(cap.name().isEmpty());
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
}

void QLCCapability_Test::loadMinGreaterThanMax()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Capability");
    doc.appendChild(root);

    root.setAttribute("Min", 20);
    root.setAttribute("Max", 19);

    QDomText name = doc.createTextNode("Test1");
    root.appendChild(name);

    QLCCapability cap;
    QVERIFY(cap.loadXML(root) == false);
    QVERIFY(cap.name().isEmpty());
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
}

void QLCCapability_Test::save()
{
    QLCCapability cap;
    cap.setName("Testing");
    cap.setMin(5);
    cap.setMax(87);

    QDomDocument doc;
    QDomElement root = doc.createElement("TestRoot");

    QVERIFY(cap.saveXML(&doc, &root) == true);
    QVERIFY(root.firstChild().toElement().tagName() == "Capability");
    QVERIFY(root.firstChild().toElement().text() == "Testing");
    QVERIFY(root.firstChild().toElement().attribute("Min") == "5");
    QVERIFY(root.firstChild().toElement().attribute("Max") == "87");
}

QTEST_APPLESS_MAIN(QLCCapability_Test)
