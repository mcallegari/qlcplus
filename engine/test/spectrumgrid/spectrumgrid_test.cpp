/*
  Q Light Controller Plus - Unit test
  spectrumgrid_test.cpp

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
#include <qmath.h>

#include "spectrumgrid_test.h"
#include "spectrumgrid.h"

static bool isPreferredTick(double f)
{
    if (f < 1.0)
        return false;
    const double decade = qPow(10.0, qFloor(qLn(f) / qLn(10.0) + 1e-9));
    const double ratio = f / decade;
    return qAbs(ratio - 1.0) < 1e-3 || qAbs(ratio - 2.0) < 1e-3 || qAbs(ratio - 5.0) < 1e-3;
}

void SpectrumGrid_Test::logUniform_edges()
{
    const double fLow = 40.0;
    const double fHigh = 5000.0;
    const int N = 20;

    const QVector<double> edges = computeSpectrumBandEdges(N, fLow, fHigh, SpectrumGridMode::LogUniform);
    QCOMPARE(edges.size(), N + 1);
    QCOMPARE(edges.first(), fLow);
    QCOMPARE(edges.last(), fHigh);

    for (int i = 0; i < edges.size() - 1; ++i)
        QVERIFY(edges[i] < edges[i + 1]);
}

void SpectrumGrid_Test::semiLog_edges()
{
    const double fLow = 40.0;
    const double fHigh = 5000.0;
    const int N = 64;

    const QVector<double> edges = computeSpectrumBandEdges(N, fLow, fHigh, SpectrumGridMode::SemiLogPreferred);
    QCOMPARE(edges.size(), N + 1);
    QCOMPARE(edges.first(), fLow);
    QCOMPARE(edges.last(), fHigh);

    for (int i = 0; i < edges.size() - 1; ++i)
        QVERIFY(edges[i] < edges[i + 1]);

    int preferredInternal = 0;
    for (int i = 1; i < edges.size() - 1; ++i)
    {
        if (isPreferredTick(edges[i]))
            ++preferredInternal;
    }
    QVERIFY(preferredInternal > 0);
}

void SpectrumGrid_Test::lowBandGamma_narrowsLowBands()
{
    const double fLow = 40.0;
    const double fHigh = 5000.0;
    const int N = 32;

    const QVector<double> neutral = computeSpectrumBandEdges(N, fLow, fHigh,
                                                             SpectrumGridMode::LogUniform, 1.0);
    const QVector<double> emphasized = computeSpectrumBandEdges(N, fLow, fHigh,
                                                                SpectrumGridMode::LogUniform, 2.0);
    QCOMPARE(neutral.size(), N + 1);
    QCOMPARE(emphasized.size(), N + 1);

    const double neutralWidth = neutral[1] - neutral[0];
    const double emphasizedWidth = emphasized[1] - emphasized[0];
    QVERIFY(emphasizedWidth < neutralWidth);
}

void SpectrumGrid_Test::registryKey_roundTrip()
{
    const quint32 key = spectrumBandsRegistryKey(20, SpectrumGridMode::SemiLogPreferred, 1.5);
    QCOMPARE(spectrumBandsCountFromKey(key), 20);
    QCOMPARE(spectrumGridModeFromKey(key), SpectrumGridMode::SemiLogPreferred);
    QCOMPARE(spectrumLowBandGammaFromKey(key), 1.5);
}

void SpectrumGrid_Test::gridMode_strings()
{
    QCOMPARE(spectrumGridModeFromString(QStringLiteral("LogUniform")), SpectrumGridMode::LogUniform);
    QCOMPARE(spectrumGridModeFromString(QStringLiteral("SemiLogPreferred")), SpectrumGridMode::SemiLogPreferred);
    QCOMPARE(spectrumGridModeToString(SpectrumGridMode::LogUniform), QStringLiteral("LogUniform"));
}

QTEST_APPLESS_MAIN(SpectrumGrid_Test)
