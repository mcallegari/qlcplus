/*
  Q Light Controller Plus - Unit test
  vcwidgetproperties_test.cpp

  Copyright (c) Heikki Junnila
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

#include <QWidget>
#include <QtTest>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "vcwidgetproperties_test.h"
#define protected public
#include "vcwidgetproperties.h"
#undef private

void VCWidgetProperties_Test::stateAndVisibility()
{
    VCWidgetProperties p;
    QVERIFY(p.state() == Qt::WindowNoState);
    QVERIFY(p.visible() == false);

    QWidget w(NULL);
    p.store(&w);
    QVERIFY(p.state() == Qt::WindowNoState);
    QVERIFY(p.visible() == false);

    w.showMinimized();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowMinimized);
    QVERIFY(p.visible() == true);

    w.showMaximized();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowMaximized);
    QVERIFY(p.visible() == true);

    w.showFullScreen();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowFullScreen);
    QVERIFY(p.visible() == true);

    w.hide();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowFullScreen);
    QVERIFY(p.visible() == false);
}

void VCWidgetProperties_Test::xy()
{
    VCWidgetProperties p;

    QWidget w(NULL);
    p.store(&w);

    QVERIFY(p.x() == 0);
    QVERIFY(p.y() == 0);

    w.move(50, 10);
    p.store(&w);

    QVERIFY(p.x() == 50);
    QVERIFY(p.y() == 10);
}

void VCWidgetProperties_Test::wh()
{
    VCWidgetProperties p;

    QWidget w(NULL);
    p.store(&w);

    QVERIFY(p.x() == 0);
    QVERIFY(p.y() == 0);

    w.resize(20, 30);
    p.store(&w);

    QVERIFY(p.width() == 20);
    QVERIFY(p.height() == 30);
}

void VCWidgetProperties_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("WidgetProperties");

    xmlWriter.writeTextElement("X", "50");
    xmlWriter.writeTextElement("Y", "70");
    xmlWriter.writeTextElement("Width", "40");
    xmlWriter.writeTextElement("Height", "60");
    xmlWriter.writeTextElement("Visible", "1");
    xmlWriter.writeTextElement("State", QString("%1").arg(Qt::WindowMinimized));
    xmlWriter.writeTextElement("Foobar", "1");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCWidgetProperties p;
    p.loadXML(xmlReader);
    QVERIFY(p.x() == 50);
    QVERIFY(p.y() == 70);
    QVERIFY(p.width() == 40);
    QVERIFY(p.height() == 60);
    QVERIFY(p.state() == Qt::WindowMinimized);
    QVERIFY(p.visible() == true);
}

void VCWidgetProperties_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("WidgetPropertiez");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCWidgetProperties p;
    QVERIFY(p.loadXML(xmlReader) == false);
    QVERIFY(p.x() == 100);
    QVERIFY(p.y() == 100);
    QVERIFY(p.width() == 0);
    QVERIFY(p.height() == 0);
    QVERIFY(p.state() == Qt::WindowNoState);
    QVERIFY(p.visible() == false);
}

void VCWidgetProperties_Test::save()
{
    VCWidgetProperties p;
    p.m_state = Qt::WindowMinimized;
    p.m_visible = true;
    p.m_x = 10;
    p.m_y = 20;
    p.m_width = 30;
    p.m_height = 40;

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(p.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("WidgetProperties"));

    bool s = false, v = false, x = false, y = false, w = false, h = false;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "State")
        {
            s = true;
            QCOMPARE(xmlReader.readElementText(), QString::number(p.m_state));
        }
        else if (xmlReader.name().toString() == "Visible")
        {
            v = true;
            QCOMPARE(xmlReader.readElementText(), QString::number(p.m_visible));
        }
        else if (xmlReader.name().toString() == "X")
        {
            x = true;
            QCOMPARE(xmlReader.readElementText(), QString::number(p.m_x));
        }
        else if (xmlReader.name().toString() == "Y")
        {
            y = true;
            QCOMPARE(xmlReader.readElementText(), QString::number(p.m_y));
        }
        else if (xmlReader.name().toString() == "Width")
        {
            w = true;
            QCOMPARE(xmlReader.readElementText(), QString::number(p.m_width));
        }
        else if (xmlReader.name().toString() == "Height")
        {
            h = true;
            QCOMPARE(xmlReader.readElementText(), QString::number(p.m_height));
        }
        else
        {
            QFAIL(QString("Unexpected widget property tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
        }
    }

    QVERIFY(s && v && x && y && w && h);
}

void VCWidgetProperties_Test::copy()
{
    VCWidgetProperties p;
    p.m_state = Qt::WindowMinimized;
    p.m_visible = true;
    p.m_x = 10;
    p.m_y = 20;
    p.m_width = 30;
    p.m_height = 40;

    VCWidgetProperties p2(p);
    QCOMPARE(p2.state(), p.state());
    QCOMPARE(p2.visible(), p.visible());
    QCOMPARE(p2.x(), p.x());
    QCOMPARE(p2.y(), p.y());
    QCOMPARE(p2.width(), p.width());
    QCOMPARE(p2.height(), p.height());
}

QTEST_MAIN(VCWidgetProperties_Test)
