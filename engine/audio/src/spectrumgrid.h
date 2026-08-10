/*
  Q Light Controller Plus
  spectrumgrid.h

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

#ifndef SPECTRUMGRID_H
#define SPECTRUMGRID_H

#include <QtGlobal>
#include <QVector>

class QString;

/** @addtogroup engine_audio Audio
 * @{
 */

/** Frequency band partitioning mode for spectrum analysis */
enum class SpectrumGridMode
{
    /** N equal intervals on ln(f) axis (legacy QLC+ behaviour) */
    LogUniform = 0,
    /** Preferred 1-2-5 decade ticks with merge/split to N bands; optional linear Hz tail above 10 kHz */
    SemiLogPreferred = 1
};

/** Upper bound of the logarithmic (1-2-5) region; frequencies above use linear Hz steps when fHigh exceeds this */
static constexpr double SPECTRUM_LINEAR_TAIL_START_HZ = 10000.0;

/**
 * Compute band edge frequencies in Hz.
 * @return vector of size bandCount + 1; band i covers [edges[i], edges[i+1])
 */
QVector<double> computeSpectrumBandEdges(int bandCount, double fLow, double fHigh,
                                         SpectrumGridMode mode, double lowBandGamma = 1.0);

/** Pack band count, grid mode and low-band gamma for AudioCapture registry map key */
quint32 spectrumBandsRegistryKey(int bandCount, SpectrumGridMode mode, double lowBandGamma = 1.0);

/** Extract components from registry key */
int spectrumBandsCountFromKey(quint32 key);
SpectrumGridMode spectrumGridModeFromKey(quint32 key);
double spectrumLowBandGammaFromKey(quint32 key);

/** Human-readable grid mode for XML / persistence */
QString spectrumGridModeToString(SpectrumGridMode mode);
SpectrumGridMode spectrumGridModeFromString(const QString &str);

/** @} */

#endif // SPECTRUMGRID_H
