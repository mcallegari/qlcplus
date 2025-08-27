#include "treewidgetitem.h"
#include "vcsliderproperties.h"


TreeWidgetItem::TreeWidgetItem(QTreeWidget* parent)
    : QTreeWidgetItem(parent)
{
}


TreeWidgetItem::TreeWidgetItem(QTreeWidgetItem* parent)
    : QTreeWidgetItem(parent)
{
}


bool TreeWidgetItem::operator<(const QTreeWidgetItem &rhs) const
{
    int column = treeWidget()->sortColumn();

    if (column == KColumnName)
        return this->text(KColumnID).toUInt() < rhs.text(KColumnID).toUInt();

    return QTreeWidgetItem::operator<(rhs);
}
