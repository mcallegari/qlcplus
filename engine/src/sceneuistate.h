/*
  Q Light Controller Plus
  sceneuistate.h

  Copyright (C) Jano Svitok

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

#ifndef SCENEUISTATE_H
#define SCENEUISTATE_H

#include "functionuistate.h"

/** @addtogroup engine_functions Functions
 * @{
 */

class SceneUiState : public FunctionUiState
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneUiState);

public:
    explicit SceneUiState(QObject * parent);
    virtual ~SceneUiState();

    virtual bool copyFrom(const FunctionUiState* uiState);

public:
    enum DisplayMode { AllChannels, Tabbed };

    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;

private:
    DisplayMode m_displayMode;

public:
    void setCurrentTab(int currentTab);
    int currentTab() const;

private:
    int m_currentTab;
};

/** @} */

#endif
