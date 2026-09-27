/*
  Q Light Controller Plus - Test Unit
  monitorproperties_test.cpp

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
#include <QBuffer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#define private public
#include "monitorproperties.h"
#undef private
#include "doc.h"
#include "monitorproperties_test.h"

void MonitorProperties_Test::defaults()
{
    MonitorProperties mp;

    QCOMPARE(mp.displayMode(), MonitorProperties::DMX);
    QCOMPARE(mp.channelStyle(), MonitorProperties::DMXChannels);
    QCOMPARE(mp.valueStyle(), MonitorProperties::DMXValues);
    QCOMPARE(mp.gridSize(), QVector3D(5, 3, 5));
    QCOMPARE(mp.gridUnits(), MonitorProperties::Meters);
    QCOMPARE(mp.pointOfView(), MonitorProperties::Undefined);
    QCOMPARE(mp.stageType(), MonitorProperties::StageSimple);
    QCOMPARE(mp.labelsVisible(), false);
    QVERIFY(mp.commonBackgroundImage().isEmpty());
}

void MonitorProperties_Test::fixtureItems()
{
    MonitorProperties mp;

    mp.setFixturePosition(10, 0, 0, QVector3D(1, 2, 3));
    mp.setFixtureRotation(10, 0, 0, QVector3D(0, 90, 0));
    mp.setFixtureGelColor(10, 0, 0, QColor(Qt::red));
    mp.setFixtureName(10, 0, 0, "Main");
    mp.setFixtureFlags(10, 0, 0, MonitorProperties::HiddenFlag);

    QCOMPARE(mp.fixturePosition(10,0,0), QVector3D(1,2,3));
    QCOMPARE(mp.fixtureRotation(10,0,0), QVector3D(0,90,0));
    QCOMPARE(mp.fixtureGelColor(10,0,0), QColor(Qt::red));
    QCOMPARE(mp.fixtureName(10,0,0), QString("Main"));
    QCOMPARE(mp.fixtureFlags(10,0,0), quint32(MonitorProperties::HiddenFlag));

    mp.removeFixture(10);
    QCOMPARE(mp.containsFixture(10), false);
}

void MonitorProperties_Test::lightItems()
{
    MonitorProperties mp;

    mp.setLightPosition("moving_head.dae", 0, QVector3D(1.5f, 2.5f, 3.5f));

    QList<QString> resources = mp.lightResources();
    QCOMPARE(resources.count(), 1);
    QCOMPARE(resources.first(), QString("moving_head.dae"));
    QCOMPARE(mp.containsLightEmitter("moving_head.dae", 0), true);
    QCOMPARE(mp.lightPosition("moving_head.dae", 0), QVector3D(1.5f, 2.5f, 3.5f));

    mp.removeLight("moving_head.dae");
    QCOMPARE(mp.containsLightEmitter("moving_head.dae", 0), false);
}

void MonitorProperties_Test::lightItemsXML()
{
    Doc doc(this);
    MonitorProperties mp;
    mp.setLightPosition("moving_head.dae", 0, QVector3D(1.5f, 2.5f, 3.5f));

    QByteArray xmlData;
    QBuffer buffer(&xmlData);
    QVERIFY(buffer.open(QIODevice::WriteOnly));

    QXmlStreamWriter writer(&buffer);
    writer.writeStartDocument();
    QVERIFY(mp.saveXML(&writer, &doc));
    writer.writeEndDocument();
    buffer.close();

    MonitorProperties loaded;
    QXmlStreamReader reader(xmlData);
    while (reader.readNextStartElement())
    {
        if (reader.name() == KXMLQLCMonitorProperties)
        {
            QVERIFY(loaded.loadXML(reader, &doc));
            break;
        }
        reader.skipCurrentElement();
    }

    QCOMPARE(loaded.lightPosition("moving_head.dae", 0), QVector3D(1.5f, 2.5f, 3.5f));
}

void MonitorProperties_Test::genericItems()
{
    MonitorProperties mp;

    quint32 id = 100;
    mp.setItemName(id, "Item");
    mp.setItemResource(id, "path");
    mp.setItemPosition(id, QVector3D(1,1,1));
    mp.setItemRotation(id, QVector3D(0,0,90));
    mp.setItemScale(id, QVector3D(2,2,2));
    mp.setItemFlags(id, MonitorProperties::InvertedPanFlag);

    QList<quint32> ids = mp.genericItemsID();
    QCOMPARE(ids.count(), 1);
    QCOMPARE(ids.first(), id);
    QCOMPARE(mp.itemName(id), QString("Item"));
    QCOMPARE(mp.itemResource(id), QString("path"));
    QCOMPARE(mp.itemPosition(id), QVector3D(1,1,1));
    QCOMPARE(mp.itemRotation(id), QVector3D(0,0,90));
    QCOMPARE(mp.itemScale(id), QVector3D(2,2,2));
    QCOMPARE(mp.itemFlags(id), quint32(MonitorProperties::InvertedPanFlag));

    // an item with no custom color reports the default one
    QCOMPARE(mp.itemColor(id), MonitorProperties::defaultItemColor());

    mp.setItemColor(id, QColor(Qt::blue));
    QCOMPARE(mp.itemColor(id), QColor(Qt::blue));

    // an item with no custom name is known by its resource file name
    quint32 unnamedID = 101;
    mp.setItemResource(unnamedID, "/some/where/curtain_tile_1m.obj");
    QCOMPARE(mp.itemName(unnamedID), QString("curtain_tile_1m"));

    mp.removeItem(id);
    QCOMPARE(mp.containsItem(id), false);
}

void MonitorProperties_Test::genericItemsXML()
{
    Doc doc(this);
    MonitorProperties mp;

    // one item with a custom name and color, one left at the defaults
    mp.setItemResource(1, "cube.obj");
    mp.setItemName(1, "Riser");
    mp.setItemColor(1, QColor(Qt::red));
    mp.setItemPosition(1, QVector3D(1000, 0, 2000));

    mp.setItemResource(2, "cube.obj");

    QByteArray xmlData;
    QBuffer buffer(&xmlData);
    QVERIFY(buffer.open(QIODevice::WriteOnly));

    QXmlStreamWriter writer(&buffer);
    writer.writeStartDocument();
    QVERIFY(mp.saveXML(&writer, &doc));
    writer.writeEndDocument();
    buffer.close();

    // an item with no custom color must be saved exactly as it was before
    // base colors existed, so that such a project stays readable by any
    // QLC+ version and looks the same in all of them
    QCOMPARE(xmlData.count("Color="), 1);

    MonitorProperties loaded;
    QXmlStreamReader reader(xmlData);
    while (reader.readNextStartElement())
    {
        if (reader.name() == KXMLQLCMonitorProperties)
        {
            QVERIFY(loaded.loadXML(reader, &doc));
            break;
        }
        reader.skipCurrentElement();
    }

    QCOMPARE(loaded.genericItemsID().count(), 2);
    QCOMPARE(loaded.itemName(1), QString("Riser"));
    QCOMPARE(loaded.itemColor(1), QColor(Qt::red));
    QCOMPARE(loaded.itemPosition(1), QVector3D(1000, 0, 2000));

    // an item saved with no color reads back as the default one, and keeps
    // taking its name from the mesh file
    QCOMPARE(loaded.itemName(2), QString("cube"));
    QCOMPARE(loaded.itemColor(2), MonitorProperties::defaultItemColor());
}

void MonitorProperties_Test::reset()
{
    MonitorProperties mp;
    mp.setGridSize(QVector3D(10,10,10));
    mp.setGridUnits(MonitorProperties::Feet);
    mp.setPointOfView(MonitorProperties::FrontView);
    mp.setStageType(MonitorProperties::StageBox);
    mp.setLabelsVisible(true);
    mp.setFixturePosition(1,0,0,QVector3D(1,2,3));
    mp.setItemName(2,"foo");
    mp.setCommonBackgroundImage("img.png");

    mp.reset();

    QCOMPARE(mp.gridSize(), QVector3D(5,3,5));
    QCOMPARE(mp.gridUnits(), MonitorProperties::Meters);
    QCOMPARE(mp.pointOfView(), MonitorProperties::Undefined);
    QCOMPARE(mp.stageType(), MonitorProperties::StageSimple);
    QCOMPARE(mp.labelsVisible(), false);
    QCOMPARE(mp.fixtureItemsID().count(), 0);
    QCOMPARE(mp.lightResources().count(), 0);
    QCOMPARE(mp.genericItemsID().count(), 0);
    QVERIFY(mp.commonBackgroundImage().isEmpty());
}

QTEST_APPLESS_MAIN(MonitorProperties_Test)
