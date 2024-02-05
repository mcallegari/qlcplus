/*
  Q Light Controller Plus
  monitorfixtureitem_test.cpp

  Copyright (C) Jano Svitok

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

#include <QLabel>
#include <QtTest>

#define protected public
#define private public
#include "monitorfixtureitem.h"
#undef protected
#undef private

#include "monitorfixtureitem_test.h"
#include "qlcfixturedefcache.h"
#include "qlcfixturedef.h"
#include "qlcfile.h"
#include "qlcmacros.h"
#include "doc.h"

#include "../../../engine/test/common/resource_paths.h"

void MonitorFixtureItem_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void MonitorFixtureItem_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void MonitorFixtureItem_Test::cleanup()
{
    m_doc->clearContents();
}

void MonitorFixtureItem_Test::computeAlpha()
{
    QFETCH(QString, manufacturer);
    QFETCH(QString, model);
    QFETCH(QString, mode);
    QFETCH(quint32, masterDimmer);
    QFETCH(quint32, head1Dimmer);
    QFETCH(quint32, head2Dimmer);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setName("Test Fixture");

    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);
    QVERIFY(def != NULL);
    QVERIFY(def != NULL);
    QVERIFY(def->channels().size() > 0);
    QLCFixtureMode* m = def->mode(mode);
    QVERIFY(m != NULL);

    fxi->setFixtureDefinition(def, m);
    fxi->setUniverse(0);
    fxi->setAddress(0);
    m_doc->addFixture(fxi);

    MonitorFixtureItem* mfi = new MonitorFixtureItem(m_doc, fxi->id());
    const FixtureHead* h1 = mfi->m_heads.at(0);
    const FixtureHead* h2 = mfi->m_heads.at(1);

    QVERIFY(h1 != NULL);
    QVERIFY(h2 != NULL);

    if (masterDimmer != QLCChannel::invalid())
    {
        QCOMPARE(h1->m_masterDimmer, masterDimmer);
        QCOMPARE(h2->m_masterDimmer, masterDimmer);
    }
    else
    {
        QCOMPARE(h1->m_masterDimmer, QLCChannel::invalid());
        QCOMPARE(h2->m_masterDimmer, QLCChannel::invalid());
    }

    if (head1Dimmer != QLCChannel::invalid())
    {
        QCOMPARE(h1->m_dimmer, head1Dimmer);
    }
    else
    {
        QCOMPARE(h1->m_dimmer, QLCChannel::invalid());
    }

    if (head2Dimmer != QLCChannel::invalid())
    {
        QCOMPARE(h2->m_dimmer, head2Dimmer);
    }
    else
    {
        QCOMPARE(h2->m_dimmer, QLCChannel::invalid());
    }

    if (masterDimmer == QLCChannel::invalid() && head1Dimmer == QLCChannel::invalid())
    {
       {
           QByteArray values(fxi->channels(), (char)255);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)255u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)255u);
       }

       {
           QByteArray values(fxi->channels(), 127);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)255u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)255u);
       }

       {
           QByteArray values(fxi->channels(), 0);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)255u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)255u);
       }
    }
    else if (masterDimmer != QLCChannel::invalid() && head1Dimmer == QLCChannel::invalid())
    {
       {
           QByteArray values(fxi->channels(), (char)255);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)255u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)255u);
       }

       {
           QByteArray values(fxi->channels(), 127);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)127u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)127u);
       }

       {
           QByteArray values(fxi->channels(), 0);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)0u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)0u);
       }
    }
    else if (masterDimmer == QLCChannel::invalid() && head1Dimmer != QLCChannel::invalid())
    {
       {
           QByteArray values(fxi->channels(), (char)255);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)255u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)255u);
       }

       {
           QByteArray values(fxi->channels(), 127);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)127u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)127u);
       }

       {
           QByteArray values(fxi->channels(), 0);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)0u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)0u);
       }
    }
    else // masterDimmer != QLCChannel::invalid() && head1Dimmer != QLCChannel::invalid()
    {
       // change both master & head dimmer

       {
           QByteArray values(fxi->channels(), (char)255);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)255u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)255u);
       }

       {
           QByteArray values(fxi->channels(), 127);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)63u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)63u);
       }

       {
           QByteArray values(fxi->channels(), 0);
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)0u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)0u);
       }

       // change only master dimmer

       {
           QByteArray values(fxi->channels(), (char)255);
           values[masterDimmer] = 127;
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)127u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)127u);
       }

       {
           QByteArray values(fxi->channels(), (char)255);
           values[masterDimmer] = 0;
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)0u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)0u);
       }

       // change only head dimmer

       {
           QByteArray values(fxi->channels(), (char)255);
           values[head1Dimmer] = 100;
           values[head2Dimmer] = (char)200;
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)100u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)200u);
       }

       {
           QByteArray values(fxi->channels(), (char)255);
           values[head1Dimmer] = 0;
           values[head2Dimmer] = 0;
           QCOMPARE(mfi->computeAlpha(h1, values), (uchar)0u);
           QCOMPARE(mfi->computeAlpha(h2, values), (uchar)0u);
       }
    }
}

void MonitorFixtureItem_Test::computeAlpha_data()
{
    QTest::addColumn<QString>("manufacturer");
    QTest::addColumn<QString>("model");
    QTest::addColumn<QString>("mode");
    QTest::addColumn<quint32>("masterDimmer");
    QTest::addColumn<quint32>("head1Dimmer");
    QTest::addColumn<quint32>("head2Dimmer");

    QTest::newRow("5 heads with dimmer")
        << "Showtec"
        << "Sunstrip Active"
        << "5 Channels Mode"
        << QLCChannel::invalid()
        << 0u
        << 1u;

    QTest::newRow("4 heads with dimmer and RGB")
        << "American DJ"
        << "Quad Scan LED"
        << "28 Channels mode"
        << QLCChannel::invalid()
        << 5u
        << 12u;

    QTest::newRow("Master dimmer and 4 heads with dimmer")
        << "American DJ"
        << "Event Bar LED"
        << "25 Channel Mode"
        << 24u
        << 4u
        << 9u;

    QTest::newRow("Master dimmer and 3 heads with RGB")
        << "American DJ"
        << "Mega Bar LED"
        << "11 Channels"
        << 10u
        << QLCChannel::invalid()
        << QLCChannel::invalid();

    QTest::newRow("Master dimmer and 4 heads with dimmer and RGB")
        << "ADB"
        << "ALC4"
        << "Extended 21 Channels (CT Linear)"
        << 0u
        << 1u
        << 6u;

     QTest::newRow("4 RGB heads")
        << "American DJ"
        << "Dotz Bar 1.4"
        << "12 Channel"
        << QLCChannel::invalid()
        << QLCChannel::invalid()
        << QLCChannel::invalid();
}

QTEST_MAIN(MonitorFixtureItem_Test)
