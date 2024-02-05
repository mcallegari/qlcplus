/*
  Q Light Controller Plus
  chaseraction.h

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

#ifndef CHASERACTION_H
#define CHASERACTION_H

#include <QtGlobal>

enum ChaserActionType
{
    ChaserNoAction,
    ChaserStopStep,
    ChaserNextStep,
    ChaserPreviousStep,
    ChaserSetStepIndex,
    ChaserPauseRequest
};

typedef struct
{
    ChaserActionType m_action;
    qreal m_masterIntensity;
    qreal m_stepIntensity;
    int m_fadeMode;
    int m_stepIndex;
} ChaserAction;

#endif /* CHASERACTION_H */
