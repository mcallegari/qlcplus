/*
  Q Light Controller - Unit test
  cue_test.cpp

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

#include "cue_test.h"
#include "cue.h"

void Cue_Test::initial()
{
    Cue cue;
    QCOMPARE(cue.name(), QString());
    QCOMPARE(cue.values().size(), 0);
    QCOMPARE(cue.fadeInSpeed(), uint(0));
    QCOMPARE(cue.fadeOutSpeed(), uint(0));
    QCOMPARE(cue.duration(), uint(0));

    cue = Cue("Foo");
    QCOMPARE(cue.name(), QString("Foo"));
    QCOMPARE(cue.values().size(), 0);

    QHash <uint,uchar> values;
    values[0] = 14;
    values[932] = 5;
    values[5] = 255;
    cue = Cue(values);
    QCOMPARE(cue.name(), QString());
    QCOMPARE(cue.values().size(), 3);
    QCOMPARE(cue.values()[0], uchar(14));
    QCOMPARE(cue.values()[932], uchar(5));
    QCOMPARE(cue.values()[5], uchar(255));
}

void Cue_Test::name()
{
    Cue cue;
    cue.setName("Foobar");
    QCOMPARE(cue.name(), QString("Foobar"));
}

void Cue_Test::value()
{
    Cue cue;
    QCOMPARE(cue.value(0), uchar(0));
    QCOMPARE(cue.value(UINT_MAX), uchar(0));
    QCOMPARE(cue.value(12345), uchar(0));

    cue.setValue(0, 15);
    QCOMPARE(cue.values().size(), 1);
    QCOMPARE(cue.value(0), uchar(15));

    cue.setValue(0, 15);
    QCOMPARE(cue.values().size(), 1);
    QCOMPARE(cue.value(0), uchar(15));

    cue.setValue(UINT_MAX, 42);
    QCOMPARE(cue.values().size(), 2);
    QCOMPARE(cue.value(0), uchar(15));
    QCOMPARE(cue.value(UINT_MAX), uchar(42));

    cue.unsetValue(0);
    QCOMPARE(cue.values().size(), 1);
    QCOMPARE(cue.value(UINT_MAX), uchar(42));

    cue.unsetValue(0);
    QCOMPARE(cue.values().size(), 1);
    QCOMPARE(cue.value(UINT_MAX), uchar(42));
}

void Cue_Test::copy()
{
    Cue cue1("Foo");
    cue1.setValue(0, 1);
    cue1.setValue(1, 2);
    cue1.setValue(2, 3);
    cue1.setFadeInSpeed(10);
    cue1.setFadeOutSpeed(20);
    cue1.setDuration(30);

    Cue cue2 = cue1;
    QCOMPARE(cue2.name(), QString("Foo"));
    QCOMPARE(cue2.values().size(), 3);
    QCOMPARE(cue2.value(0), uchar(1));
    QCOMPARE(cue2.value(1), uchar(2));
    QCOMPARE(cue2.value(2), uchar(3));
    QCOMPARE(cue2.fadeInSpeed(), uint(10));
    QCOMPARE(cue2.fadeOutSpeed(), uint(20));
    QCOMPARE(cue2.duration(), uint(30));
}

void Cue_Test::save()
{
    Cue cue("Foo");
    cue.setValue(0, 15);
    cue.setValue(31337, 255);
    cue.setValue(42, 127);
    cue.setFadeInSpeed(10);
    cue.setFadeOutSpeed(20);
    cue.setDuration(30);

    QDomDocument doc;
    QDomElement root = doc.createElement("Bar");
    doc.appendChild(root);

    QVERIFY(cue.saveXML(&doc, &root) == true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("Cue"));
    QCOMPARE(root.firstChild().toElement().attribute("Name"), QString("Foo"));

    int value = 0, speed = 0;

    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Value")
        {
            value++;
            QString ch = tag.attribute("Channel");
            QVERIFY(ch.isEmpty() == false);
            if (ch.toUInt() == 0)
                QCOMPARE(tag.text().toInt(), 15);
            else if (ch.toUInt() == 42)
                QCOMPARE(tag.text().toInt(), 127);
            else if (ch.toUInt() == 31337)
                QCOMPARE(tag.text().toInt(), 255);
            else
                QFAIL(QString("Unexpected channel in value tag: %1").arg(ch).toUtf8().constData());
        }
        else if (tag.tagName() == "Speed")
        {
            speed++;
            QCOMPARE(tag.attribute("FadeIn").toInt(), 10);
            QCOMPARE(tag.attribute("FadeOut").toInt(), 20);
            QCOMPARE(tag.attribute("Duration").toInt(), 30);
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }
        node = node.nextSibling();
    }

    QCOMPARE(value, 3);
    QCOMPARE(speed, 1);
}

void Cue_Test::load()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Cue");
    root.setAttribute("Name", "Baz");
    doc.appendChild(root);

    QDomElement value;
    QDomText valueText;

    value = doc.createElement("Value");
    value.setAttribute("Channel", "1");
    valueText = doc.createTextNode("127");
    value.appendChild(valueText);
    root.appendChild(value);

    value = doc.createElement("Value");
    value.setAttribute("Channel", "42");
    valueText = doc.createTextNode("255");
    value.appendChild(valueText);
    root.appendChild(value);

    value = doc.createElement("Value");
    value.setAttribute("Channel", "69");
    valueText = doc.createTextNode("0");
    value.appendChild(valueText);
    root.appendChild(value);

    QDomElement speed = doc.createElement("Speed");
    speed.setAttribute("FadeIn", 100);
    speed.setAttribute("FadeOut", 200);
    speed.setAttribute("Duration", 300);
    root.appendChild(speed);

    // Extra garbage
    value = doc.createElement("Foo");
    value.setAttribute("Channel", "69");
    valueText = doc.createTextNode("0");
    value.appendChild(valueText);
    root.appendChild(value);

    Cue cue;
    QVERIFY(cue.loadXML(value) == false);
    QVERIFY(cue.loadXML(root) == true);
    QCOMPARE(cue.name(), QString("Baz"));
    QCOMPARE(cue.values().size(), 3);
    QCOMPARE(cue.value(0), uchar(0));
    QCOMPARE(cue.value(1), uchar(127));
    QCOMPARE(cue.value(42), uchar(255));
    QCOMPARE(cue.value(69), uchar(0));
    QCOMPARE(cue.fadeInSpeed(), uint(100));
    QCOMPARE(cue.fadeOutSpeed(), uint(200));
    QCOMPARE(cue.duration(), uint(300));
}

QTEST_APPLESS_MAIN(Cue_Test)
