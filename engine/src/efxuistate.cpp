/*
  Q Light Controller Plus
  efxuistate.cpp

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
#include "efxuistate.h"

EfxUiState::EfxUiState(QObject * parent)
    : FunctionUiState(parent)
    , m_currentTab(0)
{
}

EfxUiState::~EfxUiState()
{
}

bool EfxUiState::copyFrom(const FunctionUiState* uiState)
{
    if (uiState == NULL)
        return false;

    const EfxUiState * s = qobject_cast<const EfxUiState *>(uiState);
    if (s)
    { 
         setCurrentTab(s->currentTab());
    }

    return FunctionUiState::copyFrom(uiState);
}

void EfxUiState::setCurrentTab(int currentTab)
{
    m_currentTab = currentTab;
}

int EfxUiState::currentTab() const
{
    return m_currentTab;
}

