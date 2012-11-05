/*
  Q Light Controller
  script_test.cpp

  Copyright (C) Heikki Junnila

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

#define private public

#include "qlcfixturedefcache.h"
#include "universearray.h"
#include "mastertimer.h"
#include "script_test.h"
#include "script.h"
#include "doc.h"

#undef private

static QString script0(
"// Comment over there\n"
"startfunction:12\r"
"stopscript:5 // Comment in here\n"
"stopfunction:33 paska\n"
"waitkey:\"SHIFT+K\"\n"
"startfunction:\"54\"\r\n"
"wait:1.05\n"
"setdmx:12 uni:2 val:127\n"
"setfixture:99 value:255 channel:1\n"
);

void Script_Test::initTestCase()
{
}
void Script_Test::initial()
{
    Doc doc(this);
    UniverseArray ua(512 * 4);

    Script scr(&doc);
    scr.setData(script0);

    for (int i = 0; i < 9; i++)
        scr.executeCommand(i, doc.masterTimer(), &ua);
}

QTEST_APPLESS_MAIN(Script_Test)
