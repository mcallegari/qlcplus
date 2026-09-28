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
#include <cmath>

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

void TempoMap_Test::insertOverlappingStart()
{
    // two songs overlapping at the edges: the new song takes over at its start
    TempoMap map;
    map.addSection(TempoSection(0, 200000, 128, 4, "Song 1"));

    QCOMPARE(map.insertSection(TempoSection(195000, 205000, 96.5, 4, "Song 2")), 1);
    QCOMPARE(map.count(), 2);
    QCOMPARE(map.section(0), TempoSection(0, 195000, 128, 4, "Song 1"));
    QCOMPARE(map.section(1), TempoSection(195000, 205000, 96.5, 4, "Song 2"));

    // no overlap: same as addSection
    QCOMPARE(map.insertSection(TempoSection(500000, 1000, 120, 4, "Song 4")), 2);
    QCOMPARE(map.count(), 3);
}

void TempoMap_Test::insertClipsEnd()
{
    // the next existing section keeps its start
    TempoMap map;
    map.addSection(TempoSection(0, 200000, 128, 4, "Song 1"));
    map.addSection(TempoSection(395000, 100000, 140, 4, "Song 3"));

    QCOMPARE(map.insertSection(TempoSection(195000, 205000, 96.5, 4, "Song 2")), 1);
    QCOMPARE(map.count(), 3);
    QCOMPARE(map.section(0), TempoSection(0, 195000, 128, 4, "Song 1"));
    QCOMPARE(map.section(1), TempoSection(195000, 200000, 96.5, 4, "Song 2"));
    QCOMPARE(map.section(2), TempoSection(395000, 100000, 140, 4, "Song 3"));
}

void TempoMap_Test::insertInside()
{
    // 120 BPM: 500 ms beats from 1000. The outer section carries on after
    // the inserted one from the next beat of its own grid
    TempoMap map;
    map.addSection(TempoSection(1000, 20000, 120, 4, "Song"));

    QCOMPARE(map.insertSection(TempoSection(5000, 3200, 90, 3, "Bridge")), 1);
    QCOMPARE(map.count(), 3);
    QCOMPARE(map.section(0), TempoSection(1000, 4000, 120, 4, "Song"));
    QCOMPARE(map.section(1), TempoSection(5000, 3200, 90, 3, "Bridge"));
    QCOMPARE(map.section(2), TempoSection(8500, 12500, 120, 4, "Song"));

    // ending on a beat of the outer grid: no gap
    TempoMap onBeat;
    onBeat.addSection(TempoSection(1000, 20000, 120, 4, "Song"));
    QCOMPARE(onBeat.insertSection(TempoSection(5000, 3000, 90, 3, "Bridge")), 1);
    QCOMPARE(onBeat.section(2), TempoSection(8000, 13000, 120, 4, "Song"));

    // less than a beat left after the inserted section: no remainder
    TempoMap tail;
    tail.addSection(TempoSection(1000, 10000, 120, 4, "Song"));
    QCOMPARE(tail.insertSection(TempoSection(5000, 5800, 90, 3, "Outro")), 1);
    QCOMPARE(tail.count(), 2);
    QCOMPARE(tail.section(0), TempoSection(1000, 4000, 120, 4, "Song"));
    QCOMPARE(tail.section(1), TempoSection(5000, 5800, 90, 3, "Outro"));
}

