/*
  Q Light Controller Plus
  tempomap.h

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

#ifndef TEMPOMAP_H
#define TEMPOMAP_H

#include <QString>
#include <QList>

class QXmlStreamReader;
class QXmlStreamWriter;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCTempoMap             QStringLiteral("TempoMap")
#define KXMLQLCTempoMapItemUnit     QStringLiteral("ItemUnit")
#define KXMLQLCTempoMapItemUnitMs   QStringLiteral("ms")

/**
 * A stretch of a Show timeline with its own tempo. The section beat grid
 * starts at startTime (bar 1, beat 1).
 */
class TempoSection
{
public:
    TempoSection(quint32 start = 0, quint32 length = 0, double beatsPerMinute = 120.0,
                 int barBeats = 4, const QString &sectionName = QString());

    bool operator==(const TempoSection &other) const;

    /** Duration of a single beat in milliseconds */
    double beatDuration() const;

    /** The time where this section ends */
    quint32 endTime() const;

    quint32 startTime;
    quint32 duration;
    double bpm;
    int beatsPerBar;
    QString name;
};

/**
 * A list of non overlapping tempo sections of a Show, with the maths
 * needed to run beat based Functions on the sections beat grids.
 *
 * Timing is split into segments:
 * - before the first section: the fallback BPM, with a grid starting at 0
 * - from the start of each section up to the start of the next one: the
 *   section tempo and grid. The gap after a section keeps the section tempo,
 *   so a Function running past the end of a section doesn't change its pace
 *   in the middle of a step.
 */
class TempoMap
{
public:
    TempoMap();

    /*********************************************************************
     * Sections
     *********************************************************************/
public:
    bool isEmpty() const;
    int count() const;

    /** Get all the sections, sorted by start time */
    const QList<TempoSection> &sections() const;

    /** Get the section at the given index */
    TempoSection section(int index) const;

    /** Check if $section can be placed without overlapping any other section,
     *  ignoring the section at $ignoreIndex. */
    bool canPlace(const TempoSection &section, int ignoreIndex = -1) const;

    /** Add a section. Returns its index, or -1 if it is invalid or overlaps */
    int addSection(const TempoSection &section);

    /** Replace the section at $index. Returns false if the result is invalid
     *  or overlaps another section */
    bool updateSection(int index, const TempoSection &section);

    /** Remove the section at $index */
    bool removeSection(int index);

    /** Split the section at $index in two at $time, which must fall strictly
     *  inside the section. Both parts keep the original tempo */
    bool splitSection(int index, quint32 time);

    void clear();

    /** Get the index of the section containing $time, or -1 if $time is not
     *  within any section */
    int sectionIndexAt(double time) const;

private:
    static bool isValid(const TempoSection &section);

private:
    QList<TempoSection> m_sections;

    /*********************************************************************
     * Beat clock
     *********************************************************************/
public:
    /** Get the duration in ms of a beat at $time, using $fallbackBpm before
     *  the first section */
    double beatDurationAt(double time, double fallbackBpm) const;

    /**
     * Get the time (in ms) when a step of $beats beats, started at
     * $startTime, ends.
     *
     * The step is locked to the beat grid: if $startTime is not on the grid
     * (whole beats, or multiples of $beats for steps shorter than a beat),
     * the end is moved to the nearest grid point. When a step runs into a new
     * section, the rest of it continues at the new section tempo and ends on
     * the new section grid.
     */
    double stepEnd(double startTime, double beats, double fallbackBpm) const;

private:
    struct Segment
    {
        double origin;  // time of beat 0 of the segment grid
        double bpm;
        double end;     // time where the next segment starts, or -1
    };

    Segment segmentAt(double time, double fallbackBpm) const;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Save the tempo map. The element is written if there are sections,
     *  or if $itemsInMs is true, to record that the items of the Show are
     *  positioned in ms (see Show::itemsInMs()) */
    bool saveXML(QXmlStreamWriter *doc, bool itemsInMs = false) const;
    bool loadXML(QXmlStreamReader &root);
};

/**
 * A copy of a Show tempo map handed to a Function started by the Show,
 * together with the Show time the Function was started at.
 */
class TempoMapClock
{
public:
    TempoMapClock(const TempoMap &tempoMap = TempoMap(), quint32 originTime = 0);

    TempoMap map;
    /** The Show time matching the Function elapsed time 0 */
    quint32 origin;
};

/** @} */

#endif
