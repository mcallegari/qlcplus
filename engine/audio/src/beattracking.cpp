/*
  Q Light Controller Plus
  beattracking.cpp

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

#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <algorithm>

#ifdef HAS_FFTW3
#include "fftw3.h"
#endif

#include "beattracking.h"


const double filterCoeffA[] = {1, 0.23484048, 0};
const double filterCoeffB[] = {0.15998789, 0.31997577, 0.15998789};

BeatTracking::BeatTracking(int channels, QObject *parent)
    : QObject(parent)
{
    m_channels = channels;
    m_sampleRate = BEAT_DEFAULT_SAMPLE_RATE;
    m_windowSize = BEAT_DEFAULT_WINDOW_SIZE;
    m_hopSize = BEAT_DEFAULT_HOP_SIZE;
    m_onsetWindowSize = ONSET_WINDOW_SIZE;

    m_currentBPM = 120;
    m_currentMs = 500;

    m_windowWeights = calculateWindowWeights(m_windowSize);
    m_onsetWeights = calculateWindowWeights(15);

    m_identifiedLag = 0;
    m_lastLag = 0;
    m_consistencyCount = 0;
    double targetLag = (m_sampleRate * 60.0) / (RAILEIGH_TARGET_BPM*m_hopSize);
    m_continuityDerivation = targetLag/8;

    m_fftInputBuffer = new double[m_windowSize];
#ifdef HAS_FFTW3
    m_fftOutputBuffer = fftw_malloc(sizeof(fftw_complex) * m_windowSize);
    m_planForward = fftw_plan_dft_r2c_1d(m_windowSize, m_fftInputBuffer, reinterpret_cast<fftw_complex*>(m_fftOutputBuffer), 0);
#endif

    m_raileighFilterBank = getRaileighFilterBank(m_onsetWindowSize, targetLag);

    m_currentPredictionState = PredictionState::ACF;

    qDebug() << "Channels: " << m_channels
             << "Sample rate:" << m_sampleRate
             << "Window size:" << m_windowSize
             << "Hop size:" << m_hopSize;
}

BeatTracking::~BeatTracking()
{
    delete[] m_fftInputBuffer;
#ifdef HAS_FFTW3
    fftw_destroy_plan(m_planForward);

    if (m_fftOutputBuffer)
        fftw_free(m_fftOutputBuffer);
#endif
}

QVector<double> BeatTracking::calculateWindowWeights(int windowSize)
{
    QVector<double> returnVector(windowSize);

    // pre calculate hanningz window
    for (int i = 0; i < windowSize; i++)
        returnVector[i] = 0.5 * (1.0 - qCos(M_PI * M_PI * i / (windowSize)));

    return returnVector;
}

QVector<double> BeatTracking::getRaileighFilterBank(int length, double tLag)
{
    QVector<double> filterBank(length, 0.0);
    // calculate target lag time

    double bSqaured = qPow(tLag,2);
    for (int i = 0; i < length; i++)
        filterBank[i] = (i / bSqaured) * qExp(-qPow(i, 2) / (2 * bSqaured));

    return filterBank;
}

QVector<double> BeatTracking::getGaussianWeighting(int windowLength, double tLag)
{
    QVector<double> returnVector(windowLength);

    double var = 2 * qPow(tLag / 8, 2.0);
    for (int i = 0; i < windowLength; i++)
        returnVector[i] = qExp(-1*qPow(i-tLag, 2.0) / var);

    return returnVector;
}

void BeatTracking::processAudio(int16_t * buffer, int bufferSize)
{
    //QElapsedTimer timer1;
    //timer1.start(); // Monitor computation time

    for (int i = 0; i < bufferSize; ++i)
    {
        // mixdown channels - Maybe do this before calling the function?
        m_windowBuffer += 0;
        for (unsigned int j = 0; j < m_channels; j++)
            m_windowBuffer[i] += static_cast<double>(buffer[i * m_channels + j]) / m_channels;

        m_windowBuffer[i] += m_windowBuffer[i] / 32768.;
    }

    // 1024 windows size, 512 advance between frames
    while (m_windowBuffer.size() > m_windowSize)
    {
        memcpy(m_fftInputBuffer, m_windowBuffer.data(), m_windowSize);

        for (int i = 0; i< m_windowSize; i++)
            m_fftInputBuffer[i] = m_fftInputBuffer[i] * m_windowWeights[i];

#ifdef HAS_FFTW3
        fftw_execute(m_planForward);
#endif

        QVector<double> magnitudes(m_windowSize/2, 0);
        double onsetValue = 0.0;
        for (int i=0; i < m_windowSize/2; i++)
        {
            double mag = qSqrt((reinterpret_cast<fftw_complex*>(m_fftOutputBuffer)[i][0] * reinterpret_cast<fftw_complex*>(m_fftOutputBuffer)[i][0]) +
                          (reinterpret_cast<fftw_complex*>(m_fftOutputBuffer)[i][1] * reinterpret_cast<fftw_complex*>(m_fftOutputBuffer)[i][1]));

            if (mag > magnitudes[i])
                onsetValue += (mag - magnitudes[i]);

            magnitudes[i] = mag;
        }

        if (m_tOnsetValues.size() == m_onsetWindowSize)
        {
            QVector<double> oCorr = getOnsetCorrelation(m_tOnsetValues);
            QVector<double> roCorr(oCorr.size());
          
            for (int l = 1; l < oCorr.size(); l++)
            {
                // reileigh filter
                roCorr[l] = oCorr[l] * m_raileighFilterBank[l];
            }  

            int acfLagIndex = getPredictedAcfLag(roCorr);

            if (acfLagIndex != 0)
            {
                double acfLag = getQuadraticValue(acfLagIndex, roCorr);
                double continuityLag = 0;
                
                if (m_currentPredictionState == PredictionState::CONTINUITY)
                {
                    int continuityIndex = 0;
                    if (m_lastLag != m_identifiedLag)
                    {
                        m_gaussianFilterBank = getGaussianWeighting(m_onsetWindowSize, m_identifiedLag);

                        m_lastLag = m_identifiedLag;
                    }
                    double maxVal = 0.0;
                    for (int l=1; l < oCorr.size(); l++)
                    {
                        double val = oCorr[l] * m_gaussianFilterBank[l];
                        if (val > maxVal)
                        {
                            maxVal = val;
                            continuityIndex = l;
                        }
                    }
                    continuityLag = getQuadraticValue(continuityIndex, oCorr);
                    m_identifiedLag = continuityLag;
                    // qDebug() << "Consistent value: " << continuityLag << " " << continuityLag * m_hopSize / 44.1 << " " << (44100*60)/(m_hopSize*continuityLag);                    
                }
                else if (m_currentPredictionState == PredictionState::ACF)
                {
                    m_identifiedLag = acfLag;
                    // qDebug() << "ACF: " << acfLag << " " << acfLag * m_hopSize / 44.1 << " " << (44100*60)/(m_hopSize*acfLag);
                    m_lastLag = 0;
                }

                // check if continuity still remains
                if (m_currentPredictionState == PredictionState::CONTINUITY && qAbs(acfLag - continuityLag) >= m_continuityDerivation)
                {
                    if (m_consistencyCount++ > 1)
                    {
                        m_currentPredictionState = PredictionState::ACF;
                        m_lastLags.clear();
                    }
                }
                else 
                {
                    m_consistencyCount = 0;
                }
                // add last lags to buffer and check consistency
                m_lastLags += acfLag;
                if (m_lastLags.size() > 3)
                {
                    m_lastLags.removeFirst();
                    //qDebug() << "DERIVATION: " << continuityDerivation << "Continuity: " << qAbs(acfLag - continuityLag) << "Consistency: " << qAbs(2*m_lastLags[2]-m_lastLags[1]-m_lastLags[0]); 
                    // check continuity
                    if (m_currentPredictionState == PredictionState::ACF && qAbs(2*m_lastLags[2]-m_lastLags[1]-m_lastLags[0]) < m_continuityDerivation)
                    {
                        //qDebug() << "CONTINUITY ESTABLISHED: " << 2*m_lastLags[2] << " " << m_lastLags[1] << " " << m_lastLags[0] << " " << qAbs(2*m_lastLags[2]-m_lastLags[1]-m_lastLags[0]); 
                        m_currentPredictionState = PredictionState::CONTINUITY;
                    }
                }

                m_currentBPM = (44100*60)/(m_hopSize*m_identifiedLag);
                m_currentMs = m_identifiedLag * m_hopSize / 44.1;

                // Beat Tracking and phase detection
                int cMax = qFloor(m_tOnsetValues.size() / m_identifiedLag);
                QVector<double> phaseOnsetValues(m_tOnsetValues.size(), 0.0);

                // reverse onset values and add weighting so that most recent events are more likely
                for (int i = 0; i < phaseOnsetValues.size(); i++)
                {
                    phaseOnsetValues[i] = m_tOnsetValues[m_tOnsetValues.size() - i - 1] * qExp(-1.0*i*(qLn(2)/m_identifiedLag));
                }

                // phase calculation by autocorrelation with train of impulses
                QVector<double> phaseValues(phaseOnsetValues.size(), 0.0);
                double maxPhaseValue = 0.0;
                int phaseIndex = 0.0;
                for (int i=0; i < phaseOnsetValues.size(); i++)
                {
                    for (int c=0; c < cMax; c++)
                    {
                        int index = i + qRound(c*m_identifiedLag);
                        if (index < phaseOnsetValues.size())
                        {
                            phaseValues[i] += phaseOnsetValues[index];
                        }
                    }

                    if (phaseValues[i] > maxPhaseValue)
                    {
                        maxPhaseValue = phaseValues[i];
                        phaseIndex = i;
                    }
                }
                double phase;
                phase = getQuadraticValue(phaseIndex, phaseValues);

                // one frame delay
                phase++;
                double beat = m_identifiedLag - phase;

                // check for negative results
                while (beat + m_identifiedLag < 0) {
                    beat += m_identifiedLag;
                }

                // TODO context dependent beat tracking?
                m_beatPredictions.clear();
                for (int i = beat; i < m_windowSize/4; i+=m_identifiedLag)
                {
                    m_beatPredictions += i;
                }
            }

            if (m_tOnsetValues.size() == m_onsetWindowSize)
            {
                m_tOnsetValues.remove(0, 128);
            }

            m_blockPosition = -1;
        }

        m_blockPosition++;
        m_onsetValuesProcessed += onsetValue;
        if (m_onsetValuesProcessed.size() > 7)
        {
            m_onsetValuesProcessed.removeFirst();
            QVector<double> filtered = calculateBiquadFilter(m_onsetValuesProcessed);

            double mean = getMean(filtered);
            double median = getMedian(filtered);
            double thresholded = filtered[5] - median - mean * 0.1;

            m_tOnsetValues += thresholded;
            //qDebug() << "T" << thresholded;
        }

        for (double v : m_beatPredictions)
        {
            if (m_blockPosition == qFloor(v))
            {
                qDebug() << m_currentBPM << " (" << m_currentMs << "ms)" << " Beat";
                //beatPredictions.removeFirst();
                break;
            }
            else if (m_blockPosition < v)
            {
                break;
            }
        }
        // remove processed samples, but use advance size 
        m_windowBuffer.remove(0, m_hopSize);
    } 
    //qDebug() << "Elapsed Time: " << timer1.elapsed();
}

QVector<double> BeatTracking::getOnsetCorrelation(QList<double> onsetValues)
{
    QVector<double> autoCorr(onsetValues.size());
    for (int l = 0; l < onsetValues.size(); l++)
    {
        double divider = qAbs(l - onsetValues.size());
        double sum = 0.0;
        for (int i = l; i < onsetValues.size(); i++)
        {
            sum += onsetValues[i] * onsetValues[i - l];
        }

        autoCorr[l] = sum / divider;
    }

    autoCorr[0] = 0;
    
    autoCorr[autoCorr.size()-1] = 0;
    
    // comb filter
    QVector<double> combRes(onsetValues.size(), 0.0);
    for (int l = 1; l < combRes.size(); l++)
    {
        for (int a = 1; a <= 4; a++)
        {
            for (int b = 1 - a; b < 2 * a; b++)
            {
                int index = l * a + b - 1;
                if (index > 0 && index < autoCorr.size())
                    combRes[l] += autoCorr[index] / (2*a-1);
            }
        }
    }
    return combRes;
}


int BeatTracking::getPredictedAcfLag(QVector<double> roCorr)
{
    QVector<double> tps2(roCorr.size()/2);
    QVector<double> tps3(roCorr.size()/2);
    
    double max2I = 0.0, max3I = 0.0;
    double max2 = 0.0, max3 = 0.0;
    for (int r = 1; r < roCorr.size() / 2 - 1; r++)
    {
        tps2[r] = roCorr[r] + 0.5 * roCorr[2 * r] + 0.25 * roCorr[2 * r - 1] + 0.25 * roCorr[2 * r + 1];

        if (tps2[r] > max2)
        {
            max2I = r;
            max2 = tps2[r];
        }
    }
    for (int r = 1; r < roCorr.size() / 3 - 1; r++)
    {
        tps3[r] = roCorr[r] + 0.33 * roCorr[3 * r] + 0.33 * roCorr[3 * r - 1] + 0.33 * roCorr[3 * r + 1];

        if (tps3[r] > max3)
        {
            max3I = r;
            max3 = tps3[r];
        }
    }
    if (max2 >= max3)
        return max2I;
    else
        return max3I;
}

QVector<double> BeatTracking::calculateBiquadFilter(QList<double> values)
{
    QList<double> processed(values.size(), 0.0);

    for (int i = 0; i < values.size(); i++)
    {
        processed[i] = filterCoeffB[0] * values[i];
        for (int order = 1; order < 3; order++)
        {
            if (i - order >= 0)
            {
                processed[i] += filterCoeffB[order] * values[i-order];
                processed[i] -= filterCoeffA[order] * processed[i-order];
            }
        }
    }

    // Calculate backwards
    for (int i = values.size()-1; i >= 0; i--)
    {
        processed[i] = filterCoeffB[0] * values[i];
        for (int order = 1; order < 3; order++)
        {
            if (i + order < values.size())
            {
                processed[i] += filterCoeffB[order] * values[i+order];
                processed[i] -= filterCoeffA[order] * processed[i+order];
            }
        }
    }

    return processed;
}

double BeatTracking::getMean(QVector<double> values)
{
    double mean = 0.0;
    for (double value : values)
        mean += value;

    mean = mean / values.size();
    return mean;
}

double BeatTracking::getMedian(QVector<double> values)
{
    std::sort(values.begin(), values.end());

    return values[qFloor(values.size()/2) + 1];
}


double BeatTracking::getQuadraticValue(int position, QVector<double> vector)
{
    double prevValue = 0;

    if (position > 0)
        prevValue = vector[position - 1];

    return static_cast<double>(position) + 0.5 * (prevValue - vector[position + 1]) / (prevValue - 2 * vector[position] + vector[position + 1]);
}  
