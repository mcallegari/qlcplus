/*
  Q Light Controller Plus
  timingstool.h

  Copyright (C) Massimo Callegari

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

#ifndef TIMINGSTOOL_H
#define TIMINGSTOOL_H

#include <QRadioButton>
#include <QGroupBox>
#include <QWidget>

class SpeedDial;
class ShowItem;

class TimingsTool : public QWidget
{
    Q_OBJECT
public:
    explicit TimingsTool(ShowItem *item, QWidget *parent = 0);
    ~TimingsTool();

    void showDurationControls(bool show);
    void showDurationOptions(bool show);

signals:
    void startTimeChanged(ShowItem *item, int msec);
    void durationChanged(ShowItem *item, int msec, bool stretch);

protected slots:
    void slotStartTimeChanged(int msec);
    void slotDurationChanged(int msec);

private:
    SpeedDial* m_startDial;
    SpeedDial* m_durationDial;
    ShowItem *m_item;

    QGroupBox* m_durationOptions;
    QRadioButton *m_stretchOriginalRadio;
    QRadioButton *m_expandLoopRadio;

};

#endif // TIMINGSTOOL_H
