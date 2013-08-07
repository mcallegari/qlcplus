/*
  Q Light Controller Plus
  vcwidgetselection.cpp

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

#include "vcwidgetselection.h"
#include "virtualconsole.h"
#include "vcframe.h"
#include "doc.h"

#define KColumnName         0
#define KColumnType         1

VCWidgetSelection::VCWidgetSelection(QList<int> filters, QWidget *parent)
    : QDialog(parent)
    , m_filters(filters)
{
    setupUi(this);

    m_tree->setRootIsDecorated(false);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setAllColumnsShowFocus(true);

    updateWidgetsTree();
}

VCWidgetSelection::~VCWidgetSelection()
{

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
    VCFrame* contents = VirtualConsole::instance()->contents();
    m_widgetsList = getChildren((VCWidget *)contents);

    foreach (QObject *object, m_widgetsList)
    {
        VCWidget *widget = (VCWidget *)object;

        QTreeWidgetItem *item = new QTreeWidgetItem(m_tree);
        item->setText(KColumnName, widget->caption());
        item->setIcon(KColumnName, VCWidget::typeToIcon(widget->type()));
        item->setText(KColumnType, VCWidget::typeToString(widget->type()));
    }
}
