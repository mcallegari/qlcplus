/*
  Q Light Controller
  rgbalgorithm_test.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QtTest>

#define private public
#include "rgbalgorithm_test.h"
#include "rgbalgorithm.h"
#include "rgbscript.h"
#include "rgbscriptscache.h"
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
    printf("%s\n", algo->name().toStdString().c_str());
    printf("%s\n", algo->name().toStdString().c_str());
    printf("%s\n", algo->name().toStdString().c_str());
    printf("%s\n", algo->name().toStdString().c_str());
    printf("%s\n", algo->name().toStdString().c_str());
    printf("%s\n", algo->name().toStdString().c_str());
    delete algo;
}

void RGBAlgorithm_Test::loader()
{
    QDomDocument doc;

    // Script algo
    QDomElement scr = doc.createElement("Algorithm");
    scr.setAttribute("Type", "Script");
    QDomText scrText = doc.createTextNode("Stripes");
    scr.appendChild(scrText);
    doc.appendChild(scr);
    RGBAlgorithm* algo = RGBAlgorithm::loader(m_doc, scr);
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Script);
    QCOMPARE(algo->name(), QString("Stripes"));
    delete algo;

    // Text algo
    QDomElement txt = doc.createElement("Algorithm");
    txt.setAttribute("Type", "Text");
    doc.appendChild(txt);
    algo = RGBAlgorithm::loader(m_doc, txt);
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Text);
    QCOMPARE(algo->name(), QString("Text"));
    delete algo;

    // Invalid type
    txt.setAttribute("Type", "Foo");
    algo = RGBAlgorithm::loader(m_doc, txt);
    QVERIFY(algo == NULL);

    // Invalid tag
    QDomElement foo = doc.createElement("Foo");
    foo.setAttribute("Type", "Text");
    doc.appendChild(foo);
    algo = RGBAlgorithm::loader(m_doc, foo);
    QVERIFY(algo == NULL);
}

QTEST_MAIN(RGBAlgorithm_Test)
