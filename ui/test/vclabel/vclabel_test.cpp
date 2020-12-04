/*
  Q Light Controller Plus - Test Unit
  vclabel_test.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFrame>
#include <QtTest>
#include <QMenu>
#include <QSet>

#define protected public
#define private public

#include "virtualconsole.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "vclabel.h"
#include "qlcfixturedefcache.h"
#include "vclabel_test.h"
#include "mastertimer.h"
#include "doc.h"

#undef private
#undef protected

void VCLabel_Test::initTestCase()
{
    m_doc = NULL;
}

void VCLabel_Test::init()
{
    m_doc = new Doc(this);
    new VirtualConsole(NULL, m_doc);
}

void VCLabel_Test::cleanup()
{
    delete VirtualConsole::instance();
    delete m_doc;
}

void VCLabel_Test::initial()
{
    QWidget w;

    VCLabel label(&w, m_doc);
    QCOMPARE(label.objectName(), QString("VCLabel"));
    QCOMPARE(label.frameStyle(), 0);
    QCOMPARE(label.caption(), tr("Label"));
    QCOMPARE(label.size(), QSize(100, 30));
}

void VCLabel_Test::copy()
{
    QWidget w;

    VCFrame parent(&w, m_doc);
    VCLabel label(&parent, m_doc);
    label.setCaption("Foobar");
    VCLabel* label2 = qobject_cast<VCLabel*> (label.createCopy(&parent));
    QVERIFY(label2 != NULL && label2 != &label);
    QCOMPARE(label2->objectName(), QString("VCLabel"));
    QCOMPARE(label2->parentWidget(), &parent);
    QCOMPARE(label2->caption(), QString("Foobar"));

    QVERIFY(label.copyFrom(NULL) == false);
}

void VCLabel_Test::loadXML()
{
    QWidget w;

    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Label");

    xmlWriter.writeStartElement("WindowState");
    xmlWriter.writeAttribute("Width", "42");
    xmlWriter.writeAttribute("Height", "69");
    xmlWriter.writeAttribute("X", "3");
    xmlWriter.writeAttribute("Y", "4");
    xmlWriter.writeAttribute("Visible", "True");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Appearance");
    QFont f(w.font());
    f.setPointSize(f.pointSize() + 3);
    xmlWriter.writeTextElement("Font", f.toString());
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Foobar");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCLabel label(&w, m_doc);
    QVERIFY(label.loadXML(xmlReader) == true);
    QCOMPARE(label.geometry().width(), 42);
    QCOMPARE(label.geometry().height(), 69);
    QCOMPARE(label.geometry().x(), 3);
    QCOMPARE(label.geometry().y(), 4);
    QCOMPARE(label.font().toString(), f.toString());

    buffer.close();
    QByteArray bData = buffer.data();
    bData.replace("<Label", "<Lable");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(label.loadXML(xmlReader) == false);
}

void VCLabel_Test::saveXML()
{
    QWidget w;

    VCLabel label(&w, m_doc);
    label.setCaption("Simo Kuassimo");

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(label.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    QVERIFY(xmlReader.readNextStartElement() == true);
    QCOMPARE(xmlReader.name().toString(), QString("Label"));
    QCOMPARE(xmlReader.attributes().value("Caption").toString(), QString("Simo Kuassimo"));

    int appearance = 0, windowstate = 0;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name() == QString("Appearance"))
        {
            appearance++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name() == QString("WindowState"))
        {
            windowstate++;
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL(QString("Unexpected XML tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
        }
    }

    QCOMPARE(appearance, 1);
    QCOMPARE(windowstate, 1);
}

void VCLabel_Test::paintEvent()
{
    QWidget w;
    VCLabel label(&w, m_doc);

    w.show();
    label.show();

    QTest::qWait(1);

    label.setCaption("Foobar");
    label.update();
    QTest::qWait(1);

    m_doc->setMode(Doc::Operate);
    label.update();
    QTest::qWait(1);
}

QTEST_MAIN(VCLabel_Test)
