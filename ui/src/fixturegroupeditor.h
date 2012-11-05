/*
  Q Light Controller
  fixturegroupeditor.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef FIXTUREGROUPEDITOR_H
#define FIXTUREGROUPEDITOR_H

#include <QWidget>

#include "ui_fixturegroupeditor.h"

class FixtureGroup;
class Doc;

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
    void slotDownClicked();
    void slotRemoveFixtureClicked();

    void slotCellActivated(int row, int column);
    void slotCellChanged(int row, int column);

private:
    FixtureGroup* m_grp; //! The group being edited
    Doc* m_doc;          //! The QLC engine object
    int m_row;           //! Currently selected row
    int m_column;        //! Currently selected column
};

#endif
