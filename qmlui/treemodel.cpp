/*
  Q Light Controller Plus
  treemodel.cpp

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

#include <QDebug>
#include <QQmlEngine>

#include "treemodel.h"
#include "treemodelitem.h"

TreeModel::TreeModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_sorting(false)
{

}

TreeModel::~TreeModel()
{
    //qDebug() << "!!! WARNING TreeModel destroyed WARNING !!!";
    clear();
}

void TreeModel::clear()
{
    int itemsCount = m_items.count();
    if (itemsCount == 0)
        return;

    beginRemoveRows(QModelIndex(), 0, itemsCount - 1);
    for (int i = 0; i < itemsCount; i++)
    {
        TreeModelItem *item = m_items.takeLast();
        delete item;
    }
    m_items.clear();
    m_itemsPathMap.clear();
    endRemoveRows();
}

void TreeModel::setColumnNames(QStringList names)
{
    m_roles = names;
}

void TreeModel::enableSorting(bool enable)
{
    m_sorting = enable;
}

void TreeModel::addItem(QString label, QVariantList data, QString path)
{
    qDebug() << "Adding item" << label << path;

    if (data.count() != m_roles.count())
        qDebug() << "Adding an item with a different number of roles" << data.count() << m_roles.count();

    if (path.isEmpty())
    {
        TreeModelItem *item = new TreeModelItem(label);
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        item->setData(data);
        int addIndex = getItemIndex(label);
        beginInsertRows(QModelIndex(), addIndex, addIndex);
        m_items.insert(addIndex, item);
        endInsertRows();
    }
    else
    {
        TreeModelItem *item = NULL;
        QStringList pathList = path.split("/");
        if (m_itemsPathMap.contains(pathList.at(0)))
        {
            item = m_itemsPathMap[pathList.at(0)];
        }
        else
        {
            item = new TreeModelItem(pathList.at(0));
            item->setPath(pathList.at(0));
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
            item->setChildrenColumns(m_roles);
            int addIndex = getFolderIndex(label);
            beginInsertRows(QModelIndex(), addIndex, addIndex);
            m_items.insert(addIndex, item);
            endInsertRows();
            m_itemsPathMap[pathList.at(0)] = item;
        }

        if (pathList.count() == 1)
            item->addChild(label, data, m_sorting);
        else
        {
            QString newPath = path.mid(path.indexOf("/") + 1);
            item->addChild(label, data, m_sorting, newPath);
        }
    }
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_items.count();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_items.count())
        return QVariant();

    int itemRow = index.row();

    if (role == LabelRole)
        return m_items.at(itemRow)->label();
    else if (role == PathRole)
        return m_items.at(itemRow)->path();
    else if (role == ItemsCountRole)
        return m_items.count();
    else if (role == HasChildrenRole)
        return m_items.at(itemRow)->hasChildren();
    else if (role == ChildrenModel)
        return QVariant::fromValue(m_items.at(itemRow)->children());

    return m_items.at(index.row())->data(role - FixedRolesEnd);
}

void TreeModel::printTree(int tab)
{
    foreach(TreeModelItem *item, m_items)
    {
        item->printItem(tab);
        if (item->hasChildren())
            item->children()->printTree(tab + 2);
    }
}

int TreeModel::getItemIndex(QString label)
{
    int index = rowCount();
    if (m_sorting == true)
    {
        index = 0;
        for (int i = 0; i < m_items.count(); i++)
        {
            if (m_items.at(i)->hasChildren() == false)
            {
                if (QString::localeAwareCompare(m_items.at(i)->label(), label) > 0)
                    return index;
            }
            index++;
        }
    }
    return index;
}

int TreeModel::getFolderIndex(QString label)
{
    int index = rowCount();
    if (m_sorting == true)
    {
        index = 0;
        for (int i = 0; i < m_items.count(); i++)
        {
            if (m_items.at(i)->hasChildren() == true)
            {
                if (QString::localeAwareCompare(m_items.at(i)->label(), label) > 0)
                    return index;
                index++;
            }
        }
    }
    return index;
}

QHash<int, QByteArray> TreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LabelRole] = "label";
    roles[PathRole] = "path";
    roles[ItemsCountRole] = "itemsCount";
    roles[HasChildrenRole] = "hasChildren";
    roles[ChildrenModel] = "childrenModel";

    int roleStartIdx = FixedRolesEnd;
    for (int i = roleStartIdx, t = 0; i < roleStartIdx + m_roles.count(); i++, t++)
        roles[i] = m_roles.at(t).toLatin1();

    //qDebug() << "Returned" << roles.count() << "roles";

    return roles;
}

