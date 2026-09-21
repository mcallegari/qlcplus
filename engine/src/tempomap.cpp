/*
  Q Light Controller Plus
  tempomap.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <cmath>

#include "tempomap.h"

#define KXMLQLCTempoSection     QStringLiteral("Section")
#define KXMLQLCTempoStart       QStringLiteral("Start")
#define KXMLQLCTempoDuration    QStringLiteral("Duration")
#define KXMLQLCTempoBPM         QStringLiteral("BPM")
#define KXMLQLCTempoBeatsPerBar QStringLiteral("BeatsPerBar")
#define KXMLQLCTempoName        QStringLiteral("Name")

/* Beat positions closer than this (in ms) to a grid point are considered
   to be on the grid. It absorbs the rounding of item start times to whole
   milliseconds */
#define GRID_TOLERANCE_MS   1.0

/****************************************************************************
 * TempoSection
 ****************************************************************************/

TempoSection::TempoSection(quint32 start, quint32 length, double beatsPerMinute,
                           int barBeats, const QString &sectionName)
    : startTime(start)
    , duration(length)
    , bpm(beatsPerMinute)
    , beatsPerBar(barBeats)
    , name(sectionName)
{
}

bool TempoSection::operator==(const TempoSection &other) const
{
    return startTime == other.startTime && duration == other.duration &&
           bpm == other.bpm && beatsPerBar == other.beatsPerBar && name == other.name;
}

double TempoSection::beatDuration() const
{
    return 60000.0 / bpm;
}

quint32 TempoSection::endTime() const
{
    return startTime + duration;
}

/****************************************************************************
 * Sections
 ****************************************************************************/

TempoMap::TempoMap()
{
}

bool TempoMap::isEmpty() const
{
    return m_sections.isEmpty();
}

int TempoMap::count() const
{
    return m_sections.count();
}

const QList<TempoSection> &TempoMap::sections() const
{
    return m_sections;
}

TempoSection TempoMap::section(int index) const
{
    if (index < 0 || index >= m_sections.count())
        return TempoSection();

    return m_sections.at(index);
}

bool TempoMap::isValid(const TempoSection &section)
{
    return section.duration > 0 && section.bpm > 0 && section.beatsPerBar > 0;
}

bool TempoMap::canPlace(const TempoSection &section, int ignoreIndex) const
{
    if (isValid(section) == false)
        return false;

    for (int i = 0; i < m_sections.count(); i++)
    {
        if (i == ignoreIndex)
            continue;

        const TempoSection &other = m_sections.at(i);
        // sections are allowed to touch, but not to overlap
        if (section.startTime < other.endTime() && other.startTime < section.endTime())
            return false;
    }

    return true;
}

int TempoMap::addSection(const TempoSection &section)
{
    if (canPlace(section) == false)
        return -1;

    int index = 0;
    while (index < m_sections.count() && m_sections.at(index).startTime < section.startTime)
        index++;

    m_sections.insert(index, section);
    return index;
}

bool TempoMap::updateSection(int index, const TempoSection &section)
{
    if (index < 0 || index >= m_sections.count())
        return false;

    if (canPlace(section, index) == false)
        return false;

    m_sections.removeAt(index);
    addSection(section);
    return true;
}

bool TempoMap::removeSection(int index)
{
    if (index < 0 || index >= m_sections.count())
        return false;

    m_sections.removeAt(index);
    return true;
}

bool TempoMap::splitSection(int index, quint32 time)
{
    if (index < 0 || index >= m_sections.count())
        return false;

    TempoSection first = m_sections.at(index);
    if (time <= first.startTime || time >= first.endTime())
        return false;

    TempoSection second = first;
    second.startTime = time;
    second.duration = first.endTime() - time;
    first.duration = time - first.startTime;

    m_sections.replace(index, first);
    m_sections.insert(index + 1, second);
    return true;
}

void TempoMap::clear()
{
    m_sections.clear();
}

int TempoMap::sectionIndexAt(double time) const
{
    for (int i = 0; i < m_sections.count(); i++)
    {
        const TempoSection &section = m_sections.at(i);
        if (time >= section.startTime && time < section.endTime())
            return i;
    }

    return -1;
}

/****************************************************************************
 * Beat clock
 ****************************************************************************/

