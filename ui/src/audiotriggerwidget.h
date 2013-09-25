/*
  Q Light Controller Plus
  audiotriggerwidget.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef AUDIOTRIGGERWIDGET_H
#define AUDIOTRIGGERWIDGET_H

#include <QWidget>

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

#endif // AUDIOTRIGGERWIDGET_H
