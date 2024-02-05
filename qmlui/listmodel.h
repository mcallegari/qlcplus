/*
  Q Light Controller Plus
  listmodel.h

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

#ifndef LISTMODEL_H
#define LISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>

#define SEARCH_MIN_CHARS    3

class ListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ListModel)
public:
    ListModel(QObject *parent = nullptr);
    ~ListModel();

    void clear();

    void setRoleNames(QStringList names);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    QVariant data(const QModelIndex & index, QString role) const;

    QVariant itemAt(int index) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    bool setDataWithRole(const QModelIndex &index, QString roleName, const QVariant &value);

    void addDataMap(QVariantMap data);

    void setDataMap(const QModelIndex &index, QVariantMap data);

protected:
    QStringList m_roles;
    QHash<int, QByteArray> roleNames() const;
    QVariantList m_data;
};

#endif // LISTMODEL_H
