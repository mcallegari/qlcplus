/*
  Q Light Controller Plus
  beattrackerbench.cpp

  Copyright (c) varghele

  Head-to-head benchmark of the beat tracker implementations on
  synthesized percussion audio with known ground-truth tempo.
  C++ port of varghele's Python benchmark harness (the source of the
  numbers quoted in issue #1881).

  Usage:
    beattrackerbench            quick mode: 4 scenarios x 4 tempi,
                                asserts the default tracker is correct
                                on every clip (used by test.sh)
    beattrackerbench --full     8 scenarios x 8 tempi comparison table

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

#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

#include "beattrackerauto.h"
#include "beattracker.h"
#include "beattracking.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define BLOCK_FRAMES 2048          // AudioCapture block size in frames
#define CLIP_SECONDS 24.0
#define TOLERANCE 0.04             // 4% relative: "correct"

typedef std::vector<double> Wave;
typedef std::mt19937 Rng;

/*****************************************************************************
 * Percussion synthesis (kick, snare, hats, bass over a sustained pad)
 *****************************************************************************/

static Wave makeKick()
{
    const int n = int(0.09 * SAMPLE_RATE);
    Wave w(n);
    double phase = 0.0;
    for (int i = 0; i < n; i++)
    {
        double t = double(i) / SAMPLE_RATE;
        double freq = 150.0 * std::exp(-t * 25.0) + 45.0;
        phase += 2.0 * M_PI * freq / SAMPLE_RATE;
        w[i] = std::sin(phase) * std::exp(-t * 30.0) * 0.9;
    }
    return w;
}

static Wave makeSnare(Rng &rng)
{
    const int n = int(0.12 * SAMPLE_RATE);
    std::normal_distribution<double> gauss(0.0, 1.0);
    Wave w(n);
    for (int i = 0; i < n; i++)
    {
        double t = double(i) / SAMPLE_RATE;
        double body = std::sin(2.0 * M_PI * 190.0 * t) * std::exp(-t * 40.0) * 0.4;
        double rattle = gauss(rng) * std::exp(-t * 35.0) * 0.25;
        w[i] = body + rattle;
    }
    return w;
}

static Wave makeHat(Rng &rng)
{
    const int n = int(0.03 * SAMPLE_RATE);
    std::normal_distribution<double> gauss(0.0, 1.0);
    Wave x(n);
    for (int i = 0; i < n; i++)
        x[i] = gauss(rng) * std::exp(-double(i) / (0.005 * SAMPLE_RATE));
    Wave w(n);
    double prev = 0.0;
    for (int i = 0; i < n; i++)
    {
        w[i] = (x[i] - prev) * 0.20; // crude high-pass
        prev = x[i];
    }
    return w;
}

static Wave makeBassNote()
{
    const int n = int(0.18 * SAMPLE_RATE);
    Wave w(n);
    for (int i = 0; i < n; i++)
    {
        double t = double(i) / SAMPLE_RATE;
        double env = std::fmin(t / 0.005, 1.0) * std::exp(-t * 12.0);
        w[i] = std::sin(2.0 * M_PI * 85.0 * t) * env * 0.45;
    }
    return w;
}

struct Event
{
    double time;
    const Wave *sample;
};

/** Beat times for a (possibly time-varying) tempo function */
template <typename TempoFn>
static std::vector<double> beatTimes(TempoFn bpmAt, double seconds)
{
    std::vector<double> times;
    double t = 0.0;
    while (t < seconds)
    {
        times.push_back(t);
        t += 60.0 / bpmAt(t);
    }
    return times;
}

/** Render events over a sustained pad + noise floor, int16 round trip */
static std::vector<int16_t> render(const std::vector<Event> &events,
                                   double seconds, Rng &rng,
                                   double gapStart = -1.0, double gapEnd = -1.0)
{
    const int n = int(seconds * SAMPLE_RATE);
    std::normal_distribution<double> gauss(0.0, 1.0);
    Wave audio(n);
    for (int i = 0; i < n; i++)
    {
        double t = double(i) / SAMPLE_RATE;
        audio[i] = 0.05 * std::sin(2.0 * M_PI * 110.0 * t)
                 + 0.02 * std::sin(2.0 * M_PI * 220.0 * t)
                 + 0.015 * std::sin(2.0 * M_PI * 330.5 * t)
                 + gauss(rng) * 0.002;
    }
    for (size_t e = 0; e < events.size(); e++)
    {
        int start = int(events[e].time * SAMPLE_RATE);
        const Wave &s = *events[e].sample;
        for (size_t i = 0; i < s.size() && start + int(i) < n; i++)
            if (start + int(i) >= 0)
                audio[start + i] += s[i];
    }
    if (gapStart >= 0.0)
        for (int i = int(gapStart * SAMPLE_RATE);
             i < int(gapEnd * SAMPLE_RATE) && i < n; i++)
            audio[i] = 0.0;

    std::vector<int16_t> pcm(n);
    for (int i = 0; i < n; i++)
    {
        double v = audio[i];
        if (v > 1.0) v = 1.0;
        if (v < -1.0) v = -1.0;
        pcm[i] = int16_t(v * 32767.0);
    }
    return pcm;
}

