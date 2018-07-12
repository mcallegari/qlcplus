/*
  Q Light Controller Plus
  treemodel.h

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

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractListModel>
#include <QStringList>

#define SEARCH_MIN_CHARS    3

class TreeModelItem;

class TreeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(TreeModel)

public:
    enum FixedRoles
    {
        LabelRole = Qt::UserRole + 1,
        PathRole,
        IsExpandedRole,
        IsSelectedRole,
        IsCheckableRole,
        IsCheckedRole,
        IsDraggableRole,
        ItemsCountRole,
        HasChildrenRole,
        ChildrenModel,
        FixedRolesEnd
    };

    enum TreeItemsFlags
    {
        Selected  = (1 << 0),
        EmptyNode = (1 << 1),
        Expanded  = (1 << 2),
        Checkable = (1 << 3),
        Checked   = (1 << 4),
        Draggable = (1 << 5)
    };

    TreeModel(QObject *parent = 0);
    ~TreeModel();

    static QChar separator();

    /** Recursive clear of all the tree items */
    void clear();

    /** Set a list of custom column names */
    void setColumnNames(QStringList names);

    /** Enable/disable the alphabetic sort of the tree items */
    void enableSorting(bool enable);

    /** Enable/disable checkable flag of tree items */
    void setCheckable(bool enable);

    /** Given an item $item, deselect all the other items in the tree */
    void setSingleSelection(TreeModelItem *item);

    /** Add a new item to this tree.
     *  Note that by 'item' here we mean 'leaf' with name $label and path $path.
     *  If $path is composed (e.g. a/b/c/d) all the top nodes will be created, by
     *  using the TreeModelItem::addChild method.
     *  Therefore, $data belongs to the leaf, if you want to add data to a top node,
     *  use the setPathData method.
     *  $flags are used to give an item a specific initial state. See TreeItemsFlags */
    TreeModelItem *addItem(QString label, QVariantList data, QString path = QString(), int flags = 0);

    TreeModelItem *itemAtPath(QString path);

    /** Remove an item with the given $path from the tree */
    bool removeItem(QString path);

    /**
     * Set the value of an item role by item path. This is recursive.
     *
     * @param path the absolute path of an item within a tree/subtree
     * @param value the value to set
     * @param role the model data role for which value is provided
     */
    void setItemRoleData(QString path, const QVariant &value, int role = Qt::EditRole);

    /**
     * Set the value of an item role by item reference. This is not recursive.
     *
     * @param item reference to the item to set
     * @param value the value to set
     * @param role the model data role for which value is provided
     */
    void setItemRoleData(TreeModelItem *item, const QVariant &value, int role = Qt::EditRole);

    /** Get the list of first level items of this tree. This doesn't include children. */
    QList<TreeModelItem *> items();

    /** Set columns data on a specific item with the provided $path */
    void setPathData(QString path, QVariantList data);

    /** Get the index of a role by its name. Useful before calling setItemRoleData */
    int roleIndex(QString role);

    /** @reimp */
    Q_INVOKABLE int rowCount(const QModelIndex & parent = QModelIndex()) const;

    /** @reimp */
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    /** @reimp */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    /** Helper method to print the tree in human readable form */
    void printTree(int tab = 0);

signals:
    void roleChanged(TreeModelItem *item, int role, const QVariant &value);

protected slots:
    void slotRoleChanged(TreeModelItem *item, int role, const QVariant &value);

protected:
    QHash<int, QByteArray> roleNames() const;
    int getItemInsertIndex(QString label);
    int getNodeInsertIndex(QString label);

protected:
    QStringList m_roles;
    bool m_sorting;
    bool m_checkable;
    QList<TreeModelItem *> m_items;
    QMap<QString, TreeModelItem *> m_itemsPathMap;
};

#endif // TREEMODEL_H
