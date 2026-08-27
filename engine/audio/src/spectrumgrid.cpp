/*
  Q Light Controller Plus
  spectrumgrid.cpp

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

#include <QString>
#include <QtGlobal>
#include <qmath.h>

#include "spectrumgrid.h"

namespace
{

double logSpan(double fLo, double fHi)
{
    if (fLo <= 0.0 || fHi <= fLo)
        return 0.0;
    return qLn(fHi / fLo);
}

QVector<double> buildLogUniformEdges(int bandCount, double fLow, double fHigh, double lowBandGamma)
{
    QVector<double> edges(bandCount + 1);
    const double logRange = logSpan(fLow, fHigh);
    if (bandCount <= 0 || logRange <= 0.0)
    {
        edges.fill(fLow);
        if (bandCount > 0)
            edges[bandCount] = fHigh;
        return edges;
    }

    const double gamma = qMax(1.0, lowBandGamma);
    for (int i = 0; i <= bandCount; ++i)
    {
        const double t = double(i) / double(bandCount);
        const double u = (gamma <= 1.0001) ? t : qPow(t, gamma);
        edges[i] = fLow * qExp(logRange * u);
    }

    edges[0] = fLow;
    edges[bandCount] = fHigh;
    return edges;
}

QVector<double> warpEdgesLowBandGamma(QVector<double> edges, double lowBandGamma)
{
    if (edges.size() < 2 || lowBandGamma <= 1.0001)
        return edges;

    const double fLow = edges.first();
    const double fHigh = edges.last();
    const double logRange = logSpan(fLow, fHigh);
    if (logRange <= 0.0)
        return edges;

    const double gamma = qMax(1.0, lowBandGamma);
    QVector<double> warped(edges.size());
    warped[0] = fLow;
    warped[warped.size() - 1] = fHigh;
    for (int i = 1; i < edges.size() - 1; ++i)
    {
        const double u = logSpan(fLow, edges[i]) / logRange;
        warped[i] = fLow * qExp(logRange * qPow(u, gamma));
    }
    return warped;
}

QVector<double> buildPreferredTickGrid(double fLow, double fHigh)
{
    QVector<double> ticks;
    ticks.reserve(64);
    ticks.append(fLow);

    if (fHigh <= fLow)
    {
        ticks.append(fHigh);
        return ticks;
    }

    const double startDecade = qPow(10.0, qFloor(qLn(qMax(fLow, 1.0)) / qLn(10.0)));
    double decade = startDecade;
    const int multipliers[] = { 1, 2, 5 };

    while (decade <= fHigh * 10.0)
    {
        for (int m : multipliers)
        {
            const double f = double(m) * decade;
            if (f > fLow && f < fHigh)
                ticks.append(f);
        }
        decade *= 10.0;
    }

    ticks.append(fHigh);

    std::sort(ticks.begin(), ticks.end());
    auto nearEqual = [](double a, double b) { return qAbs(a - b) < 1e-6; };
    const auto last = std::unique(ticks.begin(), ticks.end(), nearEqual);
    ticks.erase(last, ticks.end());
    return ticks;
}

QVector<QPair<double, double>> ticksToSegments(const QVector<double> &ticks)
{
    QVector<QPair<double, double>> segments;
    if (ticks.size() < 2)
        return segments;

    for (int i = 0; i < ticks.size() - 1; ++i)
    {
        const double lo = ticks[i];
        const double hi = ticks[i + 1];
        if (hi > lo)
            segments.append(qMakePair(lo, hi));
    }
    return segments;
}

void mergeNarrowestAdjacent(QVector<QPair<double, double>> &segments)
{
    if (segments.size() < 2)
        return;

    int bestIdx = 0;
    double bestSpan = logSpan(segments[0].first, segments[0].second);
    for (int i = 1; i < segments.size() - 1; ++i)
    {
        const double span = logSpan(segments[i].first, segments[i].second);
        if (span < bestSpan)
        {
            bestSpan = span;
            bestIdx = i;
        }
    }

    const double mergedHi = segments[bestIdx + 1].second;
    segments[bestIdx].second = mergedHi;
    segments.removeAt(bestIdx + 1);
}

void splitWidestSegment(QVector<QPair<double, double>> &segments)
{
    if (segments.isEmpty())
        return;

    int widestIdx = 0;
    double widestSpan = logSpan(segments[0].first, segments[0].second);
    for (int i = 1; i < segments.size(); ++i)
    {
        const double span = logSpan(segments[i].first, segments[i].second);
        if (span > widestSpan)
        {
            widestSpan = span;
            widestIdx = i;
        }
    }

    const double lo = segments[widestIdx].first;
    const double hi = segments[widestIdx].second;
    const double mid = qSqrt(lo * hi);
    if (mid <= lo || mid >= hi)
        return;

    segments[widestIdx].second = mid;
    segments.insert(widestIdx + 1, qMakePair(mid, hi));
}

QVector<double> segmentsToEdges(const QVector<QPair<double, double>> &segments)
{
    QVector<double> edges;
    if (segments.isEmpty())
        return edges;

    edges.append(segments.first().first);
    for (const auto &seg : segments)
        edges.append(seg.second);
    return edges;
}

QVector<double> adjustSegmentsToCount(QVector<QPair<double, double>> segments, int bandCount)
{
    while (segments.size() > bandCount)
        mergeNarrowestAdjacent(segments);

    while (segments.size() < bandCount)
        splitWidestSegment(segments);

    return segmentsToEdges(segments);
}

QVector<double> buildSemiLogEdges(int bandCount, double fLow, double fHigh)
{
    if (bandCount <= 0 || fHigh <= fLow)
        return QVector<double>();

    const bool useLinearTail = (fHigh > SPECTRUM_LINEAR_TAIL_START_HZ
                                && fLow < SPECTRUM_LINEAR_TAIL_START_HZ);

    if (!useLinearTail)
    {
        QVector<double> ticks = buildPreferredTickGrid(fLow, fHigh);
        QVector<QPair<double, double>> segments = ticksToSegments(ticks);
        if (segments.isEmpty())
            return buildLogUniformEdges(bandCount, fLow, fHigh, 1.0);

        return adjustSegmentsToCount(segments, bandCount);
    }

    const double fSplit = SPECTRUM_LINEAR_TAIL_START_HZ;
    const double logSpanTotal = logSpan(fLow, fHigh);
    const double logSpanLogPart = logSpan(fLow, fSplit);
    const double linearPart = fHigh - fSplit;

    int nLinear = 1;
    if (logSpanTotal > 0.0 && linearPart > 0.0)
    {
        const double linearWeight = linearPart / (logSpanLogPart + linearPart);
        nLinear = qBound(1, int(qRound(double(bandCount) * linearWeight)), bandCount - 1);
    }
    const int nLog = bandCount - nLinear;

    QVector<double> logEdges;
    if (nLog > 0)
    {
        QVector<double> ticks = buildPreferredTickGrid(fLow, fSplit);
        QVector<QPair<double, double>> segments = ticksToSegments(ticks);
        if (segments.isEmpty())
            logEdges = buildLogUniformEdges(nLog, fLow, fSplit, 1.0);
        else
            logEdges = adjustSegmentsToCount(segments, nLog);
    }

    QVector<double> linearEdges(nLinear + 1);
    for (int i = 0; i <= nLinear; ++i)
        linearEdges[i] = fSplit + (linearPart * double(i) / double(nLinear));

    if (logEdges.isEmpty())
        return linearEdges;

    QVector<double> combined;
    combined.reserve(bandCount + 1);
    for (int i = 0; i < logEdges.size(); ++i)
        combined.append(logEdges[i]);

    // skip duplicate split frequency
    for (int i = 1; i < linearEdges.size(); ++i)
        combined.append(linearEdges[i]);

    combined[0] = fLow;
    combined[combined.size() - 1] = fHigh;
    return combined;
}

} // namespace

QVector<double> computeSpectrumBandEdges(int bandCount, double fLow, double fHigh,
                                         SpectrumGridMode mode, double lowBandGamma)
{
    if (bandCount <= 0)
        return QVector<double>();

    fLow = qMax(1.0, fLow);
    if (fHigh <= fLow)
        fHigh = fLow + 1.0;

    const double gamma = qBound(1.0, lowBandGamma, 4.0);
    QVector<double> edges;

    switch (mode)
    {
        case SpectrumGridMode::SemiLogPreferred:
            edges = buildSemiLogEdges(bandCount, fLow, fHigh);
            break;
        case SpectrumGridMode::LogUniform:
        default:
            edges = buildLogUniformEdges(bandCount, fLow, fHigh, gamma);
            return edges;
    }

    return warpEdgesLowBandGamma(edges, gamma);
}

quint32 spectrumBandsRegistryKey(int bandCount, SpectrumGridMode mode, double lowBandGamma)
{
    const quint32 bands = quint32(qBound(0, bandCount, 0xFFFF));
    const quint32 grid = quint32(int(mode)) & 0xFFu;
    const int gammaEnc = qBound(0, int(qRound((qBound(1.0, lowBandGamma, 4.0) - 1.0) * 10.0)), 255);
    return bands | (grid << 16) | (quint32(gammaEnc) << 24);
}

int spectrumBandsCountFromKey(quint32 key)
{
    return int(key & 0xFFFFu);
}

SpectrumGridMode spectrumGridModeFromKey(quint32 key)
{
    const int mode = int((key >> 16) & 0xFFu);
    if (mode == int(SpectrumGridMode::SemiLogPreferred))
        return SpectrumGridMode::SemiLogPreferred;
    return SpectrumGridMode::LogUniform;
}

double spectrumLowBandGammaFromKey(quint32 key)
{
    const int gammaEnc = int((key >> 24) & 0xFFu);
    return 1.0 + double(gammaEnc) * 0.1;
}

QString spectrumGridModeToString(SpectrumGridMode mode)
{
    switch (mode)
    {
        case SpectrumGridMode::SemiLogPreferred:
            return QStringLiteral("SemiLogPreferred");
        default:
            return QStringLiteral("LogUniform");
    }
}

SpectrumGridMode spectrumGridModeFromString(const QString &str)
{
    if (str == QStringLiteral("SemiLogPreferred"))
        return SpectrumGridMode::SemiLogPreferred;
    return SpectrumGridMode::LogUniform;
}
