/*
  Q Light Controller
  vcxypadfixtureeditor_test.cpp

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QtTest>
#include <QtXml>

#define private public
#define protected public
#include "vcxypadfixtureeditor.h"
#undef private
#undef protected

#include "vcxypadfixtureeditor_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universearray.h"
#include "qlcfile.h"
#include "doc.h"

#define INTERNAL_FIXTUREDIR "../../../fixtures/"

void VCXYPadFixtureEditor_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);

    const QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    const QLCFixtureMode* mode = def->modes()[0];
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setName("Fixture 1");
    fxi->setFixtureDefinition(def, mode);
    fxi->setAddress(0);

    fxi = new Fixture(m_doc);
    fxi->setName("Fixture 2");
    fxi->setFixtureDefinition(def, mode);
    fxi->setAddress(10);

    fxi = new Fixture(m_doc);
    fxi->setName("Fixture 3");
    fxi->setFixtureDefinition(def, mode);
    fxi->setAddress(20);
}

void VCXYPadFixtureEditor_Test::initial()
{
    QList <VCXYPadFixture> list;

    VCXYPadFixture fxi(m_doc);

    fxi.setFixture(0);
    fxi.setX(0.1, 0.2, false);
    fxi.setY(0.3, 0.4, true);
    list << fxi;

    fxi.setFixture(1);
    fxi.setX(0, 1, true);
    fxi.setY(0, 1, false);
    list << fxi;

    VCXYPadFixtureEditor fe(NULL, list);
    QCOMPARE(fe.fixtures(), list);
    QCOMPARE(fe.m_xMin->value(), 10);
    QCOMPARE(fe.m_xMax->value(), 20);
    QCOMPARE(fe.m_xReverse->isChecked(), false);
    QCOMPARE(fe.m_yMin->value(), 30);
    QCOMPARE(fe.m_yMax->value(), 40);
    QCOMPARE(fe.m_yReverse->isChecked(), true);

    list.clear();

    VCXYPadFixtureEditor fe2(NULL, list);
    QCOMPARE(fe2.fixtures().isEmpty(), true);
    QCOMPARE(fe2.m_xMin->value(), 0);
    QCOMPARE(fe2.m_xMax->value(), 100);
    QCOMPARE(fe2.m_xReverse->isChecked(), false);
    QCOMPARE(fe2.m_yMin->value(), 0);
    QCOMPARE(fe2.m_yMax->value(), 100);
    QCOMPARE(fe2.m_yReverse->isChecked(), false);
}

void VCXYPadFixtureEditor_Test::valueSlots()
{
    QList <VCXYPadFixture> list;
    VCXYPadFixtureEditor fe(NULL, list);

    fe.m_xMin->setValue(50);
    fe.m_xMax->setValue(20);
    QCOMPARE(fe.m_xMin->value(), 19);
    QCOMPARE(fe.m_xMax->value(), 20);

    fe.m_xMin->setValue(40);
    QCOMPARE(fe.m_xMin->value(), 40);
    QCOMPARE(fe.m_xMax->value(), 41);

    fe.m_yMin->setValue(50);
    fe.m_yMax->setValue(20);
    QCOMPARE(fe.m_yMin->value(), 19);
    QCOMPARE(fe.m_yMax->value(), 20);

    fe.m_yMin->setValue(40);
    QCOMPARE(fe.m_yMin->value(), 40);
    QCOMPARE(fe.m_yMax->value(), 41);
}

void VCXYPadFixtureEditor_Test::accept()
{
    QList <VCXYPadFixture> list;

    VCXYPadFixture fxi(m_doc);

    fxi.setFixture(0);
    fxi.setX(0, 1, false);
    fxi.setY(0, 1, false);
    list << fxi;

    fxi.setFixture(1);
    fxi.setX(0.5, 0.6, true);
    fxi.setY(0.5, 0.6, true);
    list << fxi;

    VCXYPadFixtureEditor fe(NULL, list);
    fe.m_xMin->setValue(10);
    fe.m_xMax->setValue(20);
    fe.m_yMin->setValue(30);
    fe.m_yMax->setValue(40);
    fe.accept();
    QCOMPARE(fe.m_xMin->value(), 10);
    QCOMPARE(fe.m_xMax->value(), 20);
    QCOMPARE(fe.m_yMin->value(), 30);
    QCOMPARE(fe.m_yMax->value(), 40);

    list = fe.fixtures();
    QCOMPARE(list[0].fixture(), quint32(0));
    QCOMPARE(list[0].xMin(), qreal(0.1));
    QCOMPARE(list[0].xMax(), qreal(0.2));
    QCOMPARE(list[0].yMin(), qreal(0.3));
    QCOMPARE(list[0].yMax(), qreal(0.4));
    QCOMPARE(list[1].fixture(), quint32(1));
    QCOMPARE(list[1].xMin(), qreal(0.1));
    QCOMPARE(list[1].xMax(), qreal(0.2));
    QCOMPARE(list[1].yMin(), qreal(0.3));
    QCOMPARE(list[1].yMax(), qreal(0.4));
}

void VCXYPadFixtureEditor_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

QTEST_MAIN(VCXYPadFixtureEditor_Test)
