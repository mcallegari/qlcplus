/*
  Q Light Controller Plus
  sceneuistate.cpp

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
#include "sceneuistate.h"

SceneUiState::SceneUiState(QObject * parent)
    : FunctionUiState(parent)
    , m_displayMode(Tabbed)
    , m_currentTab(0)
{
}

SceneUiState::~SceneUiState()
{
}

bool SceneUiState::copyFrom(const FunctionUiState* uiState)
{
    if (uiState == NULL)
        return false;

    const SceneUiState * s = qobject_cast<const SceneUiState *>(uiState);
    if (s)
    { 
         setDisplayMode(s->displayMode());
         setCurrentTab(s->currentTab());
    }

    return FunctionUiState::copyFrom(uiState);
}

void SceneUiState::setDisplayMode(DisplayMode mode)
{
    m_displayMode = mode;
}

SceneUiState::DisplayMode SceneUiState::displayMode() const
{
    return m_displayMode;
}

void SceneUiState::setCurrentTab(int currentTab)
{
    qDebug() << Q_FUNC_INFO << currentTab;
    m_currentTab = currentTab;
}

int SceneUiState::currentTab() const
{
    return m_currentTab;
}
