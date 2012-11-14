/*
  Q Light Controller
  showeditor.h

  Copyright (c) Massimo Callegari

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

#ifndef SHOWEDITOR_H
#define SHOWEDITOR_H

#include "ui_showeditor.h"

class Show;
class Doc;

class ShowEditor : public QWidget, public Ui_ShowEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(ShowEditor)

public:
    ShowEditor(QWidget* parent, Show* fc, Doc* doc);
    ~ShowEditor();

private:
    Doc* m_doc;
    Show* m_show; // The Show being edited

private slots:
    void slotNameEdited(const QString& text);
    void slotAdd();
    void slotRemove();

private:
    void updateFunctionList();
};

#endif