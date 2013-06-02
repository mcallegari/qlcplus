/*
  Q Light Controller Plus
  vcwidgetselection.h

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

#ifndef VCWIDGETSELECTION_H
#define VCWIDGETSELECTION_H

#include <QDialog>

#include "ui_vcwidgetselection.h"

class VCWidget;

class VCWidgetSelection : public QDialog, public Ui_VCWidgetSelection
{
    Q_OBJECT
    
public:
    explicit VCWidgetSelection(QList<int> filters, QWidget *parent = 0);
    ~VCWidgetSelection();

    VCWidget* getSelectedWidget();

private:
    QList<int> m_filters;
    QList<VCWidget *> m_widgetsList;

protected:
    QList<VCWidget *> getChildren(VCWidget *obj);
    void updateWidgetsTree();
};

#endif // VCWIDGETSELECTION_H
