/*
  Q Light Controller Plus
  listmodel.cpp

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

#include "listmodel.h"

ListModel::ListModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

ListModel::~ListModel()
{
    clear();
}

void ListModel::clear()
{
    int itemsCount = m_data.count();
    if (itemsCount == 0)
        return;

    beginRemoveRows(QModelIndex(), 0, itemsCount - 1);
    for (int i = 0; i < itemsCount; i++)
        m_data.takeLast();

    m_data.clear();
    endRemoveRows();
}

int ListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data.count();
}

QVariant ListModel::data(const QModelIndex &index, int role) const
{
    int itemRow = index.row();
    if (itemRow < 0 || itemRow >= m_data.count())
        return QVariant();

    if (role < Qt::UserRole + 1 || role > Qt::UserRole + 1 + m_roles.count())
        return QVariant();

    QString roleName = m_roles.at(role - Qt::UserRole - 1);
    QVariantMap dataMap = m_data.at(itemRow).toMap();

    return dataMap[roleName];
}

QVariant ListModel::data(const QModelIndex &index, QString role) const
{
    int itemRow = index.row();
    if (itemRow < 0 || itemRow >= m_data.count() || m_roles.contains(role) == false)
        return QVariant();

    QVariantMap dataMap = m_data.at(itemRow).toMap();

    return dataMap[role];
}

QVariant ListModel::itemAt(int index) const
{
    if (index < 0 || index >= m_data.count())
        return QVariant();

    return m_data.at(index);
}

bool ListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int itemRow = index.row();
    if (itemRow < 0 || itemRow >= m_data.count())
        return false;

    if (role < Qt::UserRole + 1 || role > Qt::UserRole + 1 + m_roles.count())
        return false;

    QString roleName = m_roles.at(role - Qt::UserRole - 1);

    QVariantMap dataMap = m_data.at(itemRow).toMap();
    dataMap[roleName] = value;
    m_data[itemRow] = dataMap;
    emit dataChanged(index, index, QVector<int>(1, role));

    return true;
}

bool ListModel::setDataWithRole(const QModelIndex &index, QString roleName, const QVariant &value)
{
    int itemRow = index.row();
    if (itemRow < 0 || itemRow >= m_data.count() || m_roles.contains(roleName) == false)
        return false;

    QVariantMap dataMap = m_data.at(itemRow).toMap();
    int role = Qt::UserRole + 1 + m_roles.indexOf(roleName);
    dataMap[roleName] = value;
    m_data[itemRow] = dataMap;
    emit dataChanged(index, index, QVector<int>(1, role));

    return true;
}

void ListModel::addDataMap(QVariantMap data)
{
    int addIndex = m_data.count();
    beginInsertRows(QModelIndex(), addIndex, addIndex);
    m_data.append(data);
    endInsertRows();
}

void ListModel::setDataMap(const QModelIndex &index, QVariantMap data)
{
    int itemRow = index.row();
    if (itemRow >= m_data.count())
        return;

    m_data[itemRow] = data;

    emit dataChanged(index, index);
}

QHash<int, QByteArray> ListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    for (int i = 0; i < m_roles.count(); i++)
        roles[Qt::UserRole + 1 + i] = m_roles.at(i).toLatin1();

    return roles;
}

void ListModel::setRoleNames(QStringList names)
{
    m_roles = names;
}
