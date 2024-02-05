/*
  Q Light Controller Plus - Unit test
  keypadparser_test.cpp

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

#include "keypadparser_test.h"
#include "keypadparser.h"
#include "qlcfile.h"
#include "fixture.h"
#include "doc.h"
#undef private
#undef protected

#include "../common/resource_paths.h"

void KeyPadParser_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir));
}

void KeyPadParser_Test::parsing()
{
    KeyPadParser parser;
    QList<SceneValue> scvList;
    QByteArray universeValues;
    quint32 chnum;

    universeValues.fill(0, 512);

    /* set channel 13 to value 148 */
    scvList = parser.parseCommand(m_doc, "13 AT 148", universeValues);

    QCOMPARE(scvList.count(), 1);
    QCOMPARE(scvList.first().fxi, Fixture::invalidId());
    QCOMPARE(scvList.first().channel, quint32(12));
    QCOMPARE(scvList.first().value, uchar(148));

    /* set channels 3 to 15 to value 133 */
    scvList = parser.parseCommand(m_doc, "3 THRU 15 AT 133", universeValues);

    QCOMPARE(scvList.count(), 13);
    chnum = 2;

    foreach (SceneValue scv, scvList)
    {
        QCOMPARE(scv.fxi, Fixture::invalidId());
        QCOMPARE(scv.channel, chnum++);
        QCOMPARE(scv.value, uchar(133));
    }

    /* Set channel 18 to value 255 */
    scvList = parser.parseCommand(m_doc, "18 FULL", universeValues);

    QCOMPARE(scvList.count(), 1);
    QCOMPARE(scvList.first().fxi, Fixture::invalidId());
    QCOMPARE(scvList.first().channel, quint32(17));
    QCOMPARE(scvList.first().value, uchar(255));

    /* Set channels 1 to 10 to value 255 */
    scvList = parser.parseCommand(m_doc, "1 THRU 10 FULL", universeValues);
    chnum = 0;

    QCOMPARE(scvList.count(), 10);
    foreach (SceneValue scv, scvList)
    {
        QCOMPARE(scv.fxi, Fixture::invalidId());
        QCOMPARE(scv.channel, chnum++);
        QCOMPARE(scv.value, uchar(255));
    }

    /* Set channel 4 to value 0 */
    scvList = parser.parseCommand(m_doc, "4 ZERO", universeValues);

    QCOMPARE(scvList.count(), 1);
    QCOMPARE(scvList.first().fxi, Fixture::invalidId());
    QCOMPARE(scvList.first().channel, quint32(3));
    QCOMPARE(scvList.first().value, uchar(0));

    scvList = parser.parseCommand(m_doc, "18 THRU 25 AT 0 THRU 255", universeValues);
    chnum = 17;
    float val = 0;
    float delta = 255.0 / 7;

    QCOMPARE(scvList.count(), 8);
    foreach (SceneValue scv, scvList)
    {
        QCOMPARE(scv.fxi, Fixture::invalidId());
        QCOMPARE(scv.channel, chnum++);
        QCOMPARE(scv.value, uchar(val));
        universeValues[chnum] = val;
        val += delta;
    }

    scvList = parser.parseCommand(m_doc, "AT ZERO", universeValues);
    chnum = 17;

    QCOMPARE(scvList.count(), 8);
    foreach (SceneValue scv, scvList)
    {
        QCOMPARE(scv.fxi, Fixture::invalidId());
        QCOMPARE(scv.channel, chnum++);
        QCOMPARE(scv.value, uchar(0));
        universeValues[chnum] = val;
    }

    scvList = parser.parseCommand(m_doc, "AT 123", universeValues);
    chnum = 17;

    QCOMPARE(scvList.count(), 8);
    foreach (SceneValue scv, scvList)
    {
        QCOMPARE(scv.fxi, Fixture::invalidId());
        QCOMPARE(scv.channel, chnum++);
        QCOMPARE(scv.value, uchar(123));
        universeValues[chnum] = val;
    }

    scvList = parser.parseCommand(m_doc, "AT FULL", universeValues);
    chnum = 17;

    QCOMPARE(scvList.count(), 8);
    foreach (SceneValue scv, scvList)
    {
        QCOMPARE(scv.fxi, Fixture::invalidId());
        QCOMPARE(scv.channel, chnum++);
        QCOMPARE(scv.value, uchar(255));
        universeValues[chnum] = val;
    }

    /* Set channels 6, 8, 10, 12, 14 and 16 to value 100 */
    scvList = parser.parseCommand(m_doc, "6 THRU 16 BY 2 AT 100", universeValues);
    chnum = 5;

    QCOMPARE(scvList.count(), 6);
    foreach (SceneValue scv, scvList)
    {
        QCOMPARE(scv.fxi, Fixture::invalidId());
        QCOMPARE(scv.channel, chnum);
        QCOMPARE(scv.value, uchar(100));
        universeValues[chnum] = 100;
        chnum += 2;
    }

    /* Increase the value of the channels set by last command by 20% */
    scvList = parser.parseCommand(m_doc, "6 THRU 16 BY 2 +% 20", universeValues);
    chnum = 5;

    QCOMPARE(scvList.count(), 6);
    foreach (SceneValue scv, scvList)
    {
        QCOMPARE(scv.fxi, Fixture::invalidId());
        QCOMPARE(scv.channel, chnum);
        QCOMPARE(scv.value, uchar(120));
        universeValues[chnum] = 120;
        chnum += 2;
    }

    /* Decrease the value of the channels set by last command by 40% */
    scvList = parser.parseCommand(m_doc, "6 THRU 16 BY 2 -% 40", universeValues);
    chnum = 5;

    QCOMPARE(scvList.count(), 6);
    foreach (SceneValue scv, scvList)
    {
        QCOMPARE(scv.fxi, Fixture::invalidId());
        QCOMPARE(scv.channel, chnum);
        QCOMPARE(scv.value, uchar(72));
        universeValues[chnum] = 72;
        chnum += 2;
    }
}

void KeyPadParser_Test::cleanupTestCase()
{
    delete m_doc;
}

QTEST_APPLESS_MAIN(KeyPadParser_Test)
