/*
  Q Light Controller Plus - Test Unit
  audiobar_test.cpp

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
#include "audiobar.h"
#include "doc.h"
#undef private
#undef protected

#include "audiobar_test.h"

void AudioBar_Test::init()
{
    m_doc = new Doc(this);
}

void AudioBar_Test::cleanup()
{
    delete m_doc;
}

void AudioBar_Test::defaults()
{
    AudioBar bar;
    QCOMPARE(bar.m_type, 0);
    QCOMPARE(bar.m_value, uchar(0));
    QCOMPARE(bar.m_divisor, 1);
}

void AudioBar_Test::copy()
{
    AudioBar bar(1, 42, 100);
    bar.setName("foo");
    AudioBar* cp = bar.createCopy();
    QVERIFY(cp != NULL);
    QCOMPARE(cp->m_type, bar.m_type);
    QCOMPARE(cp->m_value, bar.m_value);
    QCOMPARE(cp->m_parentId, bar.m_parentId);
    delete cp;
}

QTEST_MAIN(AudioBar_Test)
