/*
  Q Light Controller Plus - Unit tests
  rgbalgorithm_test.cpp

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

#define private public
#include "rgbalgorithm_test.h"
#include "rgbscriptscache.h"
#include "rgbalgorithm.h"
#ifdef QT_QML_LIB
  #include "rgbscriptv4.h"
#else
  #include "rgbscript.h"
#endif
#undef private

#include "doc.h"

#include "../common/resource_paths.h"

void RGBAlgorithm_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_SCRIPTDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*.js"));
    QVERIFY(dir.entryList().size() > 0);
    QVERIFY(m_doc->rgbScriptsCache()->load(dir));
}

void RGBAlgorithm_Test::cleanupTestCase()
{
    delete m_doc;
}

void RGBAlgorithm_Test::algorithms()
{
    QStringList list = RGBAlgorithm::algorithms(m_doc);
    QVERIFY(list.contains("Text"));
    QVERIFY(list.contains("Image"));
    QVERIFY(list.contains("Stripes"));
    QVERIFY(list.contains("Opposite"));
    QVERIFY(list.contains("Random Single"));
}

void RGBAlgorithm_Test::algorithm()
{
    RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, "Foo");
    QVERIFY(algo != NULL);
    QCOMPARE(algo->apiVersion(), 0); // Invalid
    delete algo;

    algo = RGBAlgorithm::algorithm(m_doc, QString());
    QVERIFY(algo != NULL);
    QCOMPARE(algo->apiVersion(), 0); // Invalid
    delete algo;

    algo = RGBAlgorithm::algorithm(m_doc, "Text");
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Text);
    QCOMPARE(algo->name(), QString("Text"));
    delete algo;

    algo = RGBAlgorithm::algorithm(m_doc, "Stripes");
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Script);
    QCOMPARE(algo->name(), QString("Stripes"));
    delete algo;

    algo = RGBAlgorithm::algorithm(m_doc, "Balls");
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Script);
    QCOMPARE(algo->name(), QString("Balls"));
    QCOMPARE(algo->apiVersion(), 3);
    QCOMPARE(algo->acceptColors(), 5);
    QVector<QColor> colors;
    colors << Qt::red;
    colors << Qt::green;
    colors << Qt::blue;
    algo->setColors(colors);

    QCOMPARE(algo->getColor(0), Qt::red);
    QCOMPARE(algo->getColor(1), Qt::green);
    QCOMPARE(algo->getColor(2), Qt::blue);
    delete algo;
}

void RGBAlgorithm_Test::loader()
{
    // Script algo
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Algorithm");
    xmlWriter.writeAttribute("Type", "Script");
    xmlWriter.writeCharacters("Stripes");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    RGBAlgorithm* algo = RGBAlgorithm::loader(m_doc, xmlReader);
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Script);
    QCOMPARE(algo->name(), QString("Stripes"));
    delete algo;

    buffer.close();

    // Text algo
    QBuffer buffer2;
    buffer2.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWriter.setDevice(&buffer2);

    xmlWriter.writeStartElement("Algorithm");
    xmlWriter.writeAttribute("Type", "Text");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer2.close();

    buffer2.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer2);
    xmlReader.readNextStartElement();

    algo = RGBAlgorithm::loader(m_doc, xmlReader);
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Text);
    QCOMPARE(algo->name(), QString("Text"));
    delete algo;

    buffer2.close();

    // Invalid type
    QBuffer buffer3;
    buffer3.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWriter.setDevice(&buffer3);

    xmlWriter.writeStartElement("Type", "Foo");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer3.close();

    buffer3.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer3);
    xmlReader.readNextStartElement();

    algo = RGBAlgorithm::loader(m_doc, xmlReader);
    QVERIFY(algo == NULL);

    buffer3.close();

    // Invalid tag
    QBuffer buffer4;
    buffer4.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWriter.setDevice(&buffer4);

    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeAttribute("Type", "Text");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer4.close();

    buffer4.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&buffer4);
    xmlReader.readNextStartElement();

    algo = RGBAlgorithm::loader(m_doc, xmlReader);
    QVERIFY(algo == NULL);
}

QTEST_MAIN(RGBAlgorithm_Test)
