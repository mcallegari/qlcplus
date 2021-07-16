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

QTEST_MAIN(Script_Test)
