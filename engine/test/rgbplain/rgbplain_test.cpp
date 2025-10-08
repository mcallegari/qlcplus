/*
  Q Light Controller Plus - Test Unit
  rgbplain_test.cpp

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
#include "rgbplain.h"
#undef private
#include "doc.h"
#include "rgbplain_test.h"

void RGBPlain_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void RGBPlain_Test::cleanupTestCase()
{
    delete m_doc;
}

void RGBPlain_Test::defaults()
{
    RGBPlain algo(m_doc);
    QCOMPARE(algo.rgbMapStepCount(QSize(4,4)), 1);
    QCOMPARE(algo.type(), RGBAlgorithm::Plain);
    QCOMPARE(algo.acceptColors(), 1);
}

void RGBPlain_Test::mapping()
{
    RGBPlain algo(m_doc);
    RGBMap map;
    algo.rgbMap(QSize(2,2), qRgb(1,2,3), 0, map);
    QCOMPARE(map.size(),2);
    QCOMPARE(map[0][0], (uint)qRgb(1,2,3));
}

QTEST_APPLESS_MAIN(RGBPlain_Test)
