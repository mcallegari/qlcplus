/*
  Q Light Controller
  addvcslidermatrix.h

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

#ifndef ADDVCSLIDERMATRIX_H
#define ADDVCSLIDERMATRIX_H

#include <QDialog>
#include "ui_addvcslidermatrix.h"

/** @addtogroup ui_vc_props
 * @{
 */

class AddVCSliderMatrix : public QDialog, public Ui_AddVCSliderMatrix
{
    Q_OBJECT

public:
    AddVCSliderMatrix(QWidget* parent);
    ~AddVCSliderMatrix();

    int amount() const;
    int height() const;
    int width() const;

public slots:
    void accept();

protected:
    int m_amount;
    int m_height;
    int m_width;
};

/** @} */

#endif
