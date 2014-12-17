/*
  Q Light Controller Plus
  rgbaudio.h

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

#ifndef RGBAUDIO_H
#define RGBAUDIO_H

#include <QObject>
#include <QMutex>

#include "rgbalgorithm.h"

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCRGBAudio "Audio"

class AudioCapture;

class RGBAudio : public QObject, public RGBAlgorithm
{
    Q_OBJECT

public:
    RGBAudio(const Doc * doc);
    RGBAudio(const RGBAudio& t, QObject *parent = 0);
    ~RGBAudio();

    /** @reimp */
    RGBAlgorithm* clone() const;

    void setAudioCapture(AudioCapture *cap);

protected slots:
    void slotAudioBarsChanged(double *spectrumBands, int size, double maxMagnitude, quint32 power);

private:
    void calculateColors(int barsHeight = 0);

protected:
    AudioCapture *m_audioInput;
    int m_bandsNumber;
    QMutex m_mutex;
    QVector<double>m_spectrumValues;
    double m_maxMagnitude;
    quint32 m_volumePower;
    QList<uint> m_barColors;

    /************************************************************************
     * RGBAlgorithm
     ************************************************************************/
public:
    /** @reimp */
    int rgbMapStepCount(const QSize& size);

    /** @reimp */
    RGBMap rgbMap(const QSize& size, uint rgb, int step);

    /** @reimp */
    QString name() const;

    /** @reimp */
    QString author() const;

    /** @reimp */
    int apiVersion() const;

    /** @reimp */
    void setColors(QColor start, QColor end);

    /** @reimp */
    RGBAlgorithm::Type type() const;

    /** @reimp */
    int acceptColors() const;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** @reimp */
    bool loadXML(const QDomElement& root);

    /** @reimp */
    bool saveXML(QDomDocument* doc, QDomElement* mtx_root) const;
};

/** @} */

#endif
