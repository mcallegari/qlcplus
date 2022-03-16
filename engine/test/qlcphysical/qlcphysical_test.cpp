/*
  Q Light Controller Plus - Unit tests
  qlcphysical_test.cpp

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#include "qlcphysical_test.h"
#include "qlcphysical.h"

void QLCPhysical_Test::bulbType()
{
    QVERIFY(p.bulbType().isEmpty());
    p.setBulbType("BulbType");
    QVERIFY(p.bulbType() == "BulbType");
}

void QLCPhysical_Test::bulbLumens()
{
    QVERIFY(p.bulbLumens() == 0);
    p.setBulbLumens(10000);
    QVERIFY(p.bulbLumens() == 10000);
}

void QLCPhysical_Test::bulbColourTemp()
{
    QVERIFY(p.bulbColourTemperature() == 0);
    p.setBulbColourTemperature(3200);
    QVERIFY(p.bulbColourTemperature() == 3200);
}

void QLCPhysical_Test::weight()
{
    QVERIFY(p.weight() == 0);
    p.setWeight(7);
    QVERIFY(p.weight() == 7);
    p.setWeight(5.02837);
    QCOMPARE(p.weight(), 5.02837);
}

void QLCPhysical_Test::width()
{
    QVERIFY(p.width() == 0);
    p.setWidth(600);
    QVERIFY(p.width() == 600);
}

void QLCPhysical_Test::height()
{
    QVERIFY(p.height() == 0);
    p.setHeight(1200);
    QVERIFY(p.height() == 1200);
}

void QLCPhysical_Test::depth()
{
    QVERIFY(p.depth() == 0);
    p.setDepth(250);
    QVERIFY(p.depth() == 250);
}

void QLCPhysical_Test::lensName()
{
    QVERIFY(p.lensName() == "Other");
    p.setLensName("Fresnel");
    QVERIFY(p.lensName() == "Fresnel");
}

void QLCPhysical_Test::lensDegreesMin()
{
    QVERIFY(p.lensDegreesMin() == 0);
    p.setLensDegreesMin(9.4);
    QVERIFY(p.lensDegreesMin() == 9.4);
}

void QLCPhysical_Test::lensDegreesMax()
{
    QVERIFY(p.lensDegreesMax() == 0);
    p.setLensDegreesMax(40.5);
    QVERIFY(p.lensDegreesMax() == 40.5);
}

void QLCPhysical_Test::focusType()
{
    QVERIFY(p.focusType() == "Fixed");
    p.setFocusType("Head");
    QVERIFY(p.focusType() == "Head");
}

void QLCPhysical_Test::focusPanMax()
{
    QVERIFY(p.focusPanMax() == 0);
    p.setFocusPanMax(540);
    QVERIFY(p.focusPanMax() == 540);
}

void QLCPhysical_Test::focusTiltMax()
{
    QVERIFY(p.focusTiltMax() == 0);
    p.setFocusTiltMax(270);
    QVERIFY(p.focusTiltMax() == 270);
}

void QLCPhysical_Test::layoutSize()
{
    QVERIFY(p.layoutSize() == QSize(1, 1));
    p.setLayoutSize(QSize(6, 3));
    QVERIFY(p.layoutSize() == QSize(6, 3));
}

void QLCPhysical_Test::powerConsumption()
{
    QVERIFY(p.powerConsumption() == 0);
    p.setPowerConsumption(24000);
    QVERIFY(p.powerConsumption() == 24000);
}
void QLCPhysical_Test::dmxConnector()
{
    QVERIFY(p.dmxConnector() == "5-pin");
    p.setDmxConnector("3-pin");
    QVERIFY(p.dmxConnector() == "3-pin");
}

void QLCPhysical_Test::copy()
{
    QLCPhysical c = p;
    QVERIFY(c.bulbType() == p.bulbType());
    QVERIFY(c.bulbLumens() == p.bulbLumens());
    QVERIFY(c.bulbColourTemperature() == p.bulbColourTemperature());
    QVERIFY(c.weight() == p.weight());
    QVERIFY(c.width() == p.width());
    QVERIFY(c.height() == p.height());
    QVERIFY(c.depth() == p.depth());
    QVERIFY(c.lensName() == p.lensName());
    QVERIFY(c.lensDegreesMin() == p.lensDegreesMin());
    QVERIFY(c.lensDegreesMax() == p.lensDegreesMax());
    QVERIFY(c.focusType() == p.focusType());
    QVERIFY(c.focusPanMax() == p.focusPanMax());
    QVERIFY(c.focusTiltMax() == p.focusTiltMax());
    QVERIFY(c.powerConsumption() == p.powerConsumption());
    QVERIFY(c.dmxConnector() == p.dmxConnector());
}

void QLCPhysical_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Physical");

    /* Bulb */
    xmlWriter.writeStartElement("Bulb");
    xmlWriter.writeAttribute("Type", "LED");
    xmlWriter.writeAttribute("Lumens", "18000");
    xmlWriter.writeAttribute("ColourTemperature", "6500");
    xmlWriter.writeEndElement();

    /* Dimensions */
    xmlWriter.writeStartElement("Dimensions");
    xmlWriter.writeAttribute("Weight", QString::number(39.4));
    xmlWriter.writeAttribute("Width", "530");
    xmlWriter.writeAttribute("Height", "320");
    xmlWriter.writeAttribute("Depth", "260");
    xmlWriter.writeEndElement();

    /* Lens */
    xmlWriter.writeStartElement("Lens");
    xmlWriter.writeAttribute("Name", "Fresnel");
    xmlWriter.writeAttribute("DegreesMin", "8");
    xmlWriter.writeAttribute("DegreesMax", "38");
    xmlWriter.writeEndElement();

    /* Focus */
    xmlWriter.writeStartElement("Focus");
    xmlWriter.writeAttribute("Type", "Head");
    xmlWriter.writeAttribute("PanMax", "520");
    xmlWriter.writeAttribute("TiltMax", "270");
    xmlWriter.writeEndElement();

    /* Technical */
    xmlWriter.writeStartElement("Technical");
    xmlWriter.writeAttribute("PowerConsumption", "250");
    xmlWriter.writeAttribute("DmxConnector", "5-pin");
    xmlWriter.writeEndElement();

    /* Unrecognized tag */
    xmlWriter.writeStartElement("HomerSimpson");
    xmlWriter.writeAttribute("BeerConsumption", "25000");
    xmlWriter.writeAttribute("PreferredBrand", "Duff");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(p.loadXML(xmlReader) == true);
    QVERIFY(p.bulbType() == "LED");
    QVERIFY(p.bulbLumens() == 18000);
    QVERIFY(p.bulbColourTemperature() == 6500);
    QVERIFY(p.weight() == 39.4);
    QVERIFY(p.width() == 530);
    QVERIFY(p.height() == 320);
    QVERIFY(p.depth() == 260);
    QVERIFY(p.lensName() == "Fresnel");
    QVERIFY(p.lensDegreesMin() == 8);
    QVERIFY(p.lensDegreesMax() == 38);
    QVERIFY(p.focusType() == "Head");
    QVERIFY(p.focusPanMax() == 520);
    QVERIFY(p.focusTiltMax() == 270);
    QVERIFY(p.powerConsumption() == 250);
    QVERIFY(p.dmxConnector() == "5-pin");
}

