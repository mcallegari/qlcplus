/*
  Q Light Controller Plus
  tempoanalyzer.h

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

#ifndef TEMPOANALYZER_H
#define TEMPOANALYZER_H

#include <vector>

#include "beattracker.h"

/** @addtogroup engine_functions Functions
 * @{
 */

/**
 * Offline tempo estimate of a whole piece of audio, built on the live
 * BeatTracker stages. The onset front end and the tempo estimator run over
 * the audio as they would live, and the estimator reading is sampled every
 * 2 s. The tempo is the reading most of the audio agrees on (within 2%),
 * provided that is at least a quarter of it, refined over the whole audio:
 * the estimator works on a 0.25 BPM grid over 8 s, while a Show beat grid
 * drifts a whole beat over 4 minutes at 128 BPM with a 0.25 BPM error. The
 * refinement maximises the onset autocorrelation summed over up to 64 beat
 * multiples, within 1% of the reading, in 0.01 BPM steps.
 */
class TempoAnalyzer final
{
public:
    explicit TempoAnalyzer(int sampleRate);

    /** Feed mono samples in [-1, 1] */
    void push(const float *samples, int count);

    /** Seconds of audio fed so far */
    double seconds() const;

    struct Result
    {
        /** The tempo, or 0 when none was found */
        double bpm;
        /** The share (0..1) of the audio whose tempo readings agree with bpm */
        double agreement;
    };

    Result result() const;

private:
    double refine(double bpm) const;

private:
    BeatOnsetExtractor m_extractor;
    AutoBpmDetector m_detector;
    std::vector<double> m_onsets;
    std::vector<double> m_newOnsets;
    long long m_readingInterval;
    long long m_warmupFrames;
    std::vector<double> m_readingBpms;
    std::vector<double> m_readingConfidences;
};

/** @} */

#endif // TEMPOANALYZER_H
