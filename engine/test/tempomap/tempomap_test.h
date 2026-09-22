/*
  Q Light Controller Plus - Unit test
  tempomap_test.h

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

#ifndef TEMPOMAP_TEST_H
#define TEMPOMAP_TEST_H

#include <QObject>

class TempoMap_Test final : public QObject
{
    Q_OBJECT

private slots:
    void sections();
    void overlap();
    void split();
    void insertOverlappingStart();
    void insertClipsEnd();
    void insertInside();
    void insertRejected();
    void sectionIndexAt();
    void stepEndOnGrid();
    void stepEndOffGrid();
    void stepEndShortSteps();
    void stepEndNoDrift();
    void stepEndNextSection();
    void stepEndGap();
    void stepEndBeforeFirstSection();
    void beatDurationAt();
    void saveLoad();
};

#endif
