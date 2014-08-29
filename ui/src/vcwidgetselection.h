/*
  Q Light Controller Plus
  vcwidgetselection.h

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

#ifndef VCWIDGETSELECTION_H
#define VCWIDGETSELECTION_H

#include <QDialog>

#include "ui_vcwidgetselection.h"

class VCWidget;

/** @addtogroup ui_vc_props
 * @{
 */

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

protected slots:
    void slotItemSelectionChanged();
    void slotItemDoubleClicked(QTreeWidgetItem* item);
};

/** @} */

#endif // VCWIDGETSELECTION_H
