#ifndef TREEWIDGETITEM_H
#define TREEWIDGETITEM_H

#include <QTreeWidgetItem>

class TreeWidgetItem : public QTreeWidgetItem
{
public:
    TreeWidgetItem(QTreeWidget*);
    TreeWidgetItem(QTreeWidgetItem*);

private:
    bool operator<(const QTreeWidgetItem&) const;
};

#endif
