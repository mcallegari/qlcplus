/*
  Q Light Controller - Unit test
  qlcpoint_test.cpp

  Copyright (c) Heikki Junnila

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

#include "qlcpoint_test.h"
#include "qlcpoint.h"

void QLCPoint_Test::initial()
{
    QLCPoint pt;
    QCOMPARE(pt.isNull(), true);

    pt = QLCPoint(1, 2);
    QCOMPARE(pt.isNull(), false);
    QCOMPARE(pt.x(), 1);
    QCOMPARE(pt.y(), 2);
}

void QLCPoint_Test::equals()
{
    QLCPoint pt1(1, 1);
    QLCPoint pt2(2, 2);
    QCOMPARE(pt1, pt1);
    QCOMPARE(pt2, pt2);
    QVERIFY(pt1 != pt2);
}

void QLCPoint_Test::hash()
{
    QLCPoint pt(10, 10);
    qDebug() << qHash(pt);
}

QTEST_APPLESS_MAIN(QLCPoint_Test)
