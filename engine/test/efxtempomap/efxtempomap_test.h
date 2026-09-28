/*
  Q Light Controller Plus - Unit test
  efxtempomap_test.h

  Copyright (c) Matt Carter (mjcmatrix)

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

#ifndef EFXTEMPOMAP_TEST_H
#define EFXTEMPOMAP_TEST_H

#include <QObject>
#include <functional>

class Doc;
class EFX;
class Show;
class TempoMap;
class ShowFunction;

class EFXTempoMap_Test final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void itemOutsideSections();
    void itemOverlapsSectionStart();
    void itemOverlapsSectionEnd();
    void itemAcrossGap();
    void itemAcrossAdjacentSections();
    void itemAcrossAdjacentOffGrid();
    void itemOffGridStart();
    void itemAllSectionsRemoved();
    void fallbackBpmChange();
    void showStartedMidItem();
    void showStartedMidItemSerial();
    void showStartedPastSingleShot();
    void showPauseResume();
    void showPauseStop();
    void showStopRestart();
    void efxOutsideShow();
    void sharedEFXStartedOutsideFirst();
    void sharedEFXStartedByShowFirst();
    void efxInCollection();
    void efxInChaser();
    void timeEFXInTempoShow();
    void fadeInOnTempoMap();

private:
    EFX *createEFX(int fixtures, uint loopBeats);
    Show *createShow(const TempoMap &map);
    ShowFunction *addItem(Show *show, quint32 functionId, quint32 start, quint32 duration);
    void tick(int count = 1);
    bool tickUntil(EFX *efx, double time);
    quint32 showTime(Show *show) const;

    /** Run $show from $start until $efx stops or $maxTicks, checking on
     *  each tick that the EFX is where $expected (beats at a Show time)
     *  says it should be */
    void runChecked(Show *show, EFX *efx, quint32 itemStart,
                    std::function<double(double)> expected, int maxTicks);

private:
    Doc *m_doc;
};

#endif
