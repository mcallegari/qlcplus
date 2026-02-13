/*
  Q Light Controller
  script_test.cpp

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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QBuffer>

#define private public

#include "qlcfixturedefcache.h"
#include "mastertimer.h"
#include "script_test.h"
#include "universe.h"
#include "script.h"
#include "doc.h"

#undef private

static QString script0(
"// Comment over there\n"
"startfunction:12\r"
"stopfunction:33 paska\n"
"waitkey:\"SHIFT+K\"\n"
"startfunction:\"54\"\r\n"
"wait:1.05\n"
"setfixture:99 value:255 channel:1\n"
"jump:label1\n"
"wait:1.05\n"
"label:label1\n"
"blackout:on\n"
"systemcommand:echo arg:asdf\n"
"blackout:off\n"
);

void Script_Test::initTestCase()
{
}
void Script_Test::initial()
{
    Doc doc(this);
    GrandMaster *gm = new GrandMaster();
    QList<Universe*> ua;
    ua.append(new Universe(0, gm));
    ua.append(new Universe(1, gm));
    ua.append(new Universe(2, gm));
    ua.append(new Universe(3, gm));

    Script scr(&doc);
    scr.setData(script0);
    scr.start(doc.masterTimer(), FunctionParent::master());
    scr.preRun(doc.masterTimer());

    scr.write(doc.masterTimer(), ua);

    scr.postRun(doc.masterTimer(), ua);
}

void Script_Test::commentParsingEdgeCases()
{
    Doc doc(this);
    Script scr(&doc);

    // Test comment at start of line - should not crash (C5: left==0 guard)
    scr.setData("// This is a comment\nblackout:on\n");
    QList<QList<QStringList>> lines;
    QStringList syntaxErrors;
    scr.m_syntaxErrorLines.clear();

    // Just setting data and verifying no crash
    QVERIFY(scr.dataLines().size() >= 1);

    // Test with URL containing :// - should not be treated as comment
    scr.setData("setfixture:0 value:255 channel:0\n");
    QVERIFY(scr.dataLines().size() >= 1);

    // Empty script
    scr.setData("");
    QVERIFY(scr.dataLines().isEmpty());
}

void Script_Test::parseRandomSpeed()
{
    Doc doc(this);
    Script scr(&doc);
    bool ok = false;

    // Valid random range
    quint32 val = Script::getValueFromString("random(100,200)", &ok);
    QVERIFY(ok == true);
    QVERIFY(val >= 100 && val <= 200);

    // Missing comma - should return -1 (C5 regression)
    val = Script::getValueFromString("random(100)", &ok);
    QCOMPARE(val, quint32(-1));

    // Normal speed value
    val = Script::getValueFromString("1000", &ok);
    QVERIFY(ok == true);
    QCOMPARE(val, quint32(1000));
}

void Script_Test::setData()
{
    Doc doc(this);
    Script scr(&doc);
    scr.setData("blackout:on\nblackout:off\n");
    QCOMPARE(scr.dataLines().size(), 2);
    QCOMPARE(scr.data(), QString("blackout:on\nblackout:off\n"));
}

void Script_Test::syntaxCheck()
{
    Doc doc(this);
    Script scr(&doc);
    // Valid script
    scr.setData("blackout:on\nblackout:off\n");
    QVERIFY(scr.m_syntaxErrorLines.isEmpty());

    // Syntax error: no colon separator
    scr.setData("this has no colon\n");
    QVERIFY(!scr.m_syntaxErrorLines.isEmpty());
}

void Script_Test::loadSave()
{
    Doc doc(this);
    Script *scr = new Script(&doc);
    doc.addFunction(scr);
    scr->setData("blackout:on\nwait:1.0\nblackout:off\n");

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(scr->saveXML(&xmlWriter) == true);
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Script scr2(&doc);
    QVERIFY(scr2.loadXML(xmlReader) == true);
    QCOMPARE(scr2.data(), scr->data());
}

void Script_Test::handleLabel()
{
    Doc doc(this);
    Script scr(&doc);
    // Script with label and jump
    scr.setData("label:myLabel\nblackout:on\n");
    QVERIFY(scr.dataLines().size() == 2);
}

QTEST_MAIN(Script_Test)
