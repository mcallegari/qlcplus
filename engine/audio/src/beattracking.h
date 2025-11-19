/*
  Q Light Controller Plus
  beattracking.h

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

#ifndef BeatTracking_H
#define BeatTracking_H

#include <QObject>
#include "samplerate.h"
#ifdef HAS_FFTW3
#include "fftw3.h"
#endif

/** @addtogroup engine Engine
 * @{
 */

#define BEAT_DEFAULT_SAMPLE_RATE   44100
#define BEAT_DEFAULT_WINDOW_SIZE   1024
#define BEAT_DEFAULT_HOP_SIZE      512
#define ONSET_WINDOW_LENGTH        512
#define RAILEIGH_TARGET_BPM        110

class BeatTracking : public QObject
{
    Q_OBJECT

public:
    BeatTracking(int channels, QObject * parent = nullptr);
    ~BeatTracking();
    void processAudio(int16_t * buffer, int bufferSize);

private:

    enum PredictionState{ACF, CONTINUITY};

    // beat tracking state
    PredictionState currentPredictionState;

    // setup variables
    unsigned int m_channels;
    int m_sampleRate;
    int m_windowSize;
    int m_hopSize;
    double targetLag;

    float * currentFrame;

    // FFT information
    double * m_fftInputBuffer;
    void * m_fftOutputBuffer;
    fftw_plan m_planForward;
    

    // stored values that are currently processed
    QVector<double> m_windowBuffer;

    // weighting storage
    QVector<double> m_windowWeights;
    QVector<double> m_onsetWeights;

    // methods
    QVector<double> calculateWindowWeights(int windowSize);
    QVector<double> calculateBiquadFilter(QList<double> values);
    QVector<double> getGaussianWeighting(double tLag);
    QVector<double> getRaileighFilterBank(int length, double tLag);
    QVector<double> getOnsetCorrelation(QList<double> onsetValues);
    int getPredictedAcfLag(QVector<double> oCorr);
    double getMean(QVector<double> values);
    double getMedian(QVector<double> values);
    double getQuadraticValue(int position, QVector<double> vector);


    QVector<double> raileighFilterBank;
    QVector<double> gaussianFilterBank;

    // onset storage
    QList<double> tOnsetValues;
    QList<double> onsetValuesProcessed;
    
    // consistency - Context dependent model
    double lastDifference;
    double lastLag;
    int consistencyCount;
    double continuityDerivation;
    QList<double> lastLags;

    // beat tracking
    QList<double> beatPredictions;
    int blockPosition;

    // storing values
    double identifiedLag;
    double currentBPM;
    double currentMs;

};

/** @} */

#endif