void QLCPhysical_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Foosical");

    /* Bulb */
    xmlWriter.writeStartElement("Bulb");
    xmlWriter.writeAttribute("Type", "LED");
    xmlWriter.writeAttribute("Lumens", "18000");
    xmlWriter.writeAttribute("ColourTemperature", "6500");
    xmlWriter.writeEndElement();

    /* Dimensions */
    xmlWriter.writeStartElement("Dimensions");
    xmlWriter.writeAttribute("Weight", QString::number(39.4));
    xmlWriter.writeAttribute("Width", "530");
    xmlWriter.writeAttribute("Height", "320");
    xmlWriter.writeAttribute("Depth", "260");
    xmlWriter.writeEndElement();

    /* Lens */
    xmlWriter.writeStartElement("Lens");
    xmlWriter.writeAttribute("Name", "Fresnel");
    xmlWriter.writeAttribute("DegreesMin", "8");
    xmlWriter.writeAttribute("DegreesMax", "38");
    xmlWriter.writeEndElement();

    /* Focus */
    xmlWriter.writeStartElement("Focus");
    xmlWriter.writeAttribute("Type", "Head");
    xmlWriter.writeAttribute("PanMax", "520");
    xmlWriter.writeAttribute("TiltMax", "270");
    xmlWriter.writeEndElement();

    /* Technical */
    xmlWriter.writeStartElement("Technical");
    xmlWriter.writeAttribute("PowerConsumption", "250");
    xmlWriter.writeAttribute("DmxConnector", "5-pin");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(p.loadXML(xmlReader) == false);
}

void QLCPhysical_Test::save()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    bool bulb = false, dim = false, lens = false, focus = false, layout = false, technical = false;

    QVERIFY(p.saveXML(&xmlWriter) == true);

    //qDebug() << buffer.buffer();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "Physical");

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Bulb")
        {
            bulb = true;
            QVERIFY(xmlReader.attributes().value("Type").toString() == "LED");
            QVERIFY(xmlReader.attributes().value("Lumens").toString() == "18000");
            QVERIFY(xmlReader.attributes().value("ColourTemperature").toString() == "6500");
        }
        else if (xmlReader.name().toString() == "Dimensions")
        {
            dim = true;
            QVERIFY(xmlReader.attributes().value("Width").toString() == "530");
            QVERIFY(xmlReader.attributes().value("Depth").toString() == "260");
            QVERIFY(xmlReader.attributes().value("Height").toString() == "320");
            QCOMPARE(xmlReader.attributes().value("Weight").toString().toDouble(), 39.4);
        }
        else if (xmlReader.name().toString() == "Lens")
        {
            lens = true;
            QVERIFY(xmlReader.attributes().value("Name").toString() == "Fresnel");
            QVERIFY(xmlReader.attributes().value("DegreesMin").toString() == "8");
            QVERIFY(xmlReader.attributes().value("DegreesMax").toString() == "38");
        }
        else if (xmlReader.name().toString() == "Focus")
        {
            focus = true;
            QVERIFY(xmlReader.attributes().value("Type").toString() == "Head");
            QVERIFY(xmlReader.attributes().value("PanMax").toString() == "520");
            QVERIFY(xmlReader.attributes().value("TiltMax").toString() == "270");
        }
        else if (xmlReader.name().toString() == "Layout")
        {
            layout = true;
            QVERIFY(xmlReader.attributes().value("Width").toString() == "6");
            QVERIFY(xmlReader.attributes().value("Height").toString() == "3");
        }
        else if (xmlReader.name().toString() == "Technical")
        {
            technical = true;
            QVERIFY(xmlReader.attributes().value("PowerConsumption").toString() == "250");
            QVERIFY(xmlReader.attributes().value("DmxConnector").toString() == "5-pin");
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString())
                  .toLatin1());
        }
        xmlReader.skipCurrentElement();
    }

    QVERIFY(bulb == true);
    QVERIFY(dim == true);
    QVERIFY(lens == true);
    QVERIFY(focus == true);
    QVERIFY(layout == true);
    QVERIFY(technical == true);
}

QTEST_MAIN(QLCPhysical_Test)
