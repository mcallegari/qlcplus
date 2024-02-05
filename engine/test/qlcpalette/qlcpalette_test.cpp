/*
  Q Light Controller Plus - Unit tests
  qlcpalette_test.cpp

  Copyright (C) Massimo Callegari

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

#include "qlcpalette_test.h"
#include "qlcpalette.h"

void QLCPalette_Test::initialization()
{
    QLCPalette p(QLCPalette::Undefined);

    QVERIFY(p.type() == QLCPalette::Undefined);
    QVERIFY(p.id() == QLCPalette::invalidId());
    QVERIFY(p.fanningType() == QLCPalette::Flat);
    QVERIFY(p.fanningLayout() == QLCPalette::XAscending);
    QVERIFY(p.fanningAmount() == 100);
    QVERIFY(p.fanningValue() == QVariant());

    p.setID(42);
    QVERIFY(p.id() == 42);

    QVERIFY(p.name() == QString());
    p.setName("My Palette");
    QVERIFY(p.name() == QString("My Palette"));
}

void QLCPalette_Test::type()
{
    QVERIFY(QLCPalette::typeToString(QLCPalette::Undefined) == QString());
    QVERIFY(QLCPalette::typeToString(QLCPalette::Dimmer) == QString("Dimmer"));
    QVERIFY(QLCPalette::typeToString(QLCPalette::Color) == QString("Color"));
    QVERIFY(QLCPalette::typeToString(QLCPalette::Pan) == QString("Pan"));
    QVERIFY(QLCPalette::typeToString(QLCPalette::Tilt) == QString("Tilt"));
    QVERIFY(QLCPalette::typeToString(QLCPalette::PanTilt) == QString("PanTilt"));
    QVERIFY(QLCPalette::typeToString(QLCPalette::Shutter) == QString("Shutter"));
    QVERIFY(QLCPalette::typeToString(QLCPalette::Gobo) == QString("Gobo"));

    QVERIFY(QLCPalette::stringToType("Foo") == QLCPalette::Undefined);
    QVERIFY(QLCPalette::stringToType("Dimmer") == QLCPalette::Dimmer);
    QVERIFY(QLCPalette::stringToType("Color") == QLCPalette::Color);
    QVERIFY(QLCPalette::stringToType("Pan") == QLCPalette::Pan);
    QVERIFY(QLCPalette::stringToType("Tilt") == QLCPalette::Tilt);
    QVERIFY(QLCPalette::stringToType("PanTilt") == QLCPalette::PanTilt);
    QVERIFY(QLCPalette::stringToType("Shutter") == QLCPalette::Shutter);
    QVERIFY(QLCPalette::stringToType("Gobo") == QLCPalette::Gobo);
}

void QLCPalette_Test::icon()
{
    QLCPalette p1(QLCPalette::Dimmer);
    QCOMPARE(p1.iconResource(), QString(":/intensity.png"));
    QLCPalette p2(QLCPalette::Color);
    QCOMPARE(p2.iconResource(), QString(":/color.png"));
    QLCPalette p3(QLCPalette::Pan);
    QCOMPARE(p3.iconResource(), QString(":/pan.png"));
    QLCPalette p4(QLCPalette::Tilt);
    QCOMPARE(p4.iconResource(), QString(":/tilt.png"));
    QLCPalette p5(QLCPalette::PanTilt);
    QCOMPARE(p5.iconResource(true), QString("qrc:/position.svg"));
    QLCPalette p6(QLCPalette::Shutter);
    QCOMPARE(p6.iconResource(), QString(":/shutter.png"));
    QLCPalette p7(QLCPalette::Gobo);
    QCOMPARE(p7.iconResource(), QString(":/gobo.png"));
}

void QLCPalette_Test::value()
{
    /* test one single integer value */
    QLCPalette p1(QLCPalette::Dimmer);

    QVERIFY(p1.values().count() == 0);

    p1.setValue(128);
    QVERIFY(p1.values().count() == 1);
    QVERIFY(p1.value().toInt() == 128);

    /* test composite color value */
    QLCPalette p2(QLCPalette::Color);
    p2.setValue(QLCPalette::colorToString(QColor(0x11, 0x22, 0x33), QColor(0x44, 0x55, 0x66)));
    QVERIFY(p1.values().count() == 1);

    QColor rgb, wauv;
    QVERIFY(p2.value().toString() == "#112233445566");
    QVERIFY(QLCPalette::stringToColor(p2.value().toString(), rgb, wauv) == true);
    QVERIFY(rgb == QColor(0x11, 0x22, 0x33));
    QVERIFY(wauv == QColor(0x44, 0x55, 0x66));

    /* test 2 integer values */
    QLCPalette p3(QLCPalette::PanTilt);
    p3.setValue(180, 90);

    QVERIFY(p3.values().count() == 2);
    QVERIFY(p3.values().at(0).toInt() == 180);
    QVERIFY(p3.values().at(1).toInt() == 90);
}

