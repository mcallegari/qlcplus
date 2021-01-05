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

#include <QWidget>

/** @addtogroup ui_vc_widgets
 * @{
 */

class AudioTriggerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AudioTriggerWidget(QWidget *parent = 0);

    void setBarsNumber(int num);
    int barsNumber();

    void setMaxFrequency(int freq);

    uchar getUcharVolume();
    uchar getUcharBand(int idx);

signals:

public slots:
    void displaySpectrum(double *spectrumData, double maxMagnitude, quint32 power);

protected:
    void resizeEvent (QResizeEvent *e);
    void paintEvent(QPaintEvent* e);

private:
    double *m_spectrumBands;
    int m_spectrumHeight;
    quint32 m_volumeBarHeight;

    int m_barsNumber;
    float m_barWidth;
    int m_maxFrequency;
};

/** @} */

#endif // AUDIOTRIGGERWIDGET_H
