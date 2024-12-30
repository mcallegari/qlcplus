/*
  Q Light Controller Plus - Unit tests
  rgbmatrix_test.cpp

  Copyright (C) Heikki Junnila
                Massimo Callegari

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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#define protected public
#define private public
#include "rgbscriptscache.h"
#include "rgbmatrix_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fixturegroup.h"
#include "mastertimer.h"
#include "rgbmatrix.h"
#include "fixture.h"
#include "qlcfile.h"
#include "doc.h"
#undef private
#undef protected

#include "../common/resource_paths.h"

void RGBMatrix_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir fxiDir(INTERNAL_FIXTUREDIR);
    fxiDir.setFilter(QDir::Files);
    fxiDir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(fxiDir) == true);

    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Stairville", "LED PAR56");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    FixtureGroup* grp = new FixtureGroup(m_doc);
    grp->setName("Test Group");
    grp->setSize(QSize(5, 5));
    m_doc->addFixtureGroup(grp);

    for (int i = 0; i < 25; i++)
    {
        Fixture* fxi = new Fixture(m_doc);
        fxi->setFixtureDefinition(def, mode);
        fxi->setAddress(i * fxi->channels());
        m_doc->addFixture(fxi);

        grp->assignFixture(fxi->id());
    }

    QVERIFY(m_doc->rgbScriptsCache()->load(QDir(INTERNAL_SCRIPTDIR)));
    QVERIFY(m_doc->rgbScriptsCache()->names().size() != 0);
}

void RGBMatrix_Test::cleanupTestCase()
{
    delete m_doc;
}

void RGBMatrix_Test::initial()
{
    RGBMatrix mtx(m_doc);
    QCOMPARE(mtx.type(), Function::RGBMatrixType);
    QCOMPARE(mtx.fixtureGroup(), FixtureGroup::invalidId());
    QCOMPARE(mtx.getColor(0), QColor(Qt::red));
    QCOMPARE(mtx.getColor(1), QColor());
    QCOMPARE(mtx.m_fadersMap.count(), 0);
    QCOMPARE(mtx.m_stepHandler->currentStepIndex(), 0);
    QCOMPARE(mtx.name(), tr("New RGB Matrix"));
    QCOMPARE(mtx.duration(), uint(500));
    QCOMPARE(mtx.totalDuration(), uint(0));
    QVERIFY(mtx.algorithm() != NULL);
    QCOMPARE(mtx.algorithm()->name(), QString("Stripes"));
    QCOMPARE(mtx.components().size(), 0);
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
    mtx.setColor(0, Qt::blue);
    QCOMPARE(mtx.getColor(0), QColor(Qt::blue));

    mtx.setColor(0, QColor());
    QCOMPARE(mtx.getColor(0), QColor());

    mtx.setColor(1, Qt::green);
    QCOMPARE(mtx.getColor(1), QColor(Qt::green));

    mtx.setColor(1, QColor());
    QCOMPARE(mtx.getColor(1), QColor());
}

void RGBMatrix_Test::copy()
{
    RGBMatrix mtx(m_doc);
    mtx.setColor(0, Qt::magenta);
    mtx.setColor(1, Qt::yellow);
    mtx.setFixtureGroup(0);
    mtx.setAlgorithm(RGBAlgorithm::algorithm(m_doc, "Stripes"));
    QVERIFY(mtx.algorithm() != NULL);

    RGBMatrix* copyMtx = qobject_cast<RGBMatrix*> (mtx.createCopy(m_doc));
    QVERIFY(copyMtx != NULL);
    QCOMPARE(copyMtx->getColor(0), QColor(Qt::magenta));
    QCOMPARE(copyMtx->getColor(1), QColor(Qt::yellow));
    QCOMPARE(copyMtx->fixtureGroup(), uint(0));
    QVERIFY(copyMtx->algorithm() != NULL);
    QVERIFY(copyMtx->algorithm() != mtx.algorithm()); // Different object pointer!
    QCOMPARE(copyMtx->algorithm()->name(), QString("Stripes"));
}

void RGBMatrix_Test::previewMaps()
{
    RGBMatrix mtx(m_doc);
    RGBMatrixStep handler;
    QVERIFY(mtx.algorithm() != NULL);
    QCOMPARE(mtx.algorithm()->name(), QString("Stripes"));

    int steps = mtx.stepsCount();
    QCOMPARE(steps, 0);

    mtx.previewMap(0, &handler);
    QCOMPARE(handler.m_map.size(), 0); // No fixture group

    mtx.setFixtureGroup(0);
    steps = mtx.stepsCount();
    QCOMPARE(steps, 5);
    QCOMPARE(mtx.components().size(), 25);
    QCOMPARE(mtx.totalDuration(), uint(2500));

    mtx.setTotalDuration(8000);
    QCOMPARE(mtx.totalDuration(), uint(8000));

    mtx.previewMap(0, &handler);
    QCOMPARE(handler.m_map.size(), 5);

    for (int z = 0; z < steps; z++)
    {
        mtx.previewMap(z, &handler);
        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 5; x++)
            {
                if (x == z)
                    QCOMPARE(handler.m_map[y][x], QColor(Qt::black).rgb());
                else
                    QCOMPARE(handler.m_map[y][x], uint(0));
            }
        }
    }
}

void RGBMatrix_Test::property()
{
    RGBMatrix mtx(m_doc);
    QVERIFY(mtx.algorithm() != NULL);
    QCOMPARE(mtx.algorithm()->name(), QString("Stripes"));

    // check on invalid property
    QCOMPARE(mtx.property("foo"), QString());

    // check a valid property
    QCOMPARE(mtx.property("orientation"), QString("Horizontal"));

    mtx.setProperty("orientation", "Vertical");

    QCOMPARE(mtx.property("orientation"), QString("Vertical"));
}

void RGBMatrix_Test::loadSave()
{
    RGBMatrix* mtx = new RGBMatrix(m_doc);
    mtx->setColor(0, Qt::magenta);
    mtx->setColor(1, Qt::blue);
    mtx->setColor(2, Qt::green);
    mtx->setColor(3, Qt::red);
    mtx->setColor(4, Qt::yellow);
    mtx->setControlMode(RGBMatrix::ControlModeRgb);
    mtx->setFixtureGroup(42);
    mtx->setAlgorithm(RGBAlgorithm::algorithm(m_doc, "Stripes"));
    QVERIFY(mtx->algorithm() != NULL);
    QCOMPARE(mtx->algorithm()->name(), QString("Stripes"));

    mtx->setName("Xyzzy");
    mtx->setDirection(Function::Backward);
    mtx->setRunOrder(Function::PingPong);
    mtx->setDuration(1200);
    mtx->setFadeInSpeed(10);
    mtx->setFadeOutSpeed(20);
    m_doc->addFunction(mtx);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(mtx->saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Function"));
    QCOMPARE(xmlReader.attributes().value("Type").toString(), QString("RGBMatrix"));
    QCOMPARE(xmlReader.attributes().value("ID").toString(), QString::number(mtx->id()));
    QCOMPARE(xmlReader.attributes().value("Name").toString(), QString("Xyzzy"));

    int speed = 0, dir = 0, run = 0, algo = 0, grp = 0, color1 = 0, color2 = 0, color3 = 0, color4 = 0, color5 = 0, colormode = 0;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Speed")
        {
            QCOMPARE(xmlReader.attributes().value("FadeIn").toString(), QString("10"));
            QCOMPARE(xmlReader.attributes().value("FadeOut").toString(), QString("20"));
            QCOMPARE(xmlReader.attributes().value("Duration").toString(), QString("1200"));
            speed++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Direction")
        {
            QCOMPARE(xmlReader.readElementText(), QString("Backward"));
            dir++;
        }
        else if (xmlReader.name().toString() == "RunOrder")
        {
            QCOMPARE(xmlReader.readElementText(), QString("PingPong"));
            run++;
        }
        else if (xmlReader.name().toString() == "Algorithm")
        {
            // RGBAlgorithms take care of Algorithm tag's contents
            algo++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Color")
        {
            bool ok = false;
            int colorNum = xmlReader.attributes().value("Index").toInt(&ok);
            QVERIFY(ok);

            switch (colorNum)
            {
                case 0:
                    QCOMPARE(xmlReader.readElementText().toUInt(), QColor(Qt::magenta).rgb());
                    color1++;
                break;
                case 1:
                    QCOMPARE(xmlReader.readElementText().toUInt(), QColor(Qt::blue).rgb());
                    color2++;
                break;
                case 2:
                    QCOMPARE(xmlReader.readElementText().toUInt(), QColor(Qt::green).rgb());
                    color3++;
                break;
                case 3:
                    QCOMPARE(xmlReader.readElementText().toUInt(), QColor(Qt::red).rgb());
                    color4++;
                break;
                case 4:
                    QCOMPARE(xmlReader.readElementText().toUInt(), QColor(Qt::yellow).rgb());
                    color5++;
                break;
                default:
                    // The color number can be between 1 and MAXINT, but here we expect only 5.
                    QVERIFY(colorNum > 0 && colorNum <= 5);
                break;
            }
        }
        else if (xmlReader.name().toString() == "FixtureGroup")
        {
            QCOMPARE(xmlReader.readElementText(), QString("42"));
            grp++;
        }
        else if (xmlReader.name().toString() == "ControlMode")
        {
            QCOMPARE(xmlReader.readElementText(), QString("RGB"));
            colormode++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
        }
    }

    QCOMPARE(speed, 1);
    QCOMPARE(dir, 1);
    QCOMPARE(run, 1);
    QCOMPARE(algo, 1);
    QCOMPARE(color1, 1);
    QCOMPARE(color2, 1);
    QCOMPARE(color3, 1);
    QCOMPARE(color4, 1);
    QCOMPARE(color5, 1);
    QCOMPARE(grp, 1);
    QCOMPARE(colormode, 1);

    xmlReader.setDevice(NULL);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    RGBMatrix mtx2(m_doc);
    QVERIFY(mtx2.loadXML(xmlReader) == true);
    QCOMPARE(mtx2.direction(), Function::Backward);
    QCOMPARE(mtx2.runOrder(), Function::PingPong);
    QCOMPARE(mtx2.getColor(0), QColor(Qt::magenta));
    QCOMPARE(mtx2.getColor(1), QColor(Qt::blue));
    QCOMPARE(mtx2.controlMode(), RGBMatrix::ControlModeRgb);
    QCOMPARE(mtx2.fixtureGroup(), uint(42));
    QVERIFY(mtx2.algorithm() != NULL);
    QCOMPARE(mtx2.algorithm()->name(), mtx->algorithm()->name());
    QCOMPARE(mtx2.duration(), uint(1200));
    QCOMPARE(mtx2.fadeInSpeed(), uint(10));
    QCOMPARE(mtx2.fadeOutSpeed(), uint(20));

    buffer.close();
    buffer.setData(QByteArray());

    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWriter.setDevice(&buffer);

    // Put some extra garbage in
    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeEndElement();

    buffer.close();
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);

    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(mtx2.loadXML(xmlReader) == false); // Not a function node

    xmlReader.setDevice(NULL);
    buffer.close();
    buffer.setData(QByteArray());

    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWriter.setDevice(&buffer);

    // Put some extra garbage in
    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Scene");
    xmlWriter.writeEndElement();

    buffer.close();
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);

    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();
    QVERIFY(mtx2.loadXML(xmlReader) == false); // Not an RGBMatrix node

}

QTEST_MAIN(RGBMatrix_Test)