void TempoMap_Test::insertRejected()
{
    TempoMap map;
    map.addSection(TempoSection(1000, 1000, 120, 4, "Song"));

    // a section already starts there
    QCOMPARE(map.insertSection(TempoSection(1000, 5000, 90)), -1);
    // invalid
    QCOMPARE(map.insertSection(TempoSection(3000, 0, 90)), -1);
    QCOMPARE(map.insertSection(TempoSection(3000, 1000, 0)), -1);

    QCOMPARE(map.count(), 1);
    QCOMPARE(map.section(0), TempoSection(1000, 1000, 120, 4, "Song"));
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

    // without sections, the fallback segment has no end, even past 2^32 ms
    QCOMPARE(empty.stepEnd(4294967000.0, 2, 120), 4294968000.0);
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

static double moveTo(const TempoMap &map, TempoMap::BeatCursor &cursor, double time, double fallbackBpm)
{
    return map.moveBeatCursor(cursor, time, fallbackBpm);
}

void TempoMap_Test::beatCursorNoSections()
{
    TempoMap map;
    TempoMap::BeatCursor cursor(0);

    // the fallback BPM, on a grid from 0
    QCOMPARE(moveTo(map, cursor, 1000, 120), 2.0);
    QCOMPARE(moveTo(map, cursor, 1250, 120), 2.5);
    // moving back does nothing
    QCOMPARE(moveTo(map, cursor, 1000, 120), 2.5);

    // started off the grid, 0.2 beats after a beat: the count is eased
    // onto the grid over a beat, and then its whole beats are grid beats
    TempoMap::BeatCursor offGrid(100);
    QVERIFY(moveTo(map, offGrid, 300, 120) < 0.4 + 0.2);
    for (int t = 600; t <= 3000; t += 100)
        QVERIFY(qAbs(moveTo(map, offGrid, t, 120) - t / 500.0) < 1e-9);
}

void TempoMap_Test::beatCursorOnGrid()
{
    TempoMap map;
    map.addSection(TempoSection(0, 60000, 120));

    TempoMap::BeatCursor cursor(1000);
    for (int t = 1020; t <= 5000; t += 20)
        QVERIFY(qAbs(moveTo(map, cursor, t, 60) - (t - 1000) / 500.0) < 1e-9);
}

void TempoMap_Test::beatCursorAdjacentSections()
{
    TempoMap map;
    map.addSection(TempoSection(0, 4000, 120));
    map.addSection(TempoSection(4000, 20000, 90));

    TempoMap::BeatCursor cursor(0);
    QCOMPARE(moveTo(map, cursor, 4000, 60), 8.0);

    // the section starts on a beat: the count goes on at 90 BPM, on its grid
    double beatMs = 60000.0 / 90;
    for (int i = 1; i <= 6; i++)
        QVERIFY(qAbs(moveTo(map, cursor, 4000 + i * beatMs, 60) - (8 + i)) < 1e-9);
}

void TempoMap_Test::beatCursorAdjacentOffGrid()
{
    TempoMap map;
    map.addSection(TempoSection(0, 4250, 120));
    map.addSection(TempoSection(4250, 20000, 60));

    TempoMap::BeatCursor cursor(0);
    QCOMPARE(moveTo(map, cursor, 4250, 120), 8.5);

    // halfway through a beat when the next section starts: the half beat
    // is caught up over the first beat of the section, without any jump
    QVERIFY(qAbs(moveTo(map, cursor, 4260, 120) - 8.5) < 0.02);
    QVERIFY(qAbs(moveTo(map, cursor, 5250, 120) - 10.0) < 1e-9);
    for (int i = 2; i <= 5; i++)
        QVERIFY(qAbs(moveTo(map, cursor, 4250 + i * 1000, 120) - (9 + i)) < 1e-9);
}

void TempoMap_Test::beatCursorGap()
{
    TempoMap map;
    map.addSection(TempoSection(0, 2000, 120));
    map.addSection(TempoSection(5000, 20000, 60));

    // the gap keeps the tempo of the section before it
    TempoMap::BeatCursor cursor(1000);
    QCOMPARE(moveTo(map, cursor, 2000, 90), 2.0);
    QCOMPARE(moveTo(map, cursor, 3500, 90), 5.0);
    QCOMPARE(moveTo(map, cursor, 5000, 90), 8.0);
    QCOMPARE(moveTo(map, cursor, 7000, 90), 10.0);

    // the next section starting off the gap grid: eased onto its grid
    TempoMap offGridMap;
    offGridMap.addSection(TempoSection(0, 2000, 120));
    offGridMap.addSection(TempoSection(5100, 20000, 60));

    TempoMap::BeatCursor offGrid(1000);
    QVERIFY(qAbs(moveTo(offGridMap, offGrid, 5100, 90) - 8.2) < 1e-9);
    QVERIFY(qAbs(moveTo(offGridMap, offGrid, 6100, 90) - 9.0) < 1e-9);
    QVERIFY(qAbs(moveTo(offGridMap, offGrid, 8100, 90) - 11.0) < 1e-9);
}

void TempoMap_Test::beatCursorBeforeFirstSection()
{
    TempoMap map;
    map.addSection(TempoSection(3000, 10000, 60));

    // the fallback BPM up to the section, then the section tempo
    TempoMap::BeatCursor cursor(1000);
    QCOMPARE(moveTo(map, cursor, 2000, 120), 2.0);
    QCOMPARE(moveTo(map, cursor, 3000, 120), 4.0);
    QCOMPARE(moveTo(map, cursor, 5000, 120), 6.0);
}

void TempoMap_Test::beatCursorPastLastSection()
{
    TempoMap map;
    map.addSection(TempoSection(0, 2000, 90));
    double beatMs = 60000.0 / 90;

    // started 0.2 beats after a grid beat, running well past the end of
    // the last section: 90 BPM on its grid all along
    TempoMap::BeatCursor cursor(800);
    moveTo(map, cursor, 800 + beatMs, 120);
    for (int t = 2000; t <= 12000; t += 500)
    {
        double beats = moveTo(map, cursor, t, 120);
        double offset = beats - t / beatMs;
        QVERIFY(qAbs(offset - std::round(offset)) < 1e-9);
    }
    double before = moveTo(map, cursor, 20000, 120);
    QVERIFY(qAbs(moveTo(map, cursor, 21000, 120) - before - 1.5) < 1e-9);
}

void TempoMap_Test::beatCursorFallbackBpmChange()
{
    TempoMap map;

    // a BPM change on a beat: the count goes on at the new tempo
    TempoMap::BeatCursor cursor(0);
    QCOMPARE(moveTo(map, cursor, 1000, 120), 2.0);
    QCOMPARE(moveTo(map, cursor, 1000, 60), 2.0);
    QCOMPARE(moveTo(map, cursor, 2000, 60), 3.0);

    // a BPM change between two beats: no jump, then eased onto the grid
    TempoMap::BeatCursor offBeat(0);
    QCOMPARE(moveTo(map, offBeat, 1250, 120), 2.5);
    QVERIFY(qAbs(moveTo(map, offBeat, 1260, 60) - 2.5) < 0.02);
    QVERIFY(qAbs(moveTo(map, offBeat, 2250, 60) - 3.25) < 1e-9);
    QVERIFY(qAbs(moveTo(map, offBeat, 4250, 60) - 5.25) < 1e-9);

    // within a section, the fallback BPM doesn't matter
    TempoMap sectionMap;
    sectionMap.addSection(TempoSection(0, 10000, 120));
    TempoMap::BeatCursor inSection(0);
    QCOMPARE(moveTo(sectionMap, inSection, 1000, 60), 2.0);
    QCOMPARE(moveTo(sectionMap, inSection, 2000, 200), 4.0);
}

static TempoMap complexMap()
{
    // a section starting after 0, adjacent sections with an off grid
    // boundary, a gap, and a last section ending before the run does
    TempoMap map;
    map.addSection(TempoSection(1300, 4000, 128));
    map.addSection(TempoSection(5300, 3000, 97.5));
    map.addSection(TempoSection(9000, 2500, 174));
    return map;
}

void TempoMap_Test::beatCursorSeek()
{
    TempoMap map = complexMap();

    // jumping straight to a time counts the same beats as getting there
    // tick by tick, from any start
    for (int origin = 0; origin <= 6000; origin += 1500)
    {
        TempoMap::BeatCursor ticked(origin);
        for (int t = origin + 20; t <= 15000; t += 20)
        {
            double beats = moveTo(map, ticked, t, 110);
            if (t % 1000 == 0)
            {
                TempoMap::BeatCursor seek(origin);
                QVERIFY(qAbs(moveTo(map, seek, t, 110) - beats) < 1e-6);
            }
        }
    }
}

void TempoMap_Test::beatCursorSmooth()
{
    TempoMap map = complexMap();

    // the count never jumps: always forward, within half and 1.5 times
    // the tempo of where it is
    for (int origin = 0; origin <= 6000; origin += 700)
    {
        TempoMap::BeatCursor cursor(origin);
        double previous = 0;
        for (int t = origin + 20; t <= 15000; t += 20)
        {
            double beats = moveTo(map, cursor, t, 110);
            double tempoBeats = 20.0 / map.beatDurationAt(t - 20, 110);
            double tempoBeatsEnd = 20.0 / map.beatDurationAt(t, 110);
            double step = beats - previous;
            QVERIFY(step >= 0.5 * qMin(tempoBeats, tempoBeatsEnd) - 1e-9);
            QVERIFY(step <= 1.5 * qMax(tempoBeats, tempoBeatsEnd) + 1e-9);
            previous = beats;
        }
    }
}

QTEST_APPLESS_MAIN(TempoMap_Test)
