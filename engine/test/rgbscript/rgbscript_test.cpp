/*
  Q Light Controller
  rgbscript_test.cpp

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

#define private public
#include "rgbscript_test.h"
#include "rgbscript.h"
#include "rgbscriptscache.h"
#undef private

#include "doc.h"

#include "../common/resource_paths.h"

void RGBScript_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void RGBScript_Test::cleanupTestCase()
{
    delete m_doc;
}

void RGBScript_Test::initial()
{
    RGBScript script(m_doc);
    QVERIFY(script.s_engine == NULL);
    QCOMPARE(script.m_apiVersion, 0);
    QCOMPARE(script.m_fileName, QString());
    QCOMPARE(script.m_contents, QString());
}

void RGBScript_Test::directories()
{
    QDir dir = RGBScriptsCache::systemScriptsDirectory();
    QCOMPARE(dir.filter(), QDir::Files);
    QCOMPARE(dir.nameFilters(), QStringList() << QString("*.js"));
#if defined( __APPLE__) || defined(Q_OS_MAC)
    QString path("%1/../%2");
    QCOMPARE(dir.path(), path.arg(QCoreApplication::applicationDirPath())
                             .arg("Resources/RGBScripts"));
#elif defined(WIN32) || defined(Q_OS_WIN)
    QVERIFY(dir.path().endsWith("RGBScripts"));
#else
    QVERIFY(dir.path().endsWith("qlcplus/rgbscripts"));
#endif

    dir = RGBScriptsCache::userScriptsDirectory();
    QCOMPARE(dir.filter(), QDir::Files);
    QCOMPARE(dir.nameFilters(), QStringList() << QString("*.js"));
#if defined( __APPLE__) || defined(Q_OS_MAC)
    QVERIFY(dir.path().endsWith("Library/Application Support/QLC+/RGBScripts"));
#elif defined(WIN32) || defined(Q_OS_WIN)
    QVERIFY(dir.path().endsWith("RGBScripts"));
#else
    QVERIFY(dir.path().endsWith(".qlcplus/rgbscripts"));
#endif
}

void RGBScript_Test::scripts()
{
    QDir dir(INTERNAL_SCRIPTDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*.js"));
    QVERIFY(dir.entryList().size() > 0);

    QVERIFY(m_doc->rgbScriptsCache()->load(dir));
    QVERIFY(m_doc->rgbScriptsCache()->names().size() >= 0);
}

void RGBScript_Test::script()
{
    QVERIFY(m_doc->rgbScriptsCache()->load(QDir(INTERNAL_SCRIPTDIR)));

    RGBScript s = m_doc->rgbScriptsCache()->script("A script that should not exist");
    QCOMPARE(s.fileName(), QString());
    QCOMPARE(s.m_contents, QString());
    QCOMPARE(s.apiVersion(), 0);
    QCOMPARE(s.author(), QString());
    QCOMPARE(s.name(), QString());
    // QVERIFY(s.m_script.isValid() == false); // TODO: to be fixed !!
    QVERIFY(s.m_rgbMap.isValid() == false);
    QVERIFY(s.m_rgbMapStepCount.isValid() == false);

    s = m_doc->rgbScriptsCache()->script("Stripes");
    QCOMPARE(s.fileName(), QString("stripes.js"));
    QVERIFY(s.m_contents.isEmpty() == false);
    QVERIFY(s.apiVersion() > 0);
    QCOMPARE(s.author(), QString("Massimo Callegari"));
    QCOMPARE(s.name(), QString("Stripes"));
    QVERIFY(s.m_script.isValid() == true);
    QVERIFY(s.m_rgbMap.isValid() == true);
    QVERIFY(s.m_rgbMapStepCount.isValid() == true);
}

void RGBScript_Test::evaluateException()
{
    // Should be    function()
    QString code("( function { return 5; } )()");
    RGBScript s(m_doc);
    s.m_contents = code;
    QCOMPARE(s.evaluate(), false);
}

void RGBScript_Test::evaluateNoRgbMapFunction()
{
    // No rgbMap() function present
    QString code("( function() { return 5; } )()");
    RGBScript s(m_doc);
    s.m_contents = code;
    QCOMPARE(s.evaluate(), false);
    QCOMPARE(s.rgbMap(QSize(5, 5), 1, 0), RGBMap());
}

void RGBScript_Test::evaluateNoRgbMapStepCountFunction()
{
    // No rgbMapStepCount() function present
    QString code("( function() { var foo = new Object; foo.rgbMap = function() { return 0; }; return foo; } )()");
    RGBScript s(m_doc);
    s.m_contents = code;
    QCOMPARE(s.evaluate(), false);
    QCOMPARE(s.rgbMapStepCount(QSize(5, 5)), -1);
}

void RGBScript_Test::evaluateInvalidApiVersion()
{
    // No apiVersion property
    QString code("( function() { var foo = new Object; foo.rgbMap = function() { return 0; }; foo.rgbMapStepCount = function(width, height) { return 0; }; return foo; } )()");
    RGBScript s(m_doc);
    s.m_contents = code;
    QCOMPARE(s.evaluate(), false);
}

void RGBScript_Test::rgbMapStepCount()
{
    RGBScript s = m_doc->rgbScriptsCache()->script("Stripes");
    QCOMPARE(s.rgbMapStepCount(QSize(10, 15)), 10);
}

void RGBScript_Test::rgbMap()
{
    RGBScript s = m_doc->rgbScriptsCache()->script("Stripes");
    QVERIFY(s.rgbMap(QSize(3, 4), 0, 0).isEmpty() == false);

    s.setProperty("orientation", "Vertical");
    QVERIFY(s.property("orientation") == "Vertical");

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
