/*
  Q Light Controller
  aboutbox.h

  Copyright (C) Heikki Junnila

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

#ifndef ABOUTBOX_H
#define ABOUTBOX_H

#include <QDialog>
#include "ui_aboutbox.h"

class QTimer;

/** @addtogroup ui UI
 * @{
 */

class AboutBox : public QDialog, public Ui_AboutBox
{
    Q_OBJECT
    Q_DISABLE_COPY(AboutBox)

public:
    AboutBox(QWidget* parent);
    ~AboutBox();

private slots:
    void slotTimeout();
    void slotItemClicked();
    void slotWebsiteClicked();
    void slotAboutQt();

private:
    QTimer* m_timer;
    int m_row;
    int m_increment;
};

/** @} */

#endif
