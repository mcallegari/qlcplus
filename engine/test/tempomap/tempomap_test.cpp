/*
  Q Light Controller Plus - Unit test
  tempomap_test.cpp

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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "tempomap_test.h"
#include "tempomap.h"

void TempoMap_Test::sections()
{
    TempoMap map;
    QVERIFY(map.isEmpty());

    QCOMPARE(map.addSection(TempoSection(10000, 5000, 100, 4, "B")), 0);
    QCOMPARE(map.addSection(TempoSection(0, 5000, 120, 4, "A")), 0);
    QCOMPARE(map.count(), 2);
    QCOMPARE(map.section(0).name, QString("A"));
    QCOMPARE(map.section(1).name, QString("B"));
    QCOMPARE(map.section(1).endTime(), quint32(15000));

    // invalid sections are refused
    QCOMPARE(map.addSection(TempoSection(20000, 0, 120)), -1);
    QCOMPARE(map.addSection(TempoSection(20000, 1000, 0)), -1);
    QCOMPARE(map.addSection(TempoSection(20000, 1000, 120, 0)), -1);

    // moving a section keeps the list sorted
    QVERIFY(map.updateSection(0, TempoSection(20000, 1000, 90, 3, "A")));
    QCOMPARE(map.section(0).name, QString("B"));
    QCOMPARE(map.section(1).name, QString("A"));

    QVERIFY(map.removeSection(0));
    QCOMPARE(map.count(), 1);
    QVERIFY(map.removeSection(5) == false);

    map.clear();
    QVERIFY(map.isEmpty());
}

void TempoMap_Test::overlap()
{
    TempoMap map;
    QCOMPARE(map.addSection(TempoSection(1000, 1000, 120)), 0);

    // touching is fine, overlapping is not
    QVERIFY(map.canPlace(TempoSection(0, 1000, 120)));
    QVERIFY(map.canPlace(TempoSection(2000, 1000, 120)));
    QVERIFY(map.canPlace(TempoSection(0, 1001, 120)) == false);
    QVERIFY(map.canPlace(TempoSection(1999, 10, 120)) == false);
    QVERIFY(map.canPlace(TempoSection(1200, 100, 120)) == false);
    QVERIFY(map.canPlace(TempoSection(500, 2000, 120)) == false);

    // a section doesn't overlap with itself
    QVERIFY(map.canPlace(TempoSection(1500, 1000, 120), 0));

    QCOMPARE(map.addSection(TempoSection(2000, 500, 120)), 1);
    QVERIFY(map.updateSection(0, TempoSection(1500, 1000, 120)) == false);
    QCOMPARE(map.section(0).startTime, quint32(1000));
}

void TempoMap_Test::split()
{
    TempoMap map;
    map.addSection(TempoSection(1000, 9000, 128, 4, "Song"));

    QVERIFY(map.splitSection(0, 1000) == false);
    QVERIFY(map.splitSection(0, 10000) == false);
    QVERIFY(map.splitSection(0, 4000));

    QCOMPARE(map.count(), 2);
    QCOMPARE(map.section(0), TempoSection(1000, 3000, 128, 4, "Song"));
    QCOMPARE(map.section(1), TempoSection(4000, 6000, 128, 4, "Song"));
}

void TempoMap_Test::sectionIndexAt()
{
    TempoMap map;
    map.addSection(TempoSection(1000, 1000, 120));
    map.addSection(TempoSection(3000, 1000, 120));

    QCOMPARE(map.sectionIndexAt(0), -1);
    QCOMPARE(map.sectionIndexAt(1000), 0);
    QCOMPARE(map.sectionIndexAt(1999.9), 0);
    QCOMPARE(map.sectionIndexAt(2000), -1);
    QCOMPARE(map.sectionIndexAt(3500), 1);
    QCOMPARE(map.sectionIndexAt(4000), -1);
}

void TempoMap_Test::stepEndOnGrid()
{
    TempoMap map;
    map.addSection(TempoSection(1000, 60000, 120));

    // 120 BPM: a beat is 500 ms, beat 0 at 1000 ms
    QCOMPARE(map.stepEnd(1000, 1, 60), 1500.0);
    QCOMPARE(map.stepEnd(1500, 4, 60), 3500.0);
    // on grid starts keep steps that aren't whole beats
    QCOMPARE(map.stepEnd(1000, 1.5, 60), 1750.0);
    QCOMPARE(map.stepEnd(1000, 0, 60), 1000.0);
}

void TempoMap_Test::stepEndOffGrid()
{
    TempoMap map;
    map.addSection(TempoSection(0, 60000, 120));

    // started at beat 1.46: ends on beat 2 rather than 2.46
    QCOMPARE(map.stepEnd(730, 1, 60), 1000.0);
    // started at beat 1.8: ends on beat 3, the grid point nearest to 2.8
    QCOMPARE(map.stepEnd(900, 1, 60), 1500.0);
    // started at beat 1.2 for 2 beats: ends on beat 3
    QCOMPARE(map.stepEnd(600, 2, 60), 1500.0);

    // a whole ms rounded start is still on the grid: at 127.5 BPM beat 3 is
    // at 1411.76 ms, stored as 1412 ms
    TempoMap map2;
    map2.addSection(TempoSection(0, 60000, 127.5));
    double beat = 60000.0 / 127.5;
    QVERIFY(qAbs(map2.stepEnd(1412, 1, 60) - (4 * beat)) < 0.0001);
    QVERIFY(qAbs(map2.stepEnd(1412, 1.5, 60) - (4.5 * beat)) < 0.0001);
}

void TempoMap_Test::stepEndShortSteps()
{
    TempoMap map;
    map.addSection(TempoSection(0, 60000, 120));

    // half beat steps lock to half beats
    QCOMPARE(map.stepEnd(1000, 0.5, 60), 1250.0);
    QCOMPARE(map.stepEnd(600, 0.5, 60), 750.0);
    // quarter beat steps lock to quarter beats
    QCOMPARE(map.stepEnd(1010, 0.25, 60), 1125.0);
}

void TempoMap_Test::stepEndNoDrift()
{
    TempoMap map;
    map.addSection(TempoSection(1000, 3600000, 127.5));
    double beat = 60000.0 / 127.5;

    // 10000 one-beat steps chained end to start: about 78 minutes
    double time = 1000;
    for (int i = 0; i < 10000; i++)
        time = map.stepEnd(time, 1, 60);

    QVERIFY(qAbs(time - (1000 + 10000 * beat)) < 0.001);

    // three-quarter beat steps land on multiples of three quarters of a beat
    time = 1000;
    for (int i = 0; i < 1000; i++)
        time = map.stepEnd(time, 0.75, 60);

    QVERIFY(qAbs(time - (1000 + 750 * beat)) < 0.001);
}

void TempoMap_Test::stepEndNextSection()
{
    TempoMap map;
    map.addSection(TempoSection(0, 1000, 120));
    map.addSection(TempoSection(1000, 10000, 60));

    // 4 beats from beat 1 of the first section: 1 beat left in it (ends at
    // 1000 ms), then 2 beats at 60 BPM
    QCOMPARE(map.stepEnd(500, 3, 30), 3000.0);
    // ending exactly on the boundary stays in the first section
    QCOMPARE(map.stepEnd(500, 1, 30), 1000.0);

    // a step spanning a whole short section
    TempoMap map2;
    map2.addSection(TempoSection(0, 1000, 120));
    map2.addSection(TempoSection(1000, 500, 120));
    map2.addSection(TempoSection(1500, 10000, 60));
    // 2 beats in the first section, 1 in the second, 1 in the third
    QCOMPARE(map2.stepEnd(0, 4, 30), 2500.0);
}

void TempoMap_Test::stepEndGap()
{
    TempoMap map;
    map.addSection(TempoSection(0, 1000, 120));
    map.addSection(TempoSection(5000, 1000, 60));

    // the gap after a section keeps its tempo and grid
    QCOMPARE(map.stepEnd(1000, 2, 30), 2000.0);
    QCOMPARE(map.stepEnd(1100, 1, 30), 1500.0);

    // up to the next section, where its grid takes over
    QCOMPARE(map.stepEnd(4500, 2, 30), 6000.0);
}

void TempoMap_Test::stepEndBeforeFirstSection()
{
    TempoMap map;
    map.addSection(TempoSection(10000, 10000, 120));

    // before the first section, the fallback BPM with a grid from 0
    QCOMPARE(map.stepEnd(0, 1, 60), 1000.0);
    QCOMPARE(map.stepEnd(2300, 1, 60), 3000.0);
    // then the section grid
    QCOMPARE(map.stepEnd(9000, 2, 60), 10500.0);

    // no sections at all: the fallback BPM
    TempoMap empty;
    QCOMPARE(empty.stepEnd(0, 2, 120), 1000.0);
}

void TempoMap_Test::beatDurationAt()
{
    TempoMap map;
    map.addSection(TempoSection(1000, 1000, 120));
    map.addSection(TempoSection(3000, 1000, 100));

    QCOMPARE(map.beatDurationAt(0, 60), 1000.0);
    QCOMPARE(map.beatDurationAt(1000, 60), 500.0);
    QCOMPARE(map.beatDurationAt(2500, 60), 500.0);
    QCOMPARE(map.beatDurationAt(3000, 60), 600.0);
}

void TempoMap_Test::saveLoad()
{
    TempoMap map;
    map.addSection(TempoSection(12500, 215000, 127.5, 4, "Song A"));
    map.addSection(TempoSection(230000, 180000, 96, 3));

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    QVERIFY(map.saveXML(&xmlWriter));
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("TempoMap"));

    TempoMap loaded;
    QVERIFY(loaded.loadXML(xmlReader));
    QCOMPARE(loaded.count(), 2);
    QCOMPARE(loaded.section(0), map.section(0));
    QCOMPARE(loaded.section(1), map.section(1));

    // an empty map writes nothing
    QBuffer emptyBuffer;
    emptyBuffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter emptyWriter(&emptyBuffer);
    QVERIFY(TempoMap().saveXML(&emptyWriter));
    QCOMPARE(emptyBuffer.size(), qint64(0));
}

QTEST_APPLESS_MAIN(TempoMap_Test)
