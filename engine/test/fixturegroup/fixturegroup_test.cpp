/*
  Q Light Controller - Unit test
  fixturegroup_test.cpp

  Copyright (c) Heikki Junnila

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

#include "fixturegroup_test.h"
#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fixturegroup.h"
#include "qlcfile.h"
#include "doc.h"

#include "../common/resource_paths.h"

void FixtureGroup_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
}

void FixtureGroup_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void FixtureGroup_Test::init()
{
    m_doc->clearContents();
}

void FixtureGroup_Test::groupHead()
{
    GroupHead gh;
    QCOMPARE(gh.fxi, Fixture::invalidId());
    QCOMPARE(gh.head, -1);
    QVERIFY(gh.isValid() == false);

    GroupHead gh2(10, 20);
    QCOMPARE(gh2.fxi, quint32(10));
    QCOMPARE(gh2.head, 20);
    QVERIFY(gh2.isValid() == true);

    QVERIFY((gh == gh2) == false);

    gh = gh2;
    QCOMPARE(gh.fxi, quint32(10));
    QCOMPARE(gh.head, 20);
    QVERIFY(gh.isValid() == true);

    QVERIFY(gh == gh2);

    GroupHead gh3(1);
    QCOMPARE(gh3.fxi, quint32(1));
    QCOMPARE(gh3.head, -1);
    QVERIFY(gh3.isValid() == false);

    QVERIFY((gh2 == gh3) == false);

    GroupHead gh4(Fixture::invalidId(), 1);
    QCOMPARE(gh4.fxi, Fixture::invalidId());
    QCOMPARE(gh4.head, 1);
    QVERIFY(gh4.isValid() == false);

    QVERIFY((gh3 == gh4) == false);
}

void FixtureGroup_Test::id()
{
    FixtureGroup grp(m_doc);
    QSignalSpy spy(&grp, SIGNAL(changed(quint32)));
    QCOMPARE(grp.id(), FixtureGroup::invalidId());
    grp.setId(69);
    QCOMPARE(grp.id(), quint32(69));
    QCOMPARE(spy.size(), 0);
    grp.setId(42);
    QCOMPARE(grp.id(), quint32(42));
    QCOMPARE(spy.size(), 0);
}

void FixtureGroup_Test::name()
{
    FixtureGroup grp(m_doc);
    QSignalSpy spy(&grp, SIGNAL(changed(quint32)));
    QCOMPARE(grp.name(), QString());
    grp.setName("Esko Mörkö");
    QCOMPARE(spy.size(), 1);
    QCOMPARE(grp.name(), QString("Esko Mörkö"));
    grp.setName("Pertti Pasanen");
    QCOMPARE(spy.size(), 2);
    QCOMPARE(grp.name(), QString("Pertti Pasanen"));
}

void FixtureGroup_Test::size()
{
    FixtureGroup grp(m_doc);
    QSignalSpy spy(&grp, SIGNAL(changed(quint32)));
    QCOMPARE(grp.size(), QSize());
    grp.setSize(QSize(10, 10));
    QCOMPARE(spy.size(), 1);
    QCOMPARE(grp.size(), QSize(10, 10));
    grp.setSize(QSize(20, 30));
    QCOMPARE(spy.size(), 2);
    QCOMPARE(grp.size(), QSize(20, 30));
}

void FixtureGroup_Test::assignFixtureNoSize()
{
    QLCPoint pt;
    FixtureGroup grp(m_doc);
    QCOMPARE(grp.headList().size(), 0);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(2);
    m_doc->addFixture(fxi);

    grp.assignFixture(fxi->id());
    QCOMPARE(grp.headList().size(), 2);
    QCOMPARE(grp.size(), QSize(1, 1));
    QVERIFY(grp.headHash()[QLCPoint(0, 0)] == GroupHead(0, 0));
    QVERIFY(grp.headHash()[QLCPoint(0, 1)] == GroupHead(0, 1));

    // Same fixture can't be at two places
    grp.assignFixture(0, QLCPoint(100, 100));
    QCOMPARE(grp.headList().size(), 2);
    QCOMPARE(grp.size(), QSize(1, 1));
    QVERIFY(grp.headHash()[QLCPoint(0, 0)] == GroupHead(0, 0));
    QVERIFY(grp.headHash()[QLCPoint(0, 1)] == GroupHead(0, 1));

    fxi = new Fixture(m_doc);
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    grp.assignFixture(fxi->id());
    QCOMPARE(grp.headList().size(), 3);
    QCOMPARE(grp.size(), QSize(1, 1));
    QVERIFY(grp.headHash()[QLCPoint(0, 0)] == GroupHead(0, 0));
    QVERIFY(grp.headHash()[QLCPoint(0, 1)] == GroupHead(0, 1));
    QVERIFY(grp.headHash()[QLCPoint(0, 2)] == GroupHead(1, 0));

    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    grp.assignFixture(fxi->id());
    QCOMPARE(grp.headList().size(), 4);
    QCOMPARE(grp.size(), QSize(1, 1));
    QVERIFY(grp.headHash()[QLCPoint(0, 0)] == GroupHead(0, 0));
    QVERIFY(grp.headHash()[QLCPoint(0, 1)] == GroupHead(0, 1));
    QVERIFY(grp.headHash()[QLCPoint(0, 2)] == GroupHead(1, 0));
    QVERIFY(grp.headHash()[QLCPoint(0, 3)] == GroupHead(2, 0));

    def = m_doc->fixtureDefCache()->fixtureDef("i-Pix", "BB4");
    QVERIFY(def != NULL);
    mode = def->modes().last();
    QVERIFY(mode != NULL);
    QCOMPARE(mode->heads().size(), 4);

    fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    grp.assignFixture(fxi->id());
    QCOMPARE(grp.headList().size(), 8);
    QCOMPARE(grp.size(), QSize(1, 1));
    QVERIFY(grp.headHash()[QLCPoint(0, 0)] == GroupHead(0, 0));
    QVERIFY(grp.headHash()[QLCPoint(0, 1)] == GroupHead(0, 1));
    QVERIFY(grp.headHash()[QLCPoint(0, 2)] == GroupHead(1, 0));
    QVERIFY(grp.headHash()[QLCPoint(0, 3)] == GroupHead(2, 0));
    // BB4 heads
    QVERIFY(grp.headHash()[QLCPoint(0, 4)] == GroupHead(3, 0));
    QVERIFY(grp.headHash()[QLCPoint(0, 5)] == GroupHead(3, 1));
    QVERIFY(grp.headHash()[QLCPoint(0, 6)] == GroupHead(3, 2));
    QVERIFY(grp.headHash()[QLCPoint(0, 7)] == GroupHead(3, 3));
}

void FixtureGroup_Test::assignFixture4x2()
{
    QLCPoint pt;
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 2));
    QCOMPARE(grp.headList().size(), 0);

    for (int i = 0; i < 11; i++)
    {
        QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
        QVERIFY(def != NULL);
        QLCFixtureMode* mode = def->modes().first();
        QVERIFY(mode != NULL);

        Fixture* fxi = new Fixture(m_doc);
        fxi->setFixtureDefinition(def, mode);
        m_doc->addFixture(fxi);
    }

    grp.assignFixture(0);
    QCOMPARE(grp.headList().size(), 1);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));

    grp.assignFixture(1);
    QCOMPARE(grp.headList().size(), 2);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.head(pt) == GroupHead(1, 0));

    grp.assignFixture(2);
    QCOMPARE(grp.headList().size(), 3);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.head(pt) == GroupHead(1, 0));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.head(pt) == GroupHead(2, 0));

    grp.assignFixture(3);
    QCOMPARE(grp.headList().size(), 4);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.head(pt) == GroupHead(1, 0));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.head(pt) == GroupHead(2, 0));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.head(pt) == GroupHead(3, 0));

    grp.assignFixture(4);
    QCOMPARE(grp.headList().size(), 5);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.head(pt) == GroupHead(1, 0));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.head(pt) == GroupHead(2, 0));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.head(pt) == GroupHead(3, 0));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.head(pt) == GroupHead(4, 0));

    grp.assignFixture(5);
    QCOMPARE(grp.headList().size(), 6);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.head(pt) == GroupHead(1, 0));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.head(pt) == GroupHead(2, 0));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.head(pt) == GroupHead(3, 0));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.head(pt) == GroupHead(4, 0));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.head(pt) == GroupHead(5, 0));

    grp.assignFixture(6);
    QCOMPARE(grp.headList().size(), 7);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.head(pt) == GroupHead(1, 0));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.head(pt) == GroupHead(2, 0));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.head(pt) == GroupHead(3, 0));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.head(pt) == GroupHead(4, 0));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.head(pt) == GroupHead(5, 0));
    pt = QLCPoint(2, 1);
    QVERIFY(grp.head(pt) == GroupHead(6, 0));

    grp.assignFixture(7);
    QCOMPARE(grp.headList().size(), 8);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.head(pt) == GroupHead(1, 0));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.head(pt) == GroupHead(2, 0));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.head(pt) == GroupHead(3, 0));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.head(pt) == GroupHead(4, 0));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.head(pt) == GroupHead(5, 0));
    pt = QLCPoint(2, 1);
    QVERIFY(grp.head(pt) == GroupHead(6, 0));
    pt = QLCPoint(3, 1);
    QVERIFY(grp.head(pt) == GroupHead(7, 0));

    // Now beyond size(); should continue to make a third row of 4 columns
    grp.assignFixture(8);
    QCOMPARE(grp.headList().size(), 9);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.head(pt) == GroupHead(1, 0));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.head(pt) == GroupHead(2, 0));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.head(pt) == GroupHead(3, 0));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.head(pt) == GroupHead(4, 0));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.head(pt) == GroupHead(5, 0));
    pt = QLCPoint(2, 1);
    QVERIFY(grp.head(pt) == GroupHead(6, 0));
    pt = QLCPoint(3, 1);
    QVERIFY(grp.head(pt) == GroupHead(7, 0));
    pt = QLCPoint(0, 2);
    QVERIFY(grp.head(pt) == GroupHead(8, 0));

    grp.assignFixture(9);
    QCOMPARE(grp.headList().size(), 10);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.head(pt) == GroupHead(0, 0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.head(pt) == GroupHead(1, 0));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.head(pt) == GroupHead(2, 0));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.head(pt) == GroupHead(3, 0));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.head(pt) == GroupHead(4, 0));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.head(pt) == GroupHead(5, 0));
    pt = QLCPoint(2, 1);
    QVERIFY(grp.head(pt) == GroupHead(6, 0));
    pt = QLCPoint(3, 1);
    QVERIFY(grp.head(pt) == GroupHead(7, 0));
    pt = QLCPoint(0, 2);
    QVERIFY(grp.head(pt) == GroupHead(8, 0));
    pt = QLCPoint(1, 2);
    QVERIFY(grp.head(pt) == GroupHead(9, 0));

    // Going waaay beyond size should be possible
    pt = QLCPoint(1024, 2048);
    grp.assignFixture(10, pt);
    QVERIFY(grp.headHash().contains(pt) == true);
    QCOMPARE(grp.head(pt), GroupHead(10, 0));
    QCOMPARE(grp.size(), QSize(4, 2));
}

void FixtureGroup_Test::resignFixture()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 4));
    for (quint32 id = 0; id < 16; id++)
    {
        Fixture* fxi = new Fixture(m_doc);
        fxi->setChannels(1);
        m_doc->addFixture(fxi);
        grp.assignFixture(fxi->id());
    }
    QCOMPARE(grp.headList().size(), 16);

    // Remove a fixture
    grp.resignFixture(13);
    QCOMPARE(grp.headList().size(), 15);
    QVERIFY(grp.headList().contains(13) == false);
    QVERIFY(grp.headHash().contains(QLCPoint(1, 3)) == false);

    // Remove a nonexistent fixture
    grp.resignFixture(42);
    QCOMPARE(grp.headList().size(), 15);
    QVERIFY(grp.headList().contains(42) == false);
    QVERIFY(grp.headHash().contains(QLCPoint(1, 3)) == false);

    // Test that the gap is again filled
    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(1);
    m_doc->addFixture(fxi, 42);
    grp.assignFixture(42);
    QCOMPARE(grp.headList().size(), 16);
    QVERIFY(grp.headList().contains(GroupHead(42, 0)) == true);
    QVERIFY(grp.headHash().contains(QLCPoint(1, 3)) == true);
    QCOMPARE(grp.headHash()[QLCPoint(1, 3)], GroupHead(42, 0));
}

void FixtureGroup_Test::resignHead()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 4));
    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(16);
    m_doc->addFixture(fxi);

    for (quint32 id = 0; id < 16; id++)
        grp.assignFixture(fxi->id());
    QCOMPARE(grp.headList().size(), 16);

    QSignalSpy spy(&grp, SIGNAL(changed(quint32)));
    QCOMPARE(grp.resignHead(QLCPoint(0, 0)), true);
    QCOMPARE(grp.headList().size(), 15);
    QCOMPARE(grp.headHash().contains(QLCPoint(0, 0)), false);
    QCOMPARE(spy.size(), 1);

    QCOMPARE(grp.resignHead(QLCPoint(0, 0)), false);
    QCOMPARE(grp.headList().size(), 15);
    QCOMPARE(grp.headHash().contains(QLCPoint(0, 0)), false);
    QCOMPARE(spy.size(), 1);

    QCOMPARE(grp.resignHead(QLCPoint(15, 0)), false);
    QCOMPARE(grp.headList().size(), 15);
    QCOMPARE(grp.headHash().contains(QLCPoint(15, 0)), false);
    QCOMPARE(grp.headHash().contains(QLCPoint(0, 0)), false);
    QCOMPARE(spy.size(), 1);

    // Assign the head back to 0, 0
    grp.assignHead(QLCPoint(0, 0), GroupHead(fxi->id(), 0));
    QCOMPARE(spy.size(), 2);

    // Verify that head & fixtures work properly
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            QLCPoint pt(x, y);
            QCOMPARE(grp.resignHead(pt), true);
            if (x == 3 && y == 3)
                QCOMPARE(grp.fixtureList().size(), 0);
            else
                QCOMPARE(grp.fixtureList().size(), 1);
        }
    }

    QCOMPARE(spy.size(), 18);
}

void FixtureGroup_Test::fixtureRemoved()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 4));
    for (quint32 id = 0; id < 16; id++)
    {
        Fixture* fxi = new Fixture(m_doc);
        fxi->setChannels(1);
        m_doc->addFixture(fxi);
        grp.assignFixture(fxi->id());
    }
    QCOMPARE(grp.headList().size(), 16);

    // FixtureGroup should listen to Doc's fixtureRemoved() signal
    m_doc->deleteFixture(10);
    QCOMPARE(grp.headList().size(), 15);
    QVERIFY(grp.headHash().contains(QLCPoint(2, 2)) == false);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(5);
    m_doc->addFixture(fxi, 69);
    QCOMPARE(fxi->id(), quint32(69));

    // Uninteresting fixture removed (not part of group)
    m_doc->deleteFixture(69);
    QCOMPARE(grp.headList().size(), 15);
}

void FixtureGroup_Test::swap()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 4));
    for (quint32 id = 0; id < 16; id++)
    {
        Fixture* fxi = new Fixture(m_doc);
        fxi->setChannels(1);
        m_doc->addFixture(fxi);
        grp.assignFixture(fxi->id());
    }
    QCOMPARE(grp.headList().size(), 16);

    QLCPoint pt1(0, 0);
    QLCPoint pt2(2, 1);
    QVERIFY(grp.headHash().contains(pt1) == true);
    QVERIFY(grp.headHash().contains(pt2) == true);
    QCOMPARE(grp.headHash()[pt1], GroupHead(0, 0));
    QCOMPARE(grp.headHash()[pt2], GroupHead(6, 0));

    // Switch places with two fixtures
    grp.swap(pt1, pt2);
    QVERIFY(grp.headHash().contains(pt1) == true);
    QVERIFY(grp.headHash().contains(pt2) == true);
    QCOMPARE(grp.headHash()[pt1], GroupHead(6, 0));
    QCOMPARE(grp.headHash()[pt2], GroupHead(0, 0));

    // Switch places with a fixture and an empty point
    pt2 = QLCPoint(500, 500);
    grp.swap(pt1, pt2);
    QVERIFY(grp.headHash().contains(pt1) == false);
    QVERIFY(grp.headHash().contains(pt2) == true);
    QCOMPARE(grp.headHash()[pt2], GroupHead(6, 0));

    // ...and back again
    grp.swap(pt1, pt2);
    QVERIFY(grp.headHash().contains(pt1) == true);
    QVERIFY(grp.headHash().contains(pt2) == false);
    QCOMPARE(grp.headHash()[pt1], GroupHead(6, 0));
}

void FixtureGroup_Test::copy()
{
    FixtureGroup grp1(m_doc);
    grp1.setSize(QSize(4, 4));
    grp1.setName("Pertti Pasanen");
    grp1.setId(99);
    for (quint32 id = 0; id < 16; id++)
    {
        Fixture* fxi = new Fixture(m_doc);
        fxi->setChannels(1);
        m_doc->addFixture(fxi);
        grp1.assignFixture(fxi->id());
    }
    QCOMPARE(grp1.fixtureList().size(), 16);

    FixtureGroup grp2(m_doc);
    grp2.copyFrom(&grp1);
    QCOMPARE(grp2.size(), QSize(4, 4));
    QCOMPARE(grp2.name(), QString("Pertti Pasanen"));
    QVERIFY(grp2.id() != quint32(99)); // ID must not be copied
    QCOMPARE(grp2.fixtureList().size(), 16);
    for (quint32 id = 0; id < 16; id++)
        QVERIFY(grp2.fixtureList().contains(id) == true);
}

void FixtureGroup_Test::loadWrongID()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("FixtureGroup");
    root.setAttribute("ID", "Pertti");
    doc.appendChild(root);

    FixtureGroup grp(m_doc);
    QVERIFY(grp.loadXML(root) == false);
}

void FixtureGroup_Test::loadWrongHeadAttributes()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("FixtureGroup");
    root.setAttribute("ID", "12345");
    doc.appendChild(root);

    QDomElement head = doc.createElement("Head");
    head.setAttribute("X", "Seppo");
    head.setAttribute("Y", "0");
    head.setAttribute("Fixture", "42");
    QDomText headText = doc.createTextNode("0");
    head.appendChild(headText);
    root.appendChild(head);

    FixtureGroup grp(m_doc);
    QVERIFY(grp.loadXML(root) == true);
    QCOMPARE(grp.headHash().size(), 0);

    head.setAttribute("X", "0");
    head.setAttribute("Y", "Pertti");

    QVERIFY(grp.loadXML(root) == true);
    QCOMPARE(grp.headHash().size(), 0);

    head.setAttribute("Y", "0");
    head.setAttribute("Fixture", "Jorma");

    QVERIFY(grp.loadXML(root) == true);
    QCOMPARE(grp.headHash().size(), 0);

    head.setAttribute("Fixture", "42");
    headText.setData("Esko");

    QVERIFY(grp.loadXML(root) == true);
    QCOMPARE(grp.headHash().size(), 0);

    headText.setData("0");

    QVERIFY(grp.loadXML(root) == true);
    QCOMPARE(grp.headHash().size(), 1);
}

void FixtureGroup_Test::load()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 5));
    grp.setName("Pertti Pasanen");
    grp.setId(99);
    for (quint32 id = 0; id < 32; id++)
    {
        Fixture* fxi = new Fixture(m_doc);
        fxi->setChannels(1);
        m_doc->addFixture(fxi);
        grp.assignFixture(fxi->id());
    }

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    QVERIFY(grp.saveXML(&doc, &root) == true);

    // Extra garbage
    QDomElement bar = doc.createElement("Bar");
    root.firstChild().appendChild(bar);

    QVERIFY(FixtureGroup::loader(root, m_doc) == false);

    QDomElement tag = root.firstChild().toElement();
    QVERIFY(FixtureGroup::loader(tag, m_doc) == true);
    QCOMPARE(m_doc->fixtureGroups().size(), 1);
    FixtureGroup* grp2 = m_doc->fixtureGroup(99);
    QVERIFY(grp2 != NULL);
    QCOMPARE(grp2->size(), QSize(4, 5));
    QCOMPARE(grp2->name(), QString("Pertti Pasanen"));
    QCOMPARE(grp2->id(), quint32(99));
    QCOMPARE(grp2->headHash(), grp.headHash());
}

void FixtureGroup_Test::save()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 5));
    grp.setName("Pertti Pasanen");
    grp.setId(99);
    for (quint32 id = 0; id < 32; id++)
    {
        Fixture* fxi = new Fixture(m_doc);
        fxi->setChannels(1);
        m_doc->addFixture(fxi);
        grp.assignFixture(fxi->id());
    }

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");

    QVERIFY(grp.saveXML(&doc, &root) == true);
    QDomElement tag = root.firstChild().toElement();
    QCOMPARE(tag.tagName(), QString("FixtureGroup"));
    QCOMPARE(tag.attribute("ID"), QString("99"));

    int size = 0, name = 0, fixture = 0;

    QDomNode node = tag.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Size")
        {
            QCOMPARE(tag.attribute("X").toInt(), 4);
            QCOMPARE(tag.attribute("Y").toInt(), 5);
            size++;
        }
        else if (tag.tagName() == "Name")
        {
            QCOMPARE(tag.text(), QString("Pertti Pasanen"));
            name++;
        }
        else if (tag.tagName() == "Head")
        {
            quint32 id = tag.attribute("Fixture").toUInt();
            int head = tag.text().toInt();
            QLCPoint pt(tag.attribute("X").toInt(), tag.attribute("Y").toInt());
            QCOMPARE(grp.head(pt), GroupHead(id, head));
            fixture++;
        }
        else
        {
            QFAIL(QString("Unexpected tag in FixtureGroup: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(size, 1);
    QCOMPARE(name, 1);
    QCOMPARE(fixture, 32);
}

QTEST_APPLESS_MAIN(FixtureGroup_Test)