TempoMap::Segment TempoMap::segmentAt(double time, double fallbackBpm) const
{
    Segment segment;
    segment.origin = 0;
    segment.bpm = fallbackBpm > 0 ? fallbackBpm : 120.0;
    segment.end = m_sections.isEmpty() ? -1 : m_sections.first().startTime;

    for (int i = 0; i < m_sections.count(); i++)
    {
        const TempoSection &section = m_sections.at(i);
        if (time < section.startTime)
            break;

        segment.origin = section.startTime;
        segment.bpm = section.bpm;
        segment.end = i + 1 < m_sections.count() ? m_sections.at(i + 1).startTime : -1;
    }

    return segment;
}

double TempoMap::beatDurationAt(double time, double fallbackBpm) const
{
    return 60000.0 / segmentAt(time, fallbackBpm).bpm;
}

double TempoMap::stepEnd(double startTime, double beats, double fallbackBpm) const
{
    if (beats <= 0)
        return startTime;

    // the grid a step locks to: whole beats, or the step itself when it is
    // shorter than a beat, so that e.g. half beat steps land on half beats
    double grid = beats < 1.0 ? beats : 1.0;

    Segment segment = segmentAt(startTime, fallbackBpm);
    double beatMs = 60000.0 / segment.bpm;
    double position = (startTime - segment.origin) / beatMs;
    double target = position + beats;

    double gridPosition = std::round(position / grid) * grid;
    if (std::fabs(gridPosition - position) * beatMs > GRID_TOLERANCE_MS)
    {
        // off the grid: end on the grid point nearest to the requested end.
        // This keeps the step within half a grid unit of its length
        target = std::round(target / grid) * grid;
    }
    else
    {
        // on the grid: drop the ms rounding of the start time
        target = gridPosition + beats;
    }

    while (true)
    {
        double end = segment.origin + target * beatMs;

        if (segment.end < 0 || end <= segment.end)
            return end;

        // the step runs into the next section: continue its remaining beats
        // at the new tempo, ending on the new section grid
        double remaining = target - ((segment.end - segment.origin) / beatMs);

        segment = segmentAt(segment.end, fallbackBpm);
        beatMs = 60000.0 / segment.bpm;
        target = std::round(remaining / grid) * grid;
    }
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool TempoMap::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    if (m_sections.isEmpty())
        return true;

    doc->writeStartElement(KXMLQLCTempoMap);

    foreach (TempoSection section, m_sections)
    {
        doc->writeStartElement(KXMLQLCTempoSection);
        doc->writeAttribute(KXMLQLCTempoStart, QString::number(section.startTime));
        doc->writeAttribute(KXMLQLCTempoDuration, QString::number(section.duration));
        doc->writeAttribute(KXMLQLCTempoBPM, QString::number(section.bpm));
        doc->writeAttribute(KXMLQLCTempoBeatsPerBar, QString::number(section.beatsPerBar));
        if (section.name.isEmpty() == false)
            doc->writeAttribute(KXMLQLCTempoName, section.name);
        doc->writeEndElement();
    }

    doc->writeEndElement();

    return true;
}

bool TempoMap::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCTempoMap)
    {
        qWarning() << Q_FUNC_INFO << "Tempo map node not found";
        return false;
    }

    m_sections.clear();

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCTempoSection)
        {
            QXmlStreamAttributes attrs = root.attributes();
            TempoSection section(attrs.value(KXMLQLCTempoStart).toString().toUInt(),
                                 attrs.value(KXMLQLCTempoDuration).toString().toUInt(),
                                 attrs.value(KXMLQLCTempoBPM).toString().toDouble(),
                                 attrs.value(KXMLQLCTempoBeatsPerBar).toString().toInt(),
                                 attrs.value(KXMLQLCTempoName).toString());

            if (addSection(section) == -1)
                qWarning() << Q_FUNC_INFO << "Invalid or overlapping tempo section at" << section.startTime;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown tempo map tag:" << root.name();
        }
        root.skipCurrentElement();
    }

    return true;
}

/****************************************************************************
 * TempoMapClock
 ****************************************************************************/

TempoMapClock::TempoMapClock(const TempoMap &tempoMap, quint32 originTime)
    : map(tempoMap)
    , origin(originTime)
{
}
