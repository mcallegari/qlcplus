/*
  Q Light Controller
  rgbalgorithm_test.cpp

  Copyright (C) Heikki Junnila

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

#include <QDomDocument>
#include <QDomElement>
#include <QtTest>

#define private public
#include "rgbalgorithm_test.h"
#include "rgbalgorithm.h"
#include "rgbscript.h"
#undef private

#define INTERNAL_SCRIPTDIR "../../../rgbscripts"

void RGBAlgorithm_Test::initTestCase()
{
    QDir dir(INTERNAL_SCRIPTDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*.js"));
    QVERIFY(dir.entryList().size() > 0);
    RGBScript::setCustomScriptDirectory(INTERNAL_SCRIPTDIR);
}

void RGBAlgorithm_Test::algorithms()
{
    QStringList list = RGBAlgorithm::algorithms();
    QVERIFY(list.contains("Text"));
    QVERIFY(list.contains("Full Columns"));
    QVERIFY(list.contains("Full Rows"));
    QVERIFY(list.contains("Opposite Columns"));
    QVERIFY(list.contains("Opposite Rows"));
    QVERIFY(list.contains("Single Random"));
}

void RGBAlgorithm_Test::algorithm()
{
    RGBAlgorithm* algo = RGBAlgorithm::algorithm("Foo");
    QVERIFY(algo != NULL);
    QCOMPARE(algo->apiVersion(), 0); // Invalid
    delete algo;

    algo = RGBAlgorithm::algorithm(QString());
    QVERIFY(algo != NULL);
    QCOMPARE(algo->apiVersion(), 0); // Invalid
    delete algo;

    algo = RGBAlgorithm::algorithm("Text");
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Text);
    QCOMPARE(algo->name(), QString("Text"));
    delete algo;

    algo = RGBAlgorithm::algorithm("Full Rows");
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Script);
    QCOMPARE(algo->name(), QString("Full Rows"));
    delete algo;
}

void RGBAlgorithm_Test::loader()
{
    QDomDocument doc;

    // Script algo
    QDomElement scr = doc.createElement("Algorithm");
    scr.setAttribute("Type", "Script");
    QDomText scrText = doc.createTextNode("Full Rows");
    scr.appendChild(scrText);
    doc.appendChild(scr);
    RGBAlgorithm* algo = RGBAlgorithm::loader(scr);
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Script);
    QCOMPARE(algo->name(), QString("Full Rows"));
    delete algo;

    // Text algo
    QDomElement txt = doc.createElement("Algorithm");
    txt.setAttribute("Type", "Text");
    doc.appendChild(txt);
    algo = RGBAlgorithm::loader(txt);
    QVERIFY(algo != NULL);
    QCOMPARE(algo->type(), RGBAlgorithm::Text);
    QCOMPARE(algo->name(), QString("Text"));
    delete algo;

    // Invalid type
    txt.setAttribute("Type", "Foo");
    algo = RGBAlgorithm::loader(txt);
    QVERIFY(algo == NULL);

    // Invalid tag
    QDomElement foo = doc.createElement("Foo");
    foo.setAttribute("Type", "Text");
    doc.appendChild(foo);
    algo = RGBAlgorithm::loader(foo);
    QVERIFY(algo == NULL);
}

QTEST_MAIN(RGBAlgorithm_Test)