/*****************************************************************************
 * Scenarios (mirror varghele's Python benchmark suite)
 *****************************************************************************/

struct Clip
{
    std::vector<int16_t> pcm;
    double truth;
};

static Clip scenarioClip(const std::string &name, double bpm, Rng &rng)
{
    static const Wave kick = makeKick();
    static const Wave bass = makeBassNote();
    Wave snare = makeSnare(rng);
    Wave hat = makeHat(rng);
    double seconds = CLIP_SECONDS;
    std::vector<Event> ev;

    if (name == "kick four-on-floor")
    {
        std::vector<double> beats = beatTimes([&](double){ return bpm; }, seconds);
        for (size_t i = 0; i < beats.size(); i++)
            ev.push_back(Event{ beats[i], &kick });
    }
    else if (name == "kick + 8th hats")
    {
        std::vector<double> beats = beatTimes([&](double){ return bpm; }, seconds);
        for (size_t i = 0; i < beats.size(); i++)
        {
            ev.push_back(Event{ beats[i], &kick });
            ev.push_back(Event{ beats[i] + 30.0 / bpm, &hat });
        }
    }
    else if (name == "kick/snare backbeat")
    {
        std::vector<double> beats = beatTimes([&](double){ return bpm; }, seconds);
        for (size_t i = 0; i < beats.size(); i++)
            ev.push_back(Event{ beats[i], (i % 2 == 0) ? &kick : &snare });
    }
    else if (name == "8th-note bassline")
    {
        std::vector<double> beats = beatTimes([&](double){ return bpm; }, seconds);
        for (size_t i = 0; i < beats.size(); i++)
        {
            ev.push_back(Event{ beats[i], &kick });
            ev.push_back(Event{ beats[i] + 30.0 / bpm, &bass });
        }
    }
    else if (name == "swung hats")
    {
        std::vector<double> beats = beatTimes([&](double){ return bpm; }, seconds);
        for (size_t i = 0; i < beats.size(); i++)
        {
            ev.push_back(Event{ beats[i], &kick });
            ev.push_back(Event{ beats[i] + 40.0 / bpm, &hat });
        }
    }
    else if (name == "tempo drift +/-3%")
    {
        std::vector<double> beats = beatTimes(
            [&](double t){ return bpm * (0.97 + 0.06 * t / CLIP_SECONDS); }, seconds);
        for (size_t i = 0; i < beats.size(); i++)
            ev.push_back(Event{ beats[i], &kick });
    }
    else if (name == "2.5 s dropout")
    {
        std::vector<double> beats = beatTimes([&](double){ return bpm; }, seconds);
        for (size_t i = 0; i < beats.size(); i++)
            ev.push_back(Event{ beats[i], &kick });
        Clip clip;
        clip.pcm = render(ev, seconds, rng, 10.0, 12.5);
        clip.truth = bpm;
        return clip;
    }
    else if (name == "tempo step +30%")
    {
        seconds = 28.0;
        std::vector<double> beats = beatTimes(
            [&](double t){ return (t < 12.0) ? bpm / 1.3 : bpm; }, seconds);
        for (size_t i = 0; i < beats.size(); i++)
            ev.push_back(Event{ beats[i], &kick });
    }

    Clip clip;
    clip.pcm = render(ev, seconds, rng);
    clip.truth = bpm;
    return clip;
}

/*****************************************************************************
 * Tracker adapters and scoring
 *****************************************************************************/

/** Feed a mono clip in AudioCapture-sized blocks; return the final BPM */
template <typename ProcessFn, typename BpmFn>
static double runClip(const Clip &clip, ProcessFn process, BpmFn bpm)
{
    for (size_t s = 0; s + BLOCK_FRAMES <= clip.pcm.size(); s += BLOCK_FRAMES)
        process(const_cast<int16_t *>(clip.pcm.data() + s), BLOCK_FRAMES);
    return bpm();
}

