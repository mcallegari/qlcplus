/*
  Q Light Controller Plus - Test Unit
  fixturetreewidget_test.cpp

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
#include "fixturetreewidget.h"
#include "fixture.h"
#include "fixturegroup.h"
#include "doc.h"
#undef private
#undef protected

#include "fixturetreewidget_test.h"

void FixtureTreeWidget_Test::init()
{
    m_doc = new Doc(this);
}

void FixtureTreeWidget_Test::cleanup()
{
    delete m_doc;
}

void FixtureTreeWidget_Test::treeCounts()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(4);
    m_doc->addFixture(fxi);

    FixtureTreeWidget tree(m_doc, FixtureTreeWidget::UniverseNumber);
    tree.updateTree();
    QVERIFY(tree.universeCount() >= 1);
    QVERIFY(tree.fixturesCount() >= 1);
    QVERIFY(tree.channelsCount() >= 4);
}

QTEST_MAIN(FixtureTreeWidget_Test)
