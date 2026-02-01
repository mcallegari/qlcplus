/*
  Q Light Controller Plus - Test Unit
  fixtureselection_test.cpp

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
#include "fixtureselection.h"
#include "doc.h"
#undef private
#undef protected

#include "fixtureselection_test.h"

void FixtureSelection_Test::init()
{
    m_doc = new Doc(this);
}

void FixtureSelection_Test::cleanup()
{
    delete m_doc;
}

void FixtureSelection_Test::execDialog()
{
    FixtureSelection dlg(NULL, m_doc);
    QTimer::singleShot(100, &dlg, SLOT(reject()));
    QCOMPARE(dlg.exec(), int(QDialog::Rejected));
}

QTEST_MAIN(FixtureSelection_Test)