enum Verdict { Correct, Octave, Wrong, None };

static Verdict classify(double est, double truth)
{
    if (est <= 0.0)
        return None;
    if (std::fabs(est - truth) / truth <= TOLERANCE)
        return Correct;
    static const double octaves[] = { 2.0, 0.5, 3.0, 1.0 / 3.0, 1.5, 2.0 / 3.0 };
    for (size_t i = 0; i < sizeof(octaves) / sizeof(octaves[0]); i++)
        if (std::fabs(est - truth * octaves[i]) / (truth * octaves[i]) <= TOLERANCE)
            return Octave;
    return Wrong;
}

static const char *verdictName(Verdict v)
{
    switch (v)
    {
        case Correct: return "correct";
        case Octave: return "octave";
        case Wrong: return "wrong";
        default: return "none";
    }
}

int main(int argc, char **argv)
{
    const bool full = (argc > 1 && std::strcmp(argv[1], "--full") == 0);

    static const char *allScenarios[] = {
        "kick four-on-floor", "kick + 8th hats", "kick/snare backbeat",
        "8th-note bassline", "swung hats", "tempo drift +/-3%",
        "2.5 s dropout", "tempo step +30%"
    };
    static const double allTempi[] = { 60, 75, 90, 105, 120, 140, 160, 180 };
    const int nScenarios = full ? 8 : 4;
    const int nTempi = full ? 8 : 4;
    static const double quickTempi[] = { 75, 105, 140, 174 };
    const double *tempi = full ? allTempi : quickTempi;

    static const char *trackerNames[] = {
        "BeatTrackerAuto (default)", "BeatTracker (spectral flux)",
        "BeatTracking (ACF)"
    };
    int correct[3] = { 0, 0, 0 };
    int perScenario[3][8];
    std::memset(perScenario, 0, sizeof(perScenario));
    int defaultTrackerFailures = 0;

    std::printf("Beat tracker comparison: %d scenarios x %d tempi, "
                "%.0f+ s clips, tolerance %.0f%%\n\n",
                nScenarios, nTempi, CLIP_SECONDS, TOLERANCE * 100.0);

    Rng rng(1234);
    for (int s = 0; s < nScenarios; s++)
    {
        for (int t = 0; t < nTempi; t++)
        {
            Clip clip = scenarioClip(allScenarios[s], tempi[t], rng);
            double est[3];
            Verdict verdict[3];

            BeatTrackerAuto autoTracker(SAMPLE_RATE, 1);
            est[0] = runClip(clip,
                [&](int16_t *b, int n){ autoTracker.processAudio(b, n); },
                [&](){ return autoTracker.bpm(); });

            BeatTracker flux(SAMPLE_RATE, BLOCK_FRAMES, 1, 86, 1.3);
            flux.setBand(40.0, 400.0);
            flux.setFluxSmoothing(0.6);
            flux.setMinBeatInterval(0.20);
            est[1] = runClip(clip,
                [&](int16_t *b, int n){ flux.processAudio(b, n); },
                [&](){ return flux.getCurrentBpm(); });

            BeatTracking acf(SAMPLE_RATE, 1);
            est[2] = runClip(clip,
                [&](int16_t *b, int n){ acf.processAudio(b, n); },
                [&](){ return acf.currentBpm(); });

            for (int i = 0; i < 3; i++)
            {
                verdict[i] = classify(est[i], clip.truth);
                if (verdict[i] == Correct)
                {
                    correct[i]++;
                    perScenario[i][s]++;
                }
            }
            if (verdict[0] != Correct)
                defaultTrackerFailures++;

            std::printf("%-20s %3.0f BPM:  auto %7.2f (%s)  flux %7.2f (%s)"
                        "  acf %7.2f (%s)\n",
                        allScenarios[s], clip.truth,
                        est[0], verdictName(verdict[0]),
                        est[1], verdictName(verdict[1]),
                        est[2], verdictName(verdict[2]));
        }
    }

    const int total = nScenarios * nTempi;
    std::printf("\n%-30s %s\n", "tracker", "correct");
    for (int i = 0; i < 3; i++)
    {
        std::printf("%-30s %d/%d   per scenario:", trackerNames[i],
                    correct[i], total);
        for (int s = 0; s < nScenarios; s++)
            std::printf(" %d", perScenario[i][s]);
        std::printf("\n");
    }

    if (!full && defaultTrackerFailures > 0)
    {
        std::printf("\nFAIL: default tracker missed %d/%d quick clips\n",
                    defaultTrackerFailures, total);
        return 1;
    }
    return 0;
}
