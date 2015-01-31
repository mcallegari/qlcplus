/*
  Q Light Controller
  vcxypadfixture_test.cpp

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

#define private public
#include "vcxypadfixture.h"
#undef private

#include "vcxypadfixture_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universe.h"
#include "qlcfile.h"
#include "doc.h"

#include "../../../engine/test/common/resource_paths.h"

void VCXYPadFixture_Test::initTestCase()
{
    m_doc = new Doc(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
}

void VCXYPadFixture_Test::init()
{
}

void VCXYPadFixture_Test::cleanup()
{
    m_doc->clearContents();
}

void VCXYPadFixture_Test::initial()
{
    VCXYPadFixture fxi(m_doc);
    QVERIFY(!fxi.m_head.isValid());
    QCOMPARE(fxi.m_doc, m_doc);

    QCOMPARE(fxi.m_xMin, qreal(0));
    QCOMPARE(fxi.m_xMax, qreal(1));
    QCOMPARE(fxi.m_xReverse, false);

    QCOMPARE(fxi.m_yMin, qreal(0));
    QCOMPARE(fxi.m_yMax, qreal(1));
    QCOMPARE(fxi.m_yReverse, false);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::params()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setX(0.2, 0.3, false);
    QCOMPARE(fxi.xMin(), 0.2);
    QCOMPARE(fxi.xMax(), 0.3);
    QCOMPARE(fxi.xReverse(), false);
    QCOMPARE(fxi.xBrief(), QString("%1 - %2%").arg(100 * 0.2).arg(100 * 0.3));

    fxi.setX(0.2, 0.3, true);
    QCOMPARE(fxi.xReverse(), true);
    QCOMPARE(fxi.xBrief(), QString("%1: %2 - %3%").arg(tr("Reversed")).arg(100 * 0.3).arg(100 * 0.2));

    fxi.setY(0.1, 0.8, false);
    QCOMPARE(fxi.yMin(), 0.1);
    QCOMPARE(fxi.yMax(), 0.8);
    QCOMPARE(fxi.yReverse(), false);
    QCOMPARE(fxi.yBrief(), QString("%1 - %2%").arg(100 * 0.1).arg(100 * 0.8));

    fxi.setY(0.1, 0.8, true);
    QCOMPARE(fxi.yReverse(), true);
    QCOMPARE(fxi.yBrief(), QString("%1: %2 - %3%").arg(tr("Reversed")).arg(100 * 0.8).arg(100 * 0.1));
}

void VCXYPadFixture_Test::fromVariantBelowZero()
{
    QStringList list;
    list << QString("12");
    list << QString("7");
    list << QString("-0.1");
    list << QString("-0.2");
    list << QString("0");
    list << QString("-0.3");
    list << QString("-0.4");
    list << QString("0");

    VCXYPadFixture fxi(m_doc, list);
    QCOMPARE(fxi.m_head.fxi, quint32(12));
    QCOMPARE(fxi.m_head.head, 7);

    QCOMPARE(fxi.m_xMin, qreal(0));
    QCOMPARE(fxi.m_xMax, qreal(0));
    QCOMPARE(fxi.m_xReverse, false);

    QCOMPARE(fxi.m_yMin, qreal(0));
    QCOMPARE(fxi.m_yMax, qreal(0));
    QCOMPARE(fxi.m_yReverse, false);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::fromVariantAboveOne()
{
    QStringList list;
    list << QString("12");
    list << QString("0");
    list << QString("1.1");
    list << QString("1.2");
    list << QString("2");
    list << QString("1.3");
    list << QString("1.4");
    list << QString("3");

    VCXYPadFixture fxi(m_doc, list);
    QCOMPARE(fxi.m_head.fxi, quint32(12));
    QCOMPARE(fxi.m_head.head, 0);

    QCOMPARE(fxi.m_xMin, qreal(1));
    QCOMPARE(fxi.m_xMax, qreal(1));
    QCOMPARE(fxi.m_xReverse, true);

    QCOMPARE(fxi.m_yMin, qreal(1));
    QCOMPARE(fxi.m_yMax, qreal(1));
    QCOMPARE(fxi.m_yReverse, true);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::fromVariantWithinRange()
{
    QStringList list;
    list << QString("12");
    list << QString("0");
    list << QString("0.3");
    list << QString("0.4");
    list << QString("1");
    list << QString("0.5");
    list << QString("0.6");
    list << QString("1");

    VCXYPadFixture fxi(m_doc, list);
    QCOMPARE(fxi.m_head.fxi, quint32(12));
    QCOMPARE(fxi.m_head.head, 0);

    QCOMPARE(fxi.m_xMin, qreal(0.3));
    QCOMPARE(fxi.m_xMax, qreal(0.4));
    QCOMPARE(fxi.m_xReverse, true);

    QCOMPARE(fxi.m_yMin, qreal(0.5));
    QCOMPARE(fxi.m_yMax, qreal(0.6));
    QCOMPARE(fxi.m_yReverse, true);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::fromVariantWrongSize()
{
    QStringList list;
    list << QString("12");
    list << QString("0");
    list << QString("0.3");
    list << QString("0.4");
    list << QString("1");
    list << QString("0.5");
    list << QString("0.6");
    list << QString("1");
    list.takeLast();

    VCXYPadFixture fxi(m_doc, list);
    QVERIFY(!fxi.m_head.isValid());

    QCOMPARE(fxi.m_xMin, qreal(0));
    QCOMPARE(fxi.m_xMax, qreal(1));
    QCOMPARE(fxi.m_xReverse, false);

    QCOMPARE(fxi.m_yMin, qreal(0));
    QCOMPARE(fxi.m_yMax, qreal(1));
    QCOMPARE(fxi.m_yReverse, false);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::fromVariantWrongVariant()
{
    VCXYPadFixture fxi(m_doc, QVariant(42));
    QVERIFY(!fxi.m_head.isValid());

    QCOMPARE(fxi.m_xMin, qreal(0));
    QCOMPARE(fxi.m_xMax, qreal(1));
    QCOMPARE(fxi.m_xReverse, false);

    QCOMPARE(fxi.m_yMin, qreal(0));
    QCOMPARE(fxi.m_yMax, qreal(1));
    QCOMPARE(fxi.m_yReverse, false);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::toVariant()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(3000, 178));
    fxi.setX(0.1, 0.2, true);
    fxi.setY(0.3, 0.4, false);

    QVariant var(fxi);
    QVERIFY(var.canConvert<QStringList>() == true);
    QStringList list = var.toStringList();
    QCOMPARE(list.size(), 8);
    QCOMPARE(list.takeFirst(), QString("3000"));
    QCOMPARE(list.takeFirst(), QString("178"));
    QCOMPARE(list.takeFirst(), QString("0.1"));
    QCOMPARE(list.takeFirst(), QString("0.2"));
    QCOMPARE(list.takeFirst(), QString("1"));
    QCOMPARE(list.takeFirst(), QString("0.3"));
    QCOMPARE(list.takeFirst(), QString("0.4"));
    QCOMPARE(list.takeFirst(), QString("0"));
}

void VCXYPadFixture_Test::copy()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(3000, 178));
    fxi.setX(0.1, 0.2, true);
    fxi.setY(0.3, 0.4, false);

    VCXYPadFixture fxi2 = fxi;
    QCOMPARE(fxi2.m_doc, fxi.m_doc);
    QCOMPARE(fxi2.head(), fxi.head());

    QCOMPARE(fxi2.xMin(), fxi.xMin());
    QCOMPARE(fxi2.xMax(), fxi.xMax());
    QCOMPARE(fxi2.xReverse(), fxi.xReverse());

    QCOMPARE(fxi2.yMin(), fxi.yMin());
    QCOMPARE(fxi2.yMax(), fxi.yMax());
    QCOMPARE(fxi2.yReverse(), fxi.yReverse());
}

void VCXYPadFixture_Test::compare()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(42, 13));

    VCXYPadFixture fxi2(m_doc);
    fxi2.setHead(GroupHead(24, 31));

    QVERIFY((fxi == fxi2) == false);

    fxi2.setHead(GroupHead(42, 13));
    QVERIFY(fxi == fxi2);
}

void VCXYPadFixture_Test::name()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setName("Test fixture");
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    QCOMPARE(xy.name(), QString());

    xy.setHead(GroupHead(fxi->id(), 0));
    QCOMPARE(xy.name(), QString("Test fixture"));

    xy.setHead(GroupHead(fxi->id() + 1, 0));
    QCOMPARE(xy.name(), QString());

    m_doc->deleteFixture(fxi->id());
}

void VCXYPadFixture_Test::loadXMLWrongRoot()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Fixteru");
    root.setAttribute("ID", "69");
    root.setAttribute("Head", "0");
    doc.appendChild(root);

    VCXYPadFixture fxi(m_doc);
    QVERIFY(fxi.loadXML(root) == false);
}

void VCXYPadFixture_Test::loadXMLHappy()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Fixture");
    root.setAttribute("ID", "69");
    root.setAttribute("Head", "0");
    doc.appendChild(root);

    QDomElement x = doc.createElement("Axis");
    x.setAttribute("ID", "X");
    x.setAttribute("LowLimit", "0.1");
    x.setAttribute("HighLimit", "0.5");
    x.setAttribute("Reverse", "True");
    root.appendChild(x);

    QDomElement y = doc.createElement("Axis");
    y.setAttribute("ID", "Y");
    y.setAttribute("LowLimit", "0.2");
    y.setAttribute("HighLimit", "0.6");
    y.setAttribute("Reverse", "True");
    root.appendChild(y);

    QDomElement z = doc.createElement("Axis"); // Foo axis
    z.setAttribute("ID", "Z");
    z.setAttribute("LowLimit", "0.2");
    z.setAttribute("HighLimit", "0.6");
    z.setAttribute("Reverse", "True");
    root.appendChild(z);

    QDomElement foo = doc.createElement("Foo");
    foo.setAttribute("ID", "Z");
    foo.setAttribute("LowLimit", "0.2");
    foo.setAttribute("HighLimit", "0.6");
    foo.setAttribute("Reverse", "True");
    root.appendChild(foo);

    VCXYPadFixture fxi(m_doc);
    QVERIFY(fxi.loadXML(root) == true);
    QCOMPARE(fxi.head().fxi, quint32(69));
    QCOMPARE(fxi.head().head, 0);

    QCOMPARE(fxi.xMin(), qreal(0.1));
    QCOMPARE(fxi.xMax(), qreal(0.5));
    QCOMPARE(fxi.xReverse(), true);

    QCOMPARE(fxi.yMin(), qreal(0.2));
    QCOMPARE(fxi.yMax(), qreal(0.6));
    QCOMPARE(fxi.yReverse(), true);
}

void VCXYPadFixture_Test::loadXMLSad()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Fixture");
    root.setAttribute("ID", "69");
    root.setAttribute("Head", "0");
    doc.appendChild(root);

    QDomElement x = doc.createElement("Axis");
    x.setAttribute("ID", "X");
    x.setAttribute("LowLimit", "0.1");
    x.setAttribute("HighLimit", "0.5");
    x.setAttribute("Reverse", "False");
    root.appendChild(x);

    QDomElement y = doc.createElement("Axis");
    y.setAttribute("ID", "Y");
    y.setAttribute("LowLimit", "0.2");
    y.setAttribute("HighLimit", "0.6");
    y.setAttribute("Reverse", "False");
    root.appendChild(y);

    QDomElement z = doc.createElement("Axis"); // Foo axis
    z.setAttribute("ID", "Z");
    z.setAttribute("LowLimit", "0.2");
    z.setAttribute("HighLimit", "0.6");
    z.setAttribute("Reverse", "False");
    root.appendChild(z);

    QDomElement foo = doc.createElement("Foo");
    foo.setAttribute("ID", "Z");
    foo.setAttribute("LowLimit", "0.2");
    foo.setAttribute("HighLimit", "0.6");
    foo.setAttribute("Reverse", "False");
    root.appendChild(foo);

    VCXYPadFixture fxi(m_doc);
    QVERIFY(fxi.loadXML(root) == true);
    QCOMPARE(fxi.head().fxi, quint32(69));
    QCOMPARE(fxi.head().head, 0);

    QCOMPARE(fxi.xMin(), qreal(0.1));
    QCOMPARE(fxi.xMax(), qreal(0.5));
    QCOMPARE(fxi.xReverse(), false);

    QCOMPARE(fxi.yMin(), qreal(0.2));
    QCOMPARE(fxi.yMax(), qreal(0.6));
    QCOMPARE(fxi.yReverse(), false);
}

void VCXYPadFixture_Test::saveXMLHappy()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(54,32));
    fxi.setX(0.1, 0.2, true);
    fxi.setY(0.3, 0.4, true);

    QDomDocument doc;
    QDomElement root = doc.createElement("TestRoot");
    doc.appendChild(root);

    QVERIFY(fxi.saveXML(&doc, &root) == true);
    QDomNode node = root.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("Fixture"));
    QCOMPARE(node.toElement().attribute("ID"), QString("54"));
    QCOMPARE(node.toElement().attribute("Head"), QString("32"));

    bool x = false, y = false;
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Axis")
        {
            QString id = tag.attribute("ID");
            QString low = tag.attribute("LowLimit");
            QString high = tag.attribute("HighLimit");
            QString rev = tag.attribute("Reverse");
            if (id == "X")
            {
                x = true;
                QCOMPARE(low, QString("0.1"));
                QCOMPARE(high, QString("0.2"));
                QCOMPARE(rev, QString("True"));
            }
            else if (id == "Y")
            {
                y = true;
                QCOMPARE(low, QString("0.3"));
                QCOMPARE(high, QString("0.4"));
                QCOMPARE(rev, QString("True"));
            }
            else
            {
                QFAIL(QString("Unexpected axis: %1").arg(id).toUtf8().constData());
            }
        }
        else
        {
            qDebug() << doc.toString();
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }
        node = node.nextSibling();
    }

    QVERIFY(x == true);
    QVERIFY(y == true);
}

void VCXYPadFixture_Test::saveXMLSad()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(54,32));
    fxi.setX(0.1, 0.2, false);
    fxi.setY(0.3, 0.4, false);

    QDomDocument doc;
    QDomElement root = doc.createElement("TestRoot");
    doc.appendChild(root);

    QVERIFY(fxi.saveXML(&doc, &root) == true);
    QDomNode node = root.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("Fixture"));
    QCOMPARE(node.toElement().attribute("ID"), QString("54"));
    QCOMPARE(node.toElement().attribute("Head"), QString("32"));

    bool x = false, y = false;
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Axis")
        {
            QString id = tag.attribute("ID");
            QString low = tag.attribute("LowLimit");
            QString high = tag.attribute("HighLimit");
            QString rev = tag.attribute("Reverse");
            if (id == "X")
            {
                x = true;
                QCOMPARE(low, QString("0.1"));
                QCOMPARE(high, QString("0.2"));
                QCOMPARE(rev, QString("False"));
            }
            else if (id == "Y")
            {
                y = true;
                QCOMPARE(low, QString("0.3"));
                QCOMPARE(high, QString("0.4"));
                QCOMPARE(rev, QString("False"));
            }
            else
            {
                QFAIL(QString("Unexpected axis: %1").arg(id).toUtf8().constData());
            }
        }
        else
        {
            qDebug() << doc.toString();
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }
        node = node.nextSibling();
    }

    QVERIFY(x == true);
    QVERIFY(y == true);
}

void VCXYPadFixture_Test::armNoFixture()
{
    VCXYPadFixture xy(m_doc);
    xy.arm();
    QCOMPARE(xy.m_xMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_xLSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yLSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::armDimmer()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(6);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.arm();
    QCOMPARE(xy.m_xMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_xLSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yLSB, QLCChannel::invalid());

    m_doc->deleteFixture(fxi->id());
}

void VCXYPadFixture_Test::arm8bit()
{
    Fixture* fxi = new Fixture(m_doc);
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    fxi->setAddress(50);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.arm();
    QCOMPARE(xy.m_xMSB, quint32(50));
    QCOMPARE(xy.m_xLSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yMSB, quint32(51));
    QCOMPARE(xy.m_yLSB, QLCChannel::invalid());

    m_doc->deleteFixture(fxi->id());
}

void VCXYPadFixture_Test::arm16bit()
{
    Fixture* fxi = new Fixture(m_doc);
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Varytec", "Easy Move LED XS Spot");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.arm();
    QCOMPARE(xy.m_xMSB, quint32(0));
    QCOMPARE(xy.m_xLSB, quint32(1));
    QCOMPARE(xy.m_yMSB, quint32(2));
    QCOMPARE(xy.m_yLSB, quint32(3));

    m_doc->deleteFixture(fxi->id());
}

void VCXYPadFixture_Test::disarm()
{
    VCXYPadFixture xy(m_doc);
    xy.m_xMSB = 0;
    xy.m_xLSB = 1;
    xy.m_yMSB = 2;
    xy.m_yLSB = 3;
    xy.disarm();
    QCOMPARE(xy.m_xMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_xLSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yLSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::writeDimmer()
{
    VCXYPadFixture xy(m_doc);
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));

    xy.writeDMX(1, 1, ua);
    QCOMPARE(ua[0]->preGMValues()[0], char(0));

    xy.m_xMSB = 0;
    xy.m_yMSB = QLCChannel::invalid();
    xy.writeDMX(1, 1, ua);
    QCOMPARE(ua[0]->preGMValues()[0], char(0));

    xy.m_xMSB = QLCChannel::invalid();
    xy.m_yMSB = 0;
    xy.writeDMX(1, 1, ua);
    QCOMPARE(ua[0]->preGMValues()[0], char(0));
}

void VCXYPadFixture_Test::write8bitNoReverse()
{
    Fixture* fxi = new Fixture(m_doc);
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(0, 1, false);
    xy.setY(0, 1, false);
    xy.arm();

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));

    for (qreal i = 0; i <= 1.01; i += (qreal(1) / qreal(USHRT_MAX)))
    {
        xy.writeDMX(i, 1.0 - i, ua);

        ushort x = floor((qreal(USHRT_MAX) * i) + 0.5);
        ushort y = floor((qreal(USHRT_MAX) * (1.0 - i)) + 0.5);

        QCOMPARE(ua[0]->preGMValues()[0], char(x >> 8));
        QCOMPARE(ua[0]->preGMValues()[1], char(y >> 8));
        QCOMPARE(ua[0]->preGMValues()[2], char(0));
        QCOMPARE(ua[0]->preGMValues()[3], char(0));
        QCOMPARE(ua[0]->preGMValues()[4], char(0));
        QCOMPARE(ua[0]->preGMValues()[5], char(0));
    }
}

void VCXYPadFixture_Test::write8bitReverse()
{
    Fixture* fxi = new Fixture(m_doc);
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(0, 1, true);
    xy.setY(0, 1, true);
    xy.arm();

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));

    for (qreal i = 0; i <= 1.01; i += (qreal(1) / qreal(USHRT_MAX)))
    {
        xy.writeDMX(i, 1.0 - i, ua);

        ushort x = floor((qreal(USHRT_MAX) * (1.0 - i)) + 0.5);
        ushort y = floor((qreal(USHRT_MAX) * i) + 0.5);

        QCOMPARE(ua[0]->preGMValues()[0], char(x >> 8));
        QCOMPARE(ua[0]->preGMValues()[1], char(y >> 8));
        QCOMPARE(ua[0]->preGMValues()[2], char(0));
        QCOMPARE(ua[0]->preGMValues()[3], char(0));
        QCOMPARE(ua[0]->preGMValues()[4], char(0));
        QCOMPARE(ua[0]->preGMValues()[5], char(0));
    }
}

void VCXYPadFixture_Test::write16bitNoReverse()
{
    Fixture* fxi = new Fixture(m_doc);
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Varytec", "Easy Move LED XS Spot");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(0, 1, false);
    xy.setY(0, 1, false);
    xy.arm();

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));

    for (qreal i = 0; i <= 1.01; i += (qreal(1) / qreal(USHRT_MAX)))
    {
        xy.writeDMX(i, 1.0 - i, ua);

        ushort x = floor((qreal(USHRT_MAX) * i) + 0.5);
        ushort y = floor((qreal(USHRT_MAX) * (1.0 - i)) + 0.5);

        QCOMPARE(ua[0]->preGMValues()[0], char(x >> 8));
        QCOMPARE(ua[0]->preGMValues()[1], char(x & 0xFF));
        QCOMPARE(ua[0]->preGMValues()[2], char(y >> 8));
        QCOMPARE(ua[0]->preGMValues()[3], char(y & 0xFF));
    }
}

void VCXYPadFixture_Test::write16bitReverse()
{
    Fixture* fxi = new Fixture(m_doc);
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Varytec", "Easy Move LED XS Spot");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(0.1, 0.9, true);
    xy.setY(0.2, 0.8, true);
    xy.arm();

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));

    for (qreal i = 0; i <= 1.01; i += (qreal(1) / qreal(USHRT_MAX)))
    {
        xy.writeDMX(i, 1.0 - i, ua);

        qreal xmul = i;
        qreal ymul = 1.0 - i;

        xmul = ((xy.xMax() - xy.xMin()) * xmul) + xy.xMin();
        ymul = ((xy.yMax() - xy.yMin()) * ymul) + xy.yMin();

        xmul = xy.xMax() - xmul;
        ymul = xy.yMax() - ymul;

        ushort x = floor((qreal(USHRT_MAX) * xmul) + 0.5);
        ushort y = floor((qreal(USHRT_MAX) * ymul) + 0.5);

        QCOMPARE(ua[0]->preGMValues()[0], char(x >> 8));
        QCOMPARE(ua[0]->preGMValues()[1], char(x & 0xFF));
        QCOMPARE(ua[0]->preGMValues()[2], char(y >> 8));
        QCOMPARE(ua[0]->preGMValues()[3], char(y & 0xFF));
    }
}

void VCXYPadFixture_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

QTEST_MAIN(VCXYPadFixture_Test)
