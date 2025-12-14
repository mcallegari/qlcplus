/*
  Q Light Controller
  fixturegroupeditor.h

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

#ifndef FIXTUREGROUPEDITOR_H
#define FIXTUREGROUPEDITOR_H

#include <QWidget>

#include "ui_fixturegroupeditor.h"

class FixtureGroup;
class Doc;

/** @addtogroup ui_fixtures
 * @{
 */

class FixtureGroupEditor : public QWidget, public Ui_FixtureGroupEditor
{
    Q_OBJECT

public:
    FixtureGroupEditor(FixtureGroup* grp, Doc* doc, QWidget* parent);
    ~FixtureGroupEditor();

private:
    void updateTable();
    void addFixtureHeads(Qt::ArrowType direction);

private slots:
    void slotNameEdited(const QString& text);
    void slotXSpinValueChanged(int value);
    void slotYSpinValueChanged(int value);

    void slotRightClicked();
    void slotLeftClicked(); 
    void slotUpClicked();
    void slotDownClicked();
    void slotRemoveFixtureClicked();

    void slotCellActivated(int row, int column);
    void slotCellChanged(int row, int column);
    void slotResized();

private:
    FixtureGroup* m_grp; //! The group being edited
    Doc* m_doc;          //! The QLC engine object
    int m_row;           //! Currently selected row
    int m_column;        //! Currently selected column
};

/** @} */

#endif