void QLCPalette_Test::fanning()
{
    QVERIFY(QLCPalette::fanningTypeToString(QLCPalette::Flat) == "Flat");
    QVERIFY(QLCPalette::fanningTypeToString(QLCPalette::Linear) == "Linear");
    QVERIFY(QLCPalette::fanningTypeToString(QLCPalette::Sine) == "Sine");
    QVERIFY(QLCPalette::fanningTypeToString(QLCPalette::Square) == "Square");
    QVERIFY(QLCPalette::fanningTypeToString(QLCPalette::Saw) == "Saw");

    QVERIFY(QLCPalette::stringToFanningType("Foo") == QLCPalette::Flat);
    QVERIFY(QLCPalette::stringToFanningType("Flat") == QLCPalette::Flat);
    QVERIFY(QLCPalette::stringToFanningType("Linear") == QLCPalette::Linear);
    QVERIFY(QLCPalette::stringToFanningType("Sine") == QLCPalette::Sine);
    QVERIFY(QLCPalette::stringToFanningType("Square") == QLCPalette::Square);
    QVERIFY(QLCPalette::stringToFanningType("Saw") == QLCPalette::Saw);

    QVERIFY(QLCPalette::fanningLayoutToString(QLCPalette::XAscending) == "XAscending");
    QVERIFY(QLCPalette::fanningLayoutToString(QLCPalette::XDescending) == "XDescending");
    QVERIFY(QLCPalette::fanningLayoutToString(QLCPalette::XCentered) == "XCentered");
    QVERIFY(QLCPalette::fanningLayoutToString(QLCPalette::YAscending) == "YAscending");
    QVERIFY(QLCPalette::fanningLayoutToString(QLCPalette::YDescending) == "YDescending");
    QVERIFY(QLCPalette::fanningLayoutToString(QLCPalette::YCentered) == "YCentered");
    QVERIFY(QLCPalette::fanningLayoutToString(QLCPalette::ZAscending) == "ZAscending");
    QVERIFY(QLCPalette::fanningLayoutToString(QLCPalette::ZDescending) == "ZDescending");
    QVERIFY(QLCPalette::fanningLayoutToString(QLCPalette::ZCentered) == "ZCentered");

    QVERIFY(QLCPalette::stringToFanningLayout("Foo") == QLCPalette::XAscending);
    QVERIFY(QLCPalette::stringToFanningLayout("XAscending") == QLCPalette::XAscending);
    QVERIFY(QLCPalette::stringToFanningLayout("XDescending") == QLCPalette::XDescending);
    QVERIFY(QLCPalette::stringToFanningLayout("XCentered") == QLCPalette::XCentered);
    QVERIFY(QLCPalette::stringToFanningLayout("YAscending") == QLCPalette::YAscending);
    QVERIFY(QLCPalette::stringToFanningLayout("YDescending") == QLCPalette::YDescending);
    QVERIFY(QLCPalette::stringToFanningLayout("YCentered") == QLCPalette::YCentered);
    QVERIFY(QLCPalette::stringToFanningLayout("ZAscending") == QLCPalette::ZAscending);
    QVERIFY(QLCPalette::stringToFanningLayout("ZDescending") == QLCPalette::ZDescending);
    QVERIFY(QLCPalette::stringToFanningLayout("ZCentered") == QLCPalette::ZCentered);

    QLCPalette p(QLCPalette::Dimmer);
    p.setFanningAmount(75);

    QVERIFY(p.fanningAmount() == 75);
}

void QLCPalette_Test::colorHelpers()
{
    QColor rgb(0xAA, 0xBB, 0xCC);
    QColor wauv(0x11, 0x22, 0x33);

    QVERIFY(QLCPalette::colorToString(rgb, wauv) == QString("#aabbcc112233"));

    QVERIFY(QLCPalette::stringToColor("#invalid", rgb, wauv) == false);
    QVERIFY(QLCPalette::stringToColor("#11deadbeef22", rgb, wauv) == true);

    QVERIFY(rgb.red() == 0x11);
    QVERIFY(rgb.green() == 0xDE);
    QVERIFY(rgb.blue() == 0xAD);

    QVERIFY(wauv.red() == 0xBE);
    QVERIFY(wauv.green() == 0xEF);
    QVERIFY(wauv.blue() == 0x22);
}

void QLCPalette_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Palette");
    xmlWriter.writeAttribute("ID", "1");
    xmlWriter.writeAttribute("Type", "Color");
    xmlWriter.writeAttribute("Name", "Lavender");
    xmlWriter.writeAttribute("Value", "#AABBCCDDEEFF");
    xmlWriter.writeAttribute("Fan", "Linear");
    xmlWriter.writeAttribute("Layout", "LeftToRight");
    xmlWriter.writeAttribute("Amount", "42");
    xmlWriter.writeAttribute("FanValue", "#00ff00");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCPalette p(QLCPalette::Undefined);
    QVERIFY(p.loadXML(xmlReader) == true);

    QVERIFY(p.id() == 1);
    QVERIFY(p.type() == QLCPalette::Color);
    QVERIFY(p.name() == "Lavender");
    QVERIFY(p.value().toString() == "#AABBCCDDEEFF");
    QVERIFY(p.fanningType() == QLCPalette::Linear);
    QVERIFY(p.fanningLayout() == QLCPalette::XAscending);
    QVERIFY(p.fanningAmount() == 42);
    QVERIFY(p.fanningValue().toString() == "#00ff00");
}

void QLCPalette_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Politte");
    xmlWriter.writeAttribute("ID", "42");
    xmlWriter.writeAttribute("Type", "Dimmer");
    xmlWriter.writeAttribute("Name", "Just wrong");
    xmlWriter.writeAttribute("Value", "42");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCPalette p(QLCPalette::Undefined);
    QVERIFY(p.loadXML(xmlReader) == false);
}

void QLCPalette_Test::save()
{
    QLCPalette p(QLCPalette::PanTilt);
    p.setID(3);
    p.setName("Center up");
    p.setValue(90, 145);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(p.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "Palette");

    QVERIFY(xmlReader.attributes().value("ID").toString() == "3");
    QVERIFY(xmlReader.attributes().value("Name").toString() == "Center up");
    QVERIFY(xmlReader.attributes().value("Type").toString() == "PanTilt");
    QVERIFY(xmlReader.attributes().value("Value").toString() == "90,145");
}

QTEST_APPLESS_MAIN(QLCPalette_Test)
