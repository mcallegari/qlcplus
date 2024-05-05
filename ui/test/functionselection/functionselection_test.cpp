/*
  Q Light Controller
  functionselection_test.cpp

  Copyright (C) Jano Svitok

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
#include "functionselection.h"
#undef protected
#undef protected

#include "functionselection_test.h"
#include "doc.h"

void FunctionSelection_Test::initTestCase()
{
    m_doc = new Doc(this);

    QSettings settings;
    m_savedFilter = settings.value(SETTINGS_FILTER);
}

void FunctionSelection_Test::cleanupTestCase()
{
    QSettings settings;
    if (m_savedFilter.isValid())
    {
        settings.setValue(SETTINGS_FILTER, m_savedFilter);
    }
    else
    {
        settings.remove(SETTINGS_FILTER);
    }

    delete m_doc;
    m_doc = NULL;
}

void FunctionSelection_Test::initial()
{
    QWidget w;

    FunctionSelection fs(&w, m_doc);

    QCOMPARE(fs.parentWidget(), &w);
}

void FunctionSelection_Test::rememberFilter()
{
    QWidget w;
    int filter = 0;

    {
        FunctionSelection fs1(&w, m_doc);

        fs1.slotEFXChecked(false);
        fs1.slotSceneChecked(false);
        fs1.slotChaserChecked(true);

        filter = fs1.m_filter;
    }

    {
        FunctionSelection fs2(&w, m_doc);
        QCOMPARE(fs2.m_filter, filter);
    }
}

void FunctionSelection_Test::constFilter()
{
    QWidget w;
    int filter = 0;

    {
        FunctionSelection fs1(&w, m_doc);

        fs1.slotEFXChecked(false);
        fs1.slotSceneChecked(false);
        fs1.slotChaserChecked(true);

        filter = fs1.m_filter;
    }

    {
        FunctionSelection fs2(&w, m_doc);
        fs2.setFilter(Function::SceneType | Function::ChaserType, true);
        QCOMPARE(fs2.m_filter, Function::SceneType | Function::ChaserType);
    }

    {
        FunctionSelection fs3(&w, m_doc);
        QCOMPARE(fs3.m_filter, filter);
    }
}

QTEST_MAIN(FunctionSelection_Test)
