/*
  Q Light Controller Plus - Test Unit
  positiontool_test.cpp

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
#include "positiontool.h"
#undef private

#include "positiontool_test.h"

void PositionTool_Test::initial()
{
    QWidget w;
    PositionTool pt(QPointF(50, 60), QRectF(-90, -90, 180, 180), &w);
    QCOMPARE(pt.parentWidget(), &w);
    QCOMPARE(pt.position(), QPointF(50, 60));
}

void PositionTool_Test::position()
{
    PositionTool pt(QPointF(0, 0), QRectF(-90, -90, 180, 180));
    pt.setPosition(QPointF(10, 20));
    QCOMPARE(pt.position(), QPointF(10, 20));
}

QTEST_MAIN(PositionTool_Test)
