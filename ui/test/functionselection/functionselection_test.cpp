/*
  Q Light Controller
  functionselection_test.cpp

  Copyright (C) Jano Svitok

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    if(m_savedFilter.isValid())
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
        fs2.setFilter(Function::Scene | Function::Chaser, true);
        QCOMPARE(fs2.m_filter, Function::Scene | Function::Chaser );
    }

    {
        FunctionSelection fs3(&w, m_doc);
        QCOMPARE(fs3.m_filter, filter);
    }
}

QTEST_MAIN(FunctionSelection_Test)
