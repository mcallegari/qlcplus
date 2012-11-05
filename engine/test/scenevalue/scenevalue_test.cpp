/*
  Q Light Controller - Unit test
  scenevalue_test.cpp

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

#include "scenevalue_test.h"
#include "scenevalue.h"

void SceneValue_Test::initial()
{
    SceneValue scv;
    QVERIFY(scv.isValid() == false);
    QVERIFY(scv.fxi == Fixture::invalidId());
    QVERIFY(scv.channel == QLCChannel::invalid());
    QVERIFY(scv.value == 0);

    SceneValue scv2(31337, 5150, 42);
    QVERIFY(scv2.isValid() == true);
    QVERIFY(scv2.fxi == 31337);
    QVERIFY(scv2.channel == 5150);
    QVERIFY(scv2.value == 42);
}

void SceneValue_Test::lessThan()
{
    SceneValue scv1;
    SceneValue scv2;

    /* Both have invalid ID and channel; neither is less than the other */
    QVERIFY((scv1 < scv2) == false);
    QVERIFY((scv2 < scv1) == false);

    /* Both save same ID, invalid channel; same here */
    scv1.fxi = 0;
    scv2.fxi = 0;
    QVERIFY((scv1 < scv2) == false);
    QVERIFY((scv2 < scv1) == false);

    /* Same ID, different channel */
    scv1.channel = 0;
    scv2.channel = 1;
    QVERIFY((scv1 < scv2) == true);
    QVERIFY((scv2 < scv1) == false);

    /* Different ID, different channel */
    scv1.fxi = 1;
    scv2.fxi = 0;
    QVERIFY((scv1 < scv2) == false);
    QVERIFY((scv2 < scv1) == true);

    /* Different ID, same channel */
    scv1.channel = 1;
    scv2.channel = 1;
    QVERIFY((scv1 < scv2) == false);
    QVERIFY((scv2 < scv1) == true);
}

void SceneValue_Test::loadSuccess()
{
    QDomDocument doc;

    QDomElement val = doc.createElement("Value");
    val.setAttribute("Fixture", 5);
    val.setAttribute("Channel", 60);
    QDomText valText = doc.createTextNode("100");
    val.appendChild(valText);

    SceneValue scv;
    QVERIFY(scv.loadXML(val) == true);
    QVERIFY(scv.fxi == 5);
    QVERIFY(scv.channel == 60);
    QVERIFY(scv.value == 100);
}

void SceneValue_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement val = doc.createElement("Valu");
    val.setAttribute("Fixture", 5);
    val.setAttribute("Channel", 60);
    QDomText valText = doc.createTextNode("100");
    val.appendChild(valText);

    SceneValue scv;
    QVERIFY(scv.loadXML(val) == false);
}

void SceneValue_Test::loadWrongFixture()
{
    QDomDocument doc;

    QDomElement val = doc.createElement("Value");
    val.setAttribute("Fixture", Fixture::invalidId());
    val.setAttribute("Channel", 60);
    QDomText valText = doc.createTextNode("100");
    val.appendChild(valText);

    SceneValue scv;
    QVERIFY(scv.loadXML(val) == false);
}

void SceneValue_Test::loadWrongValue()
{
    QDomDocument doc;

    QDomElement val = doc.createElement("Value");
    val.setAttribute("Fixture", 5);
    val.setAttribute("Channel", 60);
    QDomText valText = doc.createTextNode("257");
    val.appendChild(valText);

    SceneValue scv;
    QVERIFY(scv.loadXML(val) == true);
    QVERIFY(scv.value == 1);
}

void SceneValue_Test::save()
{
    QDomDocument doc;
    QDomElement root;
    QDomElement tag;

    root = doc.createElement("Test");

    SceneValue scv(4, 8, 16);
    QVERIFY(scv.saveXML(&doc, &root) == true);

    tag = root.firstChild().toElement();
    QVERIFY(tag.tagName() == "Value");
    QVERIFY(tag.attribute("Fixture").toInt() == 4);
    QVERIFY(tag.attribute("Channel").toInt() == 8);
    QVERIFY(tag.text().toInt() == 16);
}

QTEST_APPLESS_MAIN(SceneValue_Test)
