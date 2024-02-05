/*
  Q Light Controller Plus
  vcwidgetselection.cpp

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

#include <QPushButton>
#include <QDebug>
#include <QSettings>

#include "vcwidgetselection.h"
#include "virtualconsole.h"
#include "vcframe.h"

#define KColumnName         0
#define KColumnType         1

#define SETTINGS_GEOMETRY "vcwidgetselection/geometry"

VCWidgetSelection::VCWidgetSelection(QList<int> filters, QWidget *parent)
    : QDialog(parent)
    , m_filters(filters)
{
    setupUi(this);

    m_tree->setRootIsDecorated(false);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setAllColumnsShowFocus(true);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    updateWidgetsTree();

    slotItemSelectionChanged();
}

VCWidgetSelection::~VCWidgetSelection()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

VCWidget *VCWidgetSelection::getSelectedWidget()
{
    int selIdx = m_tree->currentIndex().row();
    if (selIdx >= 0)
        return m_widgetsList.at(selIdx);
    return NULL;
}

QList<VCWidget *> VCWidgetSelection::getChildren(VCWidget *obj)
{
    QList<VCWidget *> list;
    if (obj == NULL)
        return list;
    QListIterator <VCWidget*> it(obj->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        qDebug() << Q_FUNC_INFO << "append: " << child->caption();
        if (m_filters.isEmpty() || m_filters.contains(child->type()))
            list.append(child);
    }
    return list;
}

void VCWidgetSelection::updateWidgetsTree()
{
    VCFrame *contents = VirtualConsole::instance()->contents();
    m_widgetsList = getChildren(contents);

    foreach (QObject *object, m_widgetsList)
    {
        VCWidget *widget = qobject_cast<VCWidget *>(object);

        QTreeWidgetItem *item = new QTreeWidgetItem(m_tree);
        item->setText(KColumnName, widget->caption());
        item->setIcon(KColumnName, VCWidget::typeToIcon(widget->type()));
        item->setText(KColumnType, VCWidget::typeToString(widget->type()));
    }
}

void VCWidgetSelection::slotItemSelectionChanged()
{
    if (m_tree->currentIndex().row() < 0)
    {
        // No widget selected
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void VCWidgetSelection::slotItemDoubleClicked(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    accept();
}
