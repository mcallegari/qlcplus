/*
  Q Light Controller - Unit tests
  qlcphysical_test.cpp

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
    QDomDocument doc;

    QDomElement root = doc.createElement("Physical");
    doc.appendChild(root);

    /* Bulb */
    QDomElement bulb = doc.createElement("Bulb");
    bulb.setAttribute("Type", "LED");
    bulb.setAttribute("Lumens", 18000);
    bulb.setAttribute("ColourTemperature", 6500);
    root.appendChild(bulb);

    /* Dimensions */
    QDomElement dim = doc.createElement("Dimensions");
    dim.setAttribute("Weight", QString::number(39.4));
    dim.setAttribute("Width", 530);
    dim.setAttribute("Height", 320);
    dim.setAttribute("Depth", 260);
    root.appendChild(dim);

    /* Lens */
    QDomElement lens = doc.createElement("Lens");
    lens.setAttribute("Name", "Fresnel");
    lens.setAttribute("DegreesMin", 8);
    lens.setAttribute("DegreesMax", 38);
    root.appendChild(lens);

    /* Focus */
    QDomElement focus = doc.createElement("Focus");
    focus.setAttribute("Type", "Head");
    focus.setAttribute("PanMax", 520);
    focus.setAttribute("TiltMax", 270);
    root.appendChild(focus);

    /* Technical */
    QDomElement technical = doc.createElement("Technical");
    technical.setAttribute("PowerConsumption", 250);
    technical.setAttribute("DmxConnector", "5-pin");
    root.appendChild(technical);

    /* Unrecognized tag */
    QDomElement homer = doc.createElement("HomerSimpson");
    homer.setAttribute("BeerConsumption", 25000);
    homer.setAttribute("PreferredBrand", "Duff");
    root.appendChild(homer);

    QVERIFY(p.loadXML(root) == true);
    QVERIFY(p.bulbType() == "LED");
    QVERIFY(p.bulbLumens() == 18000);
    QVERIFY(p.bulbColourTemperature() == 6500);
    QCOMPARE(p.weight(), 39.4);
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
    QDomDocument doc;

    QDomElement root = doc.createElement("Foosical");
    doc.appendChild(root);

    /* Bulb */
    QDomElement bulb = doc.createElement("Bulb");
    bulb.setAttribute("Type", "LED");
    bulb.setAttribute("Lumens", 18000);
    bulb.setAttribute("ColourTemperature", 6500);
    root.appendChild(bulb);

    /* Dimensions */
    QDomElement dim = doc.createElement("Dimensions");
    dim.setAttribute("Weight", 39.4);
    dim.setAttribute("Width", 530);
    dim.setAttribute("Height", 320);
    dim.setAttribute("Depth", 260);
    root.appendChild(dim);

    /* Lens */
    QDomElement lens = doc.createElement("Lens");
    lens.setAttribute("Name", "Fresnel");
    lens.setAttribute("DegreesMin", 8);
    lens.setAttribute("DegreesMax", 38);
    root.appendChild(lens);

    /* Focus */
    QDomElement focus = doc.createElement("Focus");
    focus.setAttribute("Type", "Head");
    focus.setAttribute("PanMax", 520);
    focus.setAttribute("TiltMax", 270);
    root.appendChild(focus);

    /* Technical */
    QDomElement technical=  doc.createElement("Technical");
    technical.setAttribute("PowerConsumption", 250);
    technical.setAttribute("DmxConnector", "5-pin");
    root.appendChild(technical);

    QVERIFY(p.loadXML(root) == false);
}

void QLCPhysical_Test::save()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Test Root");
    bool bulb = false, dim = false, lens = false, focus = false, technical = false;

    QVERIFY(p.saveXML(&doc, &root) == true);
    QVERIFY(root.firstChild().toElement().tagName() == "Physical");

    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement e = node.toElement();
        if (e.tagName() == "Bulb")
        {
            bulb = true;
            QVERIFY(e.attribute("Type") == "LED");
            QVERIFY(e.attribute("Lumens") == "18000");
            QVERIFY(e.attribute("ColourTemperature") == "6500");
        }
        else if (e.tagName() == "Dimensions")
        {
            dim = true;
            QVERIFY(e.attribute("Width") == "530");
            QVERIFY(e.attribute("Depth") == "260");
            QVERIFY(e.attribute("Height") == "320");
            QCOMPARE(e.attribute("Weight").toDouble(), 39.4);
        }
        else if (e.tagName() == "Lens")
        {
            lens = true;
            QVERIFY(e.attribute("Name") == "Fresnel");
            QVERIFY(e.attribute("DegreesMin") == "8");
            QVERIFY(e.attribute("DegreesMax") == "38");
        }
        else if (e.tagName() == "Focus")
        {
            focus = true;
            QVERIFY(e.attribute("Type") == "Head");
            QVERIFY(e.attribute("PanMax") == "520");
            QVERIFY(e.attribute("TiltMax") == "270");
        }
        else if (e.tagName() == "Technical")
        {
            technical = true;
            QVERIFY(e.attribute("PowerConsumption") == "250");
            QVERIFY(e.attribute("DmxConnector") == "5-pin");
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(e.tagName())
                  .toLatin1());
        }

        node = node.nextSibling();
    }

    QVERIFY(bulb == true);
    QVERIFY(dim == true);
    QVERIFY(lens == true);
    QVERIFY(focus == true);
    QVERIFY(technical == true);
}

QTEST_MAIN(QLCPhysical_Test)
