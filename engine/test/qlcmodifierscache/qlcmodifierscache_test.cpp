/*
  Q Light Controller Plus - Test Unit
  qlcmodifierscache_test.cpp

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
#define private public
#include "qlcmodifierscache.h"
#undef private
#include "channelmodifier.h"
#include "qlcmodifierscache_test.h"

void QLCModifiersCache_Test::addAndRetrieve()
{
    QLCModifiersCache cache;
    ChannelModifier *m1 = new ChannelModifier();
    m1->setName("mod1");
    QVERIFY(cache.addModifier(m1) == true);
    QVERIFY(cache.addModifier(m1) == false);

    ChannelModifier *m2 = new ChannelModifier();
    m2->setName("mod2");
    QVERIFY(cache.addModifier(m2) == true);

    QList<QString> names = cache.templateNames();
    QCOMPARE(names.count(), 2);
    QVERIFY(names.contains("mod1"));
    QVERIFY(names.contains("mod2"));

    QCOMPARE(cache.modifier("mod1"), m1);
    QCOMPARE(cache.modifier("nonexist"), static_cast<ChannelModifier*>(nullptr));
}

QTEST_APPLESS_MAIN(QLCModifiersCache_Test)
