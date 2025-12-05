/*
  Q Light Controller Plus
  beattracking.h

  Copyright (c) Dennis Suermann

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

#ifndef BEATTRACKING_H
#define BEATTRACKING_H

#include <QObject>
#ifdef HAS_FFTW3
  #include "fftw3.h"
#endif

/** @addtogroup engine Engine
 * @{
 */

#define BEAT_DEFAULT_SAMPLE_RATE   44100
#define BEAT_DEFAULT_WINDOW_SIZE   1024
#define BEAT_DEFAULT_HOP_SIZE      512
#define ONSET_WINDOW_SIZE          512
#define RAILEIGH_TARGET_BPM        110

class BeatTracking : public QObject
{
    Q_OBJECT

public:
    BeatTracking(int channels, QObject *parent = nullptr);
    ~BeatTracking();
    bool processAudio(int16_t *buffer, int bufferSize);

private:
    enum PredictionState{ACF, CONTINUITY};

    // beat tracking state
    PredictionState m_currentPredictionState;

    // setup variables
    unsigned int m_channels;
    int m_sampleRate;
    int m_windowSize;
    int m_hopSize;
    int m_onsetWindowSize;

    // FFT information
    double *m_fftInputBuffer;
    void *m_fftOutputBuffer;
#ifdef HAS_FFTW3
    fftw_plan m_planForward;
#endif

    // stored values that are currently processed
    QVector<double> m_windowBuffer;

    // weighting storage
    QVector<double> m_windowWeights;

    // methods
    QVector<double> calculateWindowWeights(int windowSize);
    QVector<double> getRaileighFilterBank(int length, double tLag);
    QVector<double> getGaussianWeighting(int length, double tLag);
    int getPredictedAcfLag(const QVector<double> &oCorr);
    QVector<double> getOnsetCorrelation(const QList<double> &onsetValues);
    QVector<double> calculateBiquadFilter(const QList<double> &values);
    double getMean(const QVector<double> &values);
    double getMedian(QVector<double> values); // keep by value since we sort
    double getQuadraticValue(int position, const QVector<double> &vector);


    QVector<double> m_raileighFilterBank;
    QVector<double> m_gaussianFilterBank;
    QVector<double> m_prevMagnitudes;

    // onset storage
    QList<double> m_tOnsetValues;
    QList<double> m_onsetValuesProcessed;
    
    // consistency - Context dependent model
    double m_lastLag;
    int m_consistencyCount;
    double m_continuityDerivation;
    QList<double> m_lastLags;

    // beat tracking
    QList<double> m_beatPredictions;
    int m_blockPosition;

    // storing values
    double m_identifiedLag;
    double m_currentBPM;
    double m_currentMs;

    double m_silenceGateThreshold;   // RMS threshold; 0.0 disables gate
};

/** @} */

#endif
