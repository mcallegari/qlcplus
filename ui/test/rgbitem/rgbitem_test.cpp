/*
  Q Light Controller Plus - Test Unit
  rgbitem_test.cpp

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
#include <QGraphicsRectItem>

#define protected public
#define private public
#include "rgbitem.h"
#undef private
#undef protected

#include "rgbitem_test.h"

void RGBItem_Test::basics()
{
    RGBItem item(new QGraphicsRectItem());
    item.setColor(QColor(Qt::red).rgb());
    QCOMPARE(item.color(), QColor(Qt::red).rgb());
}

void RGBItem_Test::draw()
{
    QGraphicsRectItem *gi = new QGraphicsRectItem();
    RGBItem item(gi);
    item.setColor(QColor(Qt::green).rgb());
    item.draw(50, 0);
    QCOMPARE(gi->brush().color(), QColor(Qt::green));
}

QTEST_MAIN(RGBItem_Test)
