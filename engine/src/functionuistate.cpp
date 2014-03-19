/*
  Q Light Controller Plus
  functionuistate.cpp

  Copyright (c) Jano Svitok

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

#include "functionuistate.h"

FunctionUiState::FunctionUiState(QObject * parent)
    : QObject(parent)
    , m_showSpeedDial(false)
{
}

FunctionUiState::~FunctionUiState()
{
}

bool FunctionUiState::copyFrom(const FunctionUiState * uiState)
{
    if (uiState == NULL)
        return false;

    setShowSpeedDial(uiState->showSpeedDial());
    return true;
}

void FunctionUiState::setShowSpeedDial(bool show)
{
    m_showSpeedDial = show;
}

bool FunctionUiState::showSpeedDial() const
{
    return m_showSpeedDial;
}

