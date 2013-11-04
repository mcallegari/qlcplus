/*
  Q Light Controller - Unit tests
  qlcmacros_test.cpp

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

#include "qlcmacros_test.h"
#include "qlcmacros.h"

void QLCMacros_Test::min()
{
    QVERIFY(MIN(0, 5) == 0);
    QVERIFY(MIN(5, 0) == 0);
    QVERIFY(MIN(0.4, -4.3) == -4.3);
    QVERIFY(MIN('a', 'h') == 'a');
}

void QLCMacros_Test::max()
{
    QVERIFY(MAX(7, 4) == 7);
    QVERIFY(MAX(3, 7) == 7);
    QVERIFY(MAX(12.5, -500.2) == 12.5);
    QVERIFY(MAX('f', 'o') == 'o');
}

void QLCMacros_Test::clamp()
{
    QVERIFY(CLAMP(500, 0, 10) == 10);
    QVERIFY(CLAMP(11, 0, 10) == 10);
    QVERIFY(CLAMP(9, 0, 10) == 9);
    QVERIFY(CLAMP(10, 0, 10) == 10);
    QVERIFY(CLAMP(0, 0, 10) == 0);
    QVERIFY(CLAMP(10, 20, 30) == 20);
}

void QLCMacros_Test::scale()
{
    QVERIFY(SCALE(5, 0, 10, 0, 20) == 10);
    QVERIFY(SCALE(0, 0, 10, 0, 20000) == 0);
    QVERIFY(SCALE(64, 0, 127, 0, UCHAR_MAX) == 128);
}

QTEST_APPLESS_MAIN(QLCMacros_Test)
