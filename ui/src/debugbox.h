/*
  Q Light Controller
  debugbox.h

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

#ifndef DEBUGBOX_H
#define DEBUGBOX_H

#include <QDialog>
#include <QMutex>
#include "ui_debugbox.h"

/** @addtogroup ui UI
 * @{
 */

#define DEBUGBOX_FLAGS Qt::WindowFlags(Qt::Window)

/** Displays debug messages in Windows and OS X (since they don't have console)
 */
class DebugBox : public QDialog, public Ui_DebugBox
{
    Q_OBJECT
    Q_DISABLE_COPY(DebugBox)

public:
    DebugBox(QWidget* parent = 0, Qt::WindowFlags flags = DEBUGBOX_FLAGS);
    ~DebugBox();

    void addText(QString text);
private:
    QMutex m_mutex;
};

/** @} */

#endif
