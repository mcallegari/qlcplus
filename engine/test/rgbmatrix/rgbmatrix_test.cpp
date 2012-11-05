/*
  Q Light Controller
  rgbmatrix_test.cpp

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
#include <QtXml>

#define private public
#include "rgbmatrix_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universearray.h"
#include "fixturegroup.h"
#include "mastertimer.h"
#include "rgbscript.h"
#include "rgbmatrix.h"
#include "fixture.h"
#include "qlcfile.h"
#include "doc.h"
#undef private

#define INTERNAL_SCRIPTDIR "../../../rgbscripts/"
#define INTERNAL_FIXTUREDIR "../../../fixtures/"

void RGBMatrix_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir fxiDir(INTERNAL_FIXTUREDIR);
    fxiDir.setFilter(QDir::Files);
    fxiDir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(fxiDir) == true);

    const QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Stairville", "LED PAR56");
    QVERIFY(def != NULL);
    const QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    FixtureGroup* grp = new FixtureGroup(m_doc);
    grp->setName("Test Group");
    grp->setSize(QSize(5, 5));
    m_doc->addFixtureGroup(grp);

    for (int i = 0; i < 25; i++)
    {
        Fixture* fxi = new Fixture(m_doc);
        fxi->setFixtureDefinition(def, mode);
        m_doc->addFixture(fxi);

        grp->assignFixture(fxi->id());
    }

    RGBScript::setCustomScriptDirectory(INTERNAL_SCRIPTDIR);
    QVERIFY(RGBScript::scripts().size() != 0);
}

void RGBMatrix_Test::cleanupTestCase()
{
    delete m_doc;
}

void RGBMatrix_Test::initial()
{
    RGBMatrix mtx(m_doc);
    QCOMPARE(mtx.type(), Function::RGBMatrix);
    QCOMPARE(mtx.fixtureGroup(), FixtureGroup::invalidId());
    QCOMPARE(mtx.monoColor(), QColor(Qt::red));
    QVERIFY(mtx.m_fader == NULL);
    QCOMPARE(mtx.m_step, 0);
    QCOMPARE(mtx.name(), tr("New RGB Matrix"));
    QCOMPARE(mtx.duration(), uint(500));
    QVERIFY(mtx.algorithm() != NULL);
    QCOMPARE(mtx.algorithm()->name(), QString("Full Columns"));
}

void RGBMatrix_Test::group()
{
    RGBMatrix mtx(m_doc);
    mtx.setFixtureGroup(0);
    QCOMPARE(mtx.fixtureGroup(), uint(0));

    mtx.setFixtureGroup(15);
    QCOMPARE(mtx.fixtureGroup(), uint(15));

    mtx.setFixtureGroup(FixtureGroup::invalidId());
    QCOMPARE(mtx.fixtureGroup(), FixtureGroup::invalidId());
}

void RGBMatrix_Test::color()
{
    RGBMatrix mtx(m_doc);
    mtx.setMonoColor(Qt::blue);
    QCOMPARE(mtx.monoColor(), QColor(Qt::blue));

    mtx.setMonoColor(QColor());
    QCOMPARE(mtx.monoColor(), QColor());
}

void RGBMatrix_Test::copy()
{
    RGBMatrix mtx(m_doc);
    mtx.setMonoColor(Qt::magenta);
    mtx.setFixtureGroup(0);
    mtx.setAlgorithm(RGBAlgorithm::algorithm("Full Columns"));
    QVERIFY(mtx.algorithm() != NULL);

    RGBMatrix* copyMtx = qobject_cast<RGBMatrix*> (mtx.createCopy(m_doc));
    QVERIFY(copyMtx != NULL);
    QCOMPARE(copyMtx->monoColor(), QColor(Qt::magenta));
    QCOMPARE(copyMtx->fixtureGroup(), uint(0));
    QVERIFY(copyMtx->algorithm() != NULL);
    QVERIFY(copyMtx->algorithm() != mtx.algorithm()); // Different object pointer!
    QCOMPARE(copyMtx->algorithm()->name(), QString("Full Columns"));
}

void RGBMatrix_Test::previewMaps()
{
    RGBMatrix mtx(m_doc);
    QVERIFY(mtx.algorithm() != NULL);
    QCOMPARE(mtx.algorithm()->name(), QString("Full Columns"));

    QList <RGBMap> maps = mtx.previewMaps();
    QCOMPARE(maps.size(), 0); // No fixture group

    mtx.setFixtureGroup(0);
    maps = mtx.previewMaps();
    QCOMPARE(maps.size(), 5);
    for (int z = 0; z < 5; z++)
    {
        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 5; x++)
            {
                if (x == z)
                    QCOMPARE(maps[z][y][x], QColor(Qt::red).rgb());
                else
                    QCOMPARE(maps[z][y][x], uint(0));
            }
        }
    }
}

void RGBMatrix_Test::loadSave()
{
    RGBMatrix* mtx = new RGBMatrix(m_doc);
    mtx->setMonoColor(Qt::magenta);
    mtx->setFixtureGroup(42);
    mtx->setAlgorithm(RGBAlgorithm::algorithm("Full Rows"));
    QVERIFY(mtx->algorithm() != NULL);
    QCOMPARE(mtx->algorithm()->name(), QString("Full Rows"));

    mtx->setName("Xyzzy");
    mtx->setDirection(Function::Backward);
    mtx->setRunOrder(Function::PingPong);
    mtx->setDuration(1200);
    mtx->setFadeInSpeed(10);
    mtx->setFadeOutSpeed(20);
    m_doc->addFunction(mtx);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    QVERIFY(mtx->saveXML(&doc, &root) == true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("Function"));
    QCOMPARE(root.firstChild().toElement().attribute("Type"), QString("RGBMatrix"));
    QCOMPARE(root.firstChild().toElement().attribute("ID"), QString::number(mtx->id()));
    QCOMPARE(root.firstChild().toElement().attribute("Name"), QString("Xyzzy"));

    int speed = 0, dir = 0, run = 0, algo = 0, monocolor = 0, grp = 0;

    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Speed")
        {
            QCOMPARE(tag.attribute("FadeIn"), QString("10"));
            QCOMPARE(tag.attribute("FadeOut"), QString("20"));
            QCOMPARE(tag.attribute("Duration"), QString("1200"));
            speed++;
        }
        else if (tag.tagName() == "Direction")
        {
            QCOMPARE(tag.text(), QString("Backward"));
            dir++;
        }
        else if (tag.tagName() == "RunOrder")
        {
            QCOMPARE(tag.text(), QString("PingPong"));
            run++;
        }
        else if (tag.tagName() == "Algorithm")
        {
            // RGBAlgorithms take care of Algorithm tag's contents
            algo++;
        }
        else if (tag.tagName() == "MonoColor")
        {
            QCOMPARE(tag.text().toUInt(), QColor(Qt::magenta).rgb());
            monocolor++;
        }
        else if (tag.tagName() == "FixtureGroup")
        {
            QCOMPARE(tag.text(), QString("42"));
            grp++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(speed, 1);
    QCOMPARE(dir, 1);
    QCOMPARE(run, 1);
    QCOMPARE(algo, 1);
    QCOMPARE(monocolor, 1);
    QCOMPARE(grp, 1);

    // Put some extra garbage in
    QDomNode parent = node.parentNode();
    QDomElement foo = doc.createElement("Foo");
    root.firstChild().appendChild(foo);

    RGBMatrix mtx2(m_doc);
    QVERIFY(mtx2.loadXML(root.firstChild().toElement()) == true);
    QCOMPARE(mtx2.direction(), Function::Backward);
    QCOMPARE(mtx2.runOrder(), Function::PingPong);
    QCOMPARE(mtx2.monoColor(), QColor(Qt::magenta));
    QCOMPARE(mtx2.fixtureGroup(), uint(42));
    QVERIFY(mtx2.algorithm() != NULL);
    QCOMPARE(mtx2.algorithm()->name(), mtx->algorithm()->name());
    QCOMPARE(mtx2.duration(), uint(1200));
    QCOMPARE(mtx2.fadeInSpeed(), uint(10));
    QCOMPARE(mtx2.fadeOutSpeed(), uint(20));

    QVERIFY(mtx2.loadXML(root.toElement()) == false); // Not a function node
    root.firstChild().toElement().setAttribute("Type", "Scene");
    QVERIFY(mtx2.loadXML(root.firstChild().toElement()) == false); // Not an RGBMatrix node
}

QTEST_MAIN(RGBMatrix_Test)
