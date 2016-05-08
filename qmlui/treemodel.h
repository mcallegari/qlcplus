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

class TreeModelItem;

class TreeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(TreeModel)
public:
    enum FixedRoles {
        LabelRole = Qt::UserRole + 1,
        PathRole,
        IsExpandedRole,
        IsSelectedRole,
        ItemsCountRole,
        HasChildrenRole,
        ChildrenModel,
        FixedRolesEnd
    };

    TreeModel(QObject *parent = 0);
    ~TreeModel();

    void clear();

    void setColumnNames(QStringList names);

    void enableSorting(bool enable);

    void addItem(QString label, QVariantList data, QString path = QString());

    Q_INVOKABLE int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void printTree(int tab = 0);

signals:
    void singleSelection(TreeModelItem *item);

protected slots:
    void setSingleSelection(TreeModelItem *item);

protected:
    int getItemIndex(QString label);
    int getFolderIndex(QString label);

protected:
    QStringList m_roles;
    QHash<int, QByteArray> roleNames() const;
    bool m_sorting;
    QList<TreeModelItem *> m_items;
    QMap<QString, TreeModelItem *> m_itemsPathMap;
};

#endif // TREEMODEL_H
