/*
  Q Light Controller - Unit test
  bus_test.cpp

  Copyright (c) Heikki Junnila

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

#include "bus_test.h"
#include "bus.h"

void Bus_Test::initTestCase()
{
    QVERIFY(Bus::instance() == NULL);
    Bus::init(this);
    QVERIFY(Bus::instance() != NULL);
}

void Bus_Test::defaults()
{
    QVERIFY(Bus::count() == 32);
    QVERIFY(Bus::defaultFade() == 0);
    QVERIFY(Bus::defaultHold() == 1);
    QCOMPARE(Bus::defaultPalette(), quint32(31));
    QVERIFY(Bus::instance()->name(0) == QString("Fade"));
    QVERIFY(Bus::instance()->name(1) == QString("Hold"));
    QCOMPARE(Bus::instance()->name(31), QString("Palette"));
}

void Bus_Test::value()
{
    QSignalSpy spy(Bus::instance(), SIGNAL(valueChanged(quint32,quint32)));

    /* Setting bus value should produce a signal */
    Bus::instance()->setValue(0, 15);
    QVERIFY(Bus::instance()->value(0) == 15);
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.at(0).count() == 2);
    QVERIFY(spy.at(0).at(0).toUInt() == 0);
    QVERIFY(spy.at(0).at(1).toUInt() == 15);

    /* Another bus should change only one bus value */
    Bus::instance()->setValue(5, 30);
    QVERIFY(Bus::instance()->value(0) == 15);
    QVERIFY(Bus::instance()->value(5) == 30);
    QVERIFY(spy.count() == 2);
    QVERIFY(spy.at(1).at(0).toUInt() == 5);
    QVERIFY(spy.at(1).at(1).toUInt() == 30);

    /* Invalid bus shouldn't produce signals */
    Bus::instance()->setValue(Bus::count(), 30);
    QVERIFY(Bus::instance()->value(0) == 15);
    QVERIFY(Bus::instance()->value(5) == 30);
    QVERIFY(spy.count() == 2);

    /* Invalid bus shouldn't produce signals */
    Bus::instance()->setValue(0, UINT_MAX);
    QVERIFY(Bus::instance()->value(0) == UINT_MAX);
    QVERIFY(Bus::instance()->value(5) == 30);
    QVERIFY(spy.count() == 3);
    QVERIFY(spy.at(2).at(0).toUInt() == 0);
    QVERIFY(spy.at(2).at(1).toUInt() == UINT_MAX);
}

void Bus_Test::name()
{
    QSignalSpy spy(Bus::instance(), SIGNAL(nameChanged(quint32,QString)));

    /* Setting bus name should produce a signal */
    QVERIFY(Bus::instance()->name(0) == QString("Fade"));
    Bus::instance()->setName(0, "Foo");
    QVERIFY(Bus::instance()->name(0) == QString("Foo"));
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.at(0).at(0).toUInt() == 0);
    QVERIFY(spy.at(0).at(1).toString() == QString("Foo"));

    /* Invalid bus should not produce a signal */
    Bus::instance()->setName(5000, "Bar");
    QVERIFY(spy.count() == 1);
}

void Bus_Test::idName()
{
    Bus::instance()->setName(0, "Fade");

    QCOMPARE(Bus::instance()->idName(0), QString("Fade"));
    QCOMPARE(Bus::instance()->idName(1), QString("Hold"));
    QCOMPARE(Bus::instance()->idName(5), QString("Bus 6"));
    QCOMPARE(Bus::instance()->idName(Bus::count()), QString());

    QStringList idNames(Bus::instance()->idNames());
    QCOMPARE(idNames.size(), int(Bus::count()));
    QCOMPARE(idNames[0], QString("Fade"));
    QCOMPARE(idNames[1], QString("Hold"));
    QCOMPARE(idNames[5], QString("Bus 6"));
}

void Bus_Test::tap()
{
    QSignalSpy spy(Bus::instance(), SIGNAL(tapped(quint32)));

    /* Tapping an existing bus should produce a signal */
    Bus::instance()->tap(17);
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.at(0).at(0).toUInt() == 17);

    /* Tapping a non-existing bus should produce a signal */
    Bus::instance()->tap(6342);
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.at(0).at(0).toUInt() == 17);
}

void Bus_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Bush");
    doc.appendChild(root);
    QVERIFY(Bus::instance()->loadXML(root) == false);
}

void Bus_Test::load()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Bus");
    doc.appendChild(root);
    root.setAttribute("ID", 0);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foo");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement value = doc.createElement("Value");
    QDomText valueText = doc.createTextNode("142");
    value.appendChild(valueText);
    root.appendChild(value);

    QDomElement foo = doc.createElement("Foobar");
    QDomText fooText = doc.createTextNode("Xyzzy");
    foo.appendChild(fooText);
    root.appendChild(foo);

    QVERIFY(Bus::instance()->loadXML(root) == true);
    QVERIFY(Bus::instance()->value(0) == 142);
    QVERIFY(Bus::instance()->name(0) == "Foo");
}

void Bus_Test::loadWrongID()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Bus");
    doc.appendChild(root);
    root.setAttribute("ID", Bus::count());

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foo");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement value = doc.createElement("Value");
    QDomText valueText = doc.createTextNode("142");
    value.appendChild(valueText);
    root.appendChild(value);

    QVERIFY(Bus::instance()->loadXML(root) == false);
    QVERIFY(Bus::instance()->value(0) == 142);
    QVERIFY(Bus::instance()->name(0) == "Foo");
}

void Bus_Test::cleanupTestCase()
{
    QVERIFY(Bus::instance() != NULL);
    delete Bus::instance();
    QVERIFY(Bus::instance() == NULL);
}

QTEST_APPLESS_MAIN(Bus_Test)
