/*
  Q Light Controller Plus - Test Unit
  simpledeskengine_test.cpp

  Copyright (c) Massimo Callegari

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

#define protected public
#define private public
#include "simpledeskengine.h"
#include "doc.h"
#undef protected
#undef private

#include "simpledeskengine_test.h"

void SimpleDeskEngine_Test::init()
{
    m_doc = new Doc(this);
}

void SimpleDeskEngine_Test::cleanup()
{
    delete m_doc;
}

void SimpleDeskEngine_Test::values()
{
    SimpleDeskEngine eng(m_doc);
    eng.setValue(5, 100);
    QCOMPARE(eng.value(5), uchar(100));
    QVERIFY(eng.hasChannel(5));
    eng.resetChannel(5);
    QVERIFY(!eng.hasChannel(5));
}

void SimpleDeskEngine_Test::cueStack()
{
    SimpleDeskEngine eng(m_doc);
    CueStack* stack = eng.cueStack(0);
    QVERIFY(stack != NULL);
    QCOMPARE(eng.cueStack(0), stack);
}

QTEST_MAIN(SimpleDeskEngine_Test)
