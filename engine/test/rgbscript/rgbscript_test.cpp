/*
  Q Light Controller
  rgbscript_test.cpp

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
#include "rgbscript_test.h"
#include "rgbscript.h"
#undef private

#define INTERNAL_SCRIPTDIR "../../../rgbscripts"

void RGBScript_Test::initial()
{
    RGBScript script;
    QVERIFY(script.s_engine == NULL);
    QCOMPARE(script.m_apiVersion, 0);
    QCOMPARE(script.m_fileName, QString());
    QCOMPARE(script.m_contents, QString());
}

void RGBScript_Test::directories()
{
    QDir dir = RGBScript::systemScriptDirectory();
    QCOMPARE(dir.filter(), QDir::Files);
    QCOMPARE(dir.nameFilters(), QStringList() << QString("*.js"));
#ifdef __APPLE__
    QString path("%1/../%2");
    QCOMPARE(dir.path(), path.arg(QCoreApplication::applicationDirPath())
                             .arg("Resources/RGBScripts"));
#elif WIN32
    QVERIFY(dir.path().endsWith("RGBScripts"));
#else
    QVERIFY(dir.path().endsWith("qlc/rgbscripts"));
#endif

    dir = RGBScript::userScriptDirectory();
    QCOMPARE(dir.filter(), QDir::Files);
    QCOMPARE(dir.nameFilters(), QStringList() << QString("*.js"));
#ifdef __APPLE__
    QVERIFY(dir.path().endsWith("Library/Application Support/QLC/RGBScripts"));
#elif WIN32
    QVERIFY(dir.path().endsWith("RGBScripts"));
#else
    QVERIFY(dir.path().endsWith(".qlc/rgbscripts"));
#endif

    dir = RGBScript::customScriptDirectory();
    QCOMPARE(dir.filter(), QDir::Files);
    QCOMPARE(dir.nameFilters(), QStringList() << QString("*.js"));
    QCOMPARE(dir.dirName(), QString("."));

    RGBScript::setCustomScriptDirectory(INTERNAL_SCRIPTDIR);
    dir = RGBScript::customScriptDirectory();
    QCOMPARE(dir.filter(), QDir::Files);
    QCOMPARE(dir.nameFilters(), QStringList() << QString("*.js"));
    QVERIFY(dir.path().endsWith(INTERNAL_SCRIPTDIR));
}

void RGBScript_Test::scripts()
{
    QDir dir(INTERNAL_SCRIPTDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*.js"));
    QVERIFY(dir.entryList().size() > 0);

    RGBScript::setCustomScriptDirectory(INTERNAL_SCRIPTDIR);
    QList <RGBScript> list = RGBScript::scripts();
    QVERIFY(list.size() >= 0);
}

void RGBScript_Test::script()
{
    RGBScript::setCustomScriptDirectory(INTERNAL_SCRIPTDIR);

    RGBScript s = RGBScript::script("A script that should not exist");
    QCOMPARE(s.fileName(), QString());
    QCOMPARE(s.m_contents, QString());
    QCOMPARE(s.apiVersion(), 0);
    QCOMPARE(s.author(), QString());
    QCOMPARE(s.name(), QString());
    QVERIFY(s.m_script.isValid() == false);
    QVERIFY(s.m_rgbMap.isValid() == false);
    QVERIFY(s.m_rgbMapStepCount.isValid() == false);

    s = RGBScript::script("Full Rows");
    QCOMPARE(s.fileName(), QString("fullrows.js"));
    QVERIFY(s.m_contents.isEmpty() == false);
    QVERIFY(s.apiVersion() > 0);
    QCOMPARE(s.author(), QString("Heikki Junnila"));
    QCOMPARE(s.name(), QString("Full Rows"));
    QVERIFY(s.m_script.isValid() == true);
    QVERIFY(s.m_rgbMap.isValid() == true);
    QVERIFY(s.m_rgbMapStepCount.isValid() == true);
}

void RGBScript_Test::evaluateException()
{
    // Should be    function()
    QString code("( function { return 5; } )()");
    RGBScript s;
    s.m_contents = code;
    QCOMPARE(s.evaluate(), false);
}

void RGBScript_Test::evaluateNoRgbMapFunction()
{
    // No rgbMap() function present
    QString code("( function() { return 5; } )()");
    RGBScript s;
    s.m_contents = code;
    QCOMPARE(s.evaluate(), false);
    QCOMPARE(s.rgbMap(QSize(5, 5), 1, 0), RGBMap());
}

void RGBScript_Test::evaluateNoRgbMapStepCountFunction()
{
    // No rgbMapStepCount() function present
    QString code("( function() { var foo = new Object; foo.rgbMap = function() { return 0; }; return foo; } )()");
    RGBScript s;
    s.m_contents = code;
    QCOMPARE(s.evaluate(), false);
    QCOMPARE(s.rgbMapStepCount(QSize(5, 5)), -1);
}

void RGBScript_Test::evaluateInvalidApiVersion()
{
    // No apiVersion property
    QString code("( function() { var foo = new Object; foo.rgbMap = function() { return 0; }; foo.rgbMapStepCount = function(width, height) { return 0; }; return foo; } )()");
    RGBScript s;
    s.m_contents = code;
    QCOMPARE(s.evaluate(), false);
}

void RGBScript_Test::rgbMapStepCount()
{
    RGBScript s = RGBScript::script("Full Rows");
    QCOMPARE(s.rgbMapStepCount(QSize(10, 15)), 15);
}

void RGBScript_Test::rgbMap()
{
    RGBScript s = RGBScript::script("Full Rows");
    QVERIFY(s.rgbMap(QSize(3, 4), 0, 0).isEmpty() == false);

    for (int z = 0; z < 5; z++)
    {
        RGBMap map = s.rgbMap(QSize(5, 5), QColor(Qt::red).rgb(), z);
        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 5; x++)
            {
                if (y == z)
                    QCOMPARE(map[y][x], QColor(Qt::red).rgb());
                else
                    QCOMPARE(map[y][x], uint(0));
            }
        }
    }
}

QTEST_MAIN(RGBScript_Test)
