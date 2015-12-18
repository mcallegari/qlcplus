/*
  Q Light Controller Plus
  functionuistate.h

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

#ifndef FUNCTIONUISTATE_H
#define FUNCTIONUISTATE_H

#include <QObject>

/** @addtogroup engine_functions Functions
 * @{
 */

class FunctionUiState : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FunctionUiState)

public:
    explicit FunctionUiState(QObject * parent);
    virtual ~FunctionUiState();

    virtual bool copyFrom(const FunctionUiState* uiState);

public:
    void setShowSpeedDial(bool show);
    bool showSpeedDial() const;

private:
    bool m_showSpeedDial;
};

/** @} */

#endif
