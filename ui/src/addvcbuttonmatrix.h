/*
  Q Light Controller
  addvcbuttonmatrix.h

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

#ifndef ADDVCBUTTONMATRIX_H
#define ADDVCBUTTONMATRIX_H

#include <QDialog>

#include "ui_addvcbuttonmatrix.h"

class MasterTimer;
class OutputMap;
class InputMap;
class Doc;

/** @addtogroup ui_vc_props
 * @{
 */

class AddVCButtonMatrix : public QDialog, public Ui_AddVCButtonMatrix
{
    Q_OBJECT

public:
    AddVCButtonMatrix(QWidget* parent, Doc* doc);
    ~AddVCButtonMatrix();

public:
    enum FrameStyle
    {
        NormalFrame = 0,
        SoloFrame
    };

    /** Functions to assign to buttons */
    QList <quint32> functions() const;

    /** Number of buttons horizontally */
    quint32 horizontalCount() const;

    /** Number of buttons vertically */
    quint32 verticalCount() const;

    /** Buttons' size (n x n) */
    quint32 buttonSize() const;

    /** Put buttons either in Normal or Solo VCVrame */
    FrameStyle frameStyle() const;

protected slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotHorizontalChanged();
    void slotVerticalChanged();
    void slotButtonSizeChanged();
    void slotNormalFrameToggled(bool toggled);
    void accept();

private:
    void addFunction(quint32 fid);
    void setAllocationText();
    void setFrameStyle(FrameStyle style);

private:
    QList <quint32> m_functions;
    quint32 m_horizontalCount;
    quint32 m_verticalCount;
    quint32 m_buttonSize;
    FrameStyle m_frameStyle;

    Doc* m_doc;
    OutputMap* m_outputMap;
    InputMap* m_inputMap;
    MasterTimer* m_masterTimer;
};

/** @} */

#endif
