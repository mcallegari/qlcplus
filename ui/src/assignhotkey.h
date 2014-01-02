/*
  Q Light Controller
  assignhotkey.h

  Copyright (c) Heikki Junnila

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

#ifndef ASSIGNHOTKEY_H
#define ASSIGNHOTKEY_H

#include <QKeySequence>
#include <QDialog>

#include "ui_assignhotkey.h"

class QKeyEvent;

/** @addtogroup ui UI
 * @{
 */

class AssignHotKey : public QDialog, public Ui_AssignHotKey
{
    Q_OBJECT
    Q_DISABLE_COPY(AssignHotKey)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    AssignHotKey(QWidget* parent, const QKeySequence& keySequence = QKeySequence());
    ~AssignHotKey();

    /*********************************************************************
     * Key sequence
     *********************************************************************/
public:
    /** Get the key sequence */
    QKeySequence keySequence() const;

private:
    QKeySequence m_keySequence;

protected:
    /** @reimp */
    void keyPressEvent(QKeyEvent* event);
};

/** @} */

#endif
