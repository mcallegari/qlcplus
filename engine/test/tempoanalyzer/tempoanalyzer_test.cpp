/*
  Q Light Controller Plus
  tempoanalyzer_test.cpp

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
#include <cmath>
#include <vector>

#include "tempoanalyzer.h"
#include "tempoanalyzer_test.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define BLOCK_FRAMES 4096

/** Mix a synthetic kick drum (150->45 Hz sweep) into the buffer */
static void addKick(std::vector<float> &buffer, double time)
{
    const int start = int(time * SAMPLE_RATE);
    const int length = int(0.09 * SAMPLE_RATE);
    double phase = 0.0;
    for (int i = 0; i < length && start + i < int(buffer.size()); i++)
    {
        double t = double(i) / SAMPLE_RATE;
        phase += 2.0 * M_PI * (150.0 * std::exp(-t * 25.0) + 45.0) / SAMPLE_RATE;
        buffer[start + i] += float(std::sin(phase) * std::exp(-t * 30.0) * 0.6);
    }
}

/** A kick on every beat from $beatStart, except between $breakStart and $breakEnd */
static std::vector<float> makeKickTrack(double bpm, double seconds, double beatStart = 0.0,
                                        double breakStart = -1.0, double breakEnd = -1.0)
{
    std::vector<float> buffer(size_t(seconds * SAMPLE_RATE), 0.0f);
    for (double beat = beatStart; beat < seconds; beat += 60.0 / bpm)
        if (beat < breakStart || beat >= breakEnd)
            addKick(buffer, beat);
    return buffer;
}

static TempoAnalyzer::Result analyze(const std::vector<float> &audio)
{
    TempoAnalyzer analyzer(SAMPLE_RATE);
    for (size_t pos = 0; pos < audio.size(); pos += BLOCK_FRAMES)
        analyzer.push(audio.data() + pos, int(qMin(size_t(BLOCK_FRAMES), audio.size() - pos)));
    return analyzer.result();
}

void TempoAnalyzer_Test::refinesOffGridTempo_data()
{
    QTest::addColumn<double>("bpm");

    // the live estimator works on a 0.25 BPM grid
    QTest::newRow("123.37") << 123.37;
    QTest::newRow("96.5") << 96.5;
    QTest::newRow("174.2") << 174.2;
    QTest::newRow("71.3") << 71.3;
}

void TempoAnalyzer_Test::refinesOffGridTempo()
{
    QFETCH(double, bpm);

    TempoAnalyzer::Result result = analyze(makeKickTrack(bpm, 60.0));

    // 0.02 BPM drifts less than a beat over 45 minutes
    QVERIFY2(std::fabs(result.bpm - bpm) <= 0.02, qPrintable(QString::number(result.bpm)));
    QVERIFY(result.agreement > 0.9);
}

void TempoAnalyzer_Test::findsTempoAcrossIntroAndBreak()
{
    TempoAnalyzer::Result result = analyze(makeKickTrack(140.0, 80.0, 10.0, 35.0, 50.0));

    QVERIFY2(std::fabs(result.bpm - 140.0) <= 0.02, qPrintable(QString::number(result.bpm)));
}

void TempoAnalyzer_Test::silenceGivesNoTempo()
{
    TempoAnalyzer::Result result = analyze(std::vector<float>(SAMPLE_RATE * 30, 0.0f));

    QCOMPARE(result.bpm, 0.0);
    QCOMPARE(result.agreement, 0.0);
}

void TempoAnalyzer_Test::briefBeatGivesNoTempo()
{
    // a beat through a sixth of the audio
    TempoAnalyzer::Result result = analyze(makeKickTrack(120.0, 90.0, 0.0, 15.0, 90.0));

    QCOMPARE(result.bpm, 0.0);
}

void TempoAnalyzer_Test::shortAudioGivesNoTempo()
{
    // no reading before the estimator has warmed up
    TempoAnalyzer::Result result = analyze(makeKickTrack(120.0, 6.0));

    QCOMPARE(result.bpm, 0.0);
}

QTEST_GUILESS_MAIN(TempoAnalyzer_Test)
