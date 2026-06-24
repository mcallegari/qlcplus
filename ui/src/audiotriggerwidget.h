/*
  Q Light Controller Plus
  audiotriggerwidget.h

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

#ifndef AUDIOTRIGGERWIDGET_H
#define AUDIOTRIGGERWIDGET_H

#include <QVector>
#include <QWidget>

/** @addtogroup ui_vc_widgets
 * @{
 */

class AudioTriggerWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit AudioTriggerWidget(QWidget *parent = 0);
    ~AudioTriggerWidget();

    void setBarsNumber(int num);
    int barsNumber() const;

    void setMaxFrequency(int freq);

    /** Band edges in Hz (size bars + 1); drives bar widths on the log-frequency axis */
    void setBandFrequencyEdges(const QVector<double> &edges);

    uchar getUcharVolume() const;
    uchar getUcharBand(int idx);

signals:

public slots:
    void displaySpectrum(double *spectrumData, double maxMagnitude, quint32 power);

protected:
    void resizeEvent (QResizeEvent *e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void recomputeBarLayout();

    double *m_spectrumBands;
    int m_spectrumHeight;
    quint32 m_volumeBarHeight;

    int m_barsNumber;
    float m_barWidth;
    int m_maxFrequency;

    QVector<double> m_bandEdges;
    QVector<float> m_barXStart;
    QVector<float> m_barPixelWidth;
    float m_spectrumPixelWidth;
    float m_volumeBarPixelWidth;
};

/** @} */

#endif // AUDIOTRIGGERWIDGET_H
