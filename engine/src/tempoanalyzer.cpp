/*
  Q Light Controller Plus
  tempoanalyzer.cpp

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

#include <algorithm>
#include <cmath>

#include "tempoanalyzer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define READING_SECONDS     2.0
#define WARMUP_SECONDS      8.0
#define AGREEMENT_TOLERANCE 0.02
#define MIN_READINGS        3
#define MIN_AGREEMENT       0.25
#define REFINE_SPAN         0.01
#define REFINE_STEP         0.01
#define REFINE_MAX_BEATS    64
#define REFINE_MIN_BEATS    8

TempoAnalyzer::TempoAnalyzer(int sampleRate)
    : m_extractor(sampleRate > 0 ? sampleRate : 44100)
    , m_detector(m_extractor.frameRateHz())
{
    m_readingInterval = std::max(1LL, (long long)std::llround(READING_SECONDS * m_extractor.frameRateHz()));
    m_warmupFrames = (long long)std::llround(WARMUP_SECONDS * m_extractor.frameRateHz());
}

void TempoAnalyzer::push(const float *samples, int count)
{
    m_newOnsets.clear();
    m_extractor.push(samples, count, m_newOnsets);

    for (double value : m_newOnsets)
    {
        m_onsets.push_back(value);
        m_detector.pushOnset(value);

        long long frames = m_detector.frameCount();
        if (frames >= m_warmupFrames && frames % m_readingInterval == 0)
        {
            m_readingBpms.push_back(m_detector.bpm());
            m_readingConfidences.push_back(m_detector.confidence());
        }
    }
}

double TempoAnalyzer::seconds() const
{
    return double(m_onsets.size()) / m_extractor.frameRateHz();
}

TempoAnalyzer::Result TempoAnalyzer::result() const
{
    Result res = { 0.0, 0.0 };

    // the readings of silent or aperiodic stretches are 0
    std::vector<size_t> valid;
    for (size_t i = 0; i < m_readingBpms.size(); i++)
        if (m_readingBpms[i] > 0.0)
            valid.push_back(i);

    if (valid.empty())
        return res;

    // the reading whose neighbourhood weighs most, by confidence
    size_t best = valid.front();
    double bestWeight = -1.0;
    for (size_t i : valid)
    {
        double weight = 0.0;
        for (size_t j : valid)
            if (std::fabs(m_readingBpms[j] - m_readingBpms[i]) <= AGREEMENT_TOLERANCE * m_readingBpms[i])
                weight += m_readingConfidences[j];
        if (weight > bestWeight)
        {
            bestWeight = weight;
            best = i;
        }
    }

    std::vector<double> cluster;
    for (size_t j : valid)
        if (std::fabs(m_readingBpms[j] - m_readingBpms[best]) <= AGREEMENT_TOLERANCE * m_readingBpms[best])
            cluster.push_back(m_readingBpms[j]);

    // a tempo heard through only a little of the audio is noise
    double agreement = double(cluster.size()) / double(m_readingBpms.size());
    if (cluster.size() < MIN_READINGS || agreement < MIN_AGREEMENT)
        return res;

    std::sort(cluster.begin(), cluster.end());
    double median = cluster[cluster.size() / 2];

    res.bpm = refine(median);
    res.agreement = agreement;
    return res;
}

double TempoAnalyzer::refine(double bpm) const
{
    const double rate = m_extractor.frameRateHz();
    const int n = int(m_onsets.size());
    const double period = 60.0 * rate / bpm;
    const int beats = std::min(REFINE_MAX_BEATS, int(0.5 * n / period));

    if (beats < REFINE_MIN_BEATS)
        return bpm;

    // DC removed and smoothed with the ~+/-23 ms Hann kernel of the estimator
    double mean = 0.0;
    for (double v : m_onsets)
        mean += v;
    mean /= n;

    const int half = std::max(1, int(std::lround(0.023 * rate)));
    std::vector<double> kernel(2 * half + 1);
    double kernelSum = 0.0;
    for (int k = 0; k <= 2 * half; k++)
    {
        kernel[k] = 0.5 - 0.5 * std::cos(2.0 * M_PI * (k + 1) / (2 * half + 2));
        kernelSum += kernel[k];
    }

    std::vector<double> signal(n, 0.0);
    for (int i = 0; i < n; i++)
    {
        double acc = 0.0;
        for (int k = -half; k <= half; k++)
        {
            int idx = i + k;
            if (idx >= 0 && idx < n)
                acc += (m_onsets[idx] - mean) * kernel[k + half];
        }
        signal[i] = acc / kernelSum;
    }

    const int maxLag = std::min(n - 1, int(std::ceil(beats * period * (1.0 + REFINE_SPAN))) + 2);
    std::vector<double> acf(maxLag + 1, 0.0);
    for (int lag = 1; lag <= maxLag; lag++)
    {
        double sum = 0.0;
        for (int i = 0; i + lag < n; i++)
            sum += signal[i] * signal[i + lag];
        acf[lag] = sum / double(n - lag);
    }

    auto acfAt = [&acf, maxLag](double lag)
    {
        int i = int(lag);
        if (i < 0 || i + 1 > maxLag)
            return 0.0;
        double frac = lag - i;
        return acf[i] * (1.0 - frac) + acf[i + 1] * frac;
    };

    double bestBpm = bpm;
    double bestScore = -1e300;
    const int steps = int(std::lround(bpm * REFINE_SPAN / REFINE_STEP));
    for (int s = -steps; s <= steps; s++)
    {
        double candidate = bpm + s * REFINE_STEP;
        double candidatePeriod = 60.0 * rate / candidate;
        double score = 0.0;
        for (int m = 1; m <= beats; m++)
            score += acfAt(m * candidatePeriod);
        if (score > bestScore)
        {
            bestScore = score;
            bestBpm = candidate;
        }
    }

    return bestBpm;
}
