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

TreeModelItem *TreeModel::addItem(QString label, QVariantList data, QString path, int flags)
{
    //qDebug() << "Adding item" << label << path;

    TreeModelItem *item = NULL;

    if (data.count() != m_roles.count())
        qDebug() << "Adding an item with a different number of roles" << data.count() << m_roles.count();

    if (path.isEmpty())
    {
        item = new TreeModelItem(label);
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        item->setData(data);
        if (flags & Expanded)
            item->setExpanded(true);
        if (flags & Checked)
            item->setChecked(true);
        int addIndex = getItemIndex(label);
        beginInsertRows(QModelIndex(), addIndex, addIndex);
        m_items.insert(addIndex, item);
        endInsertRows();
    }
    else
    {
        QStringList pathList = path.split("/");
        if (m_itemsPathMap.contains(pathList.at(0)))
        {
            item = m_itemsPathMap[pathList.at(0)];
        }
        else
        {
            item = new TreeModelItem(pathList.at(0));
            item->setPath(pathList.at(0));
            if (flags & Expanded)
                item->setExpanded(true);
            if (flags & Checked)
                item->setChecked(true);
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
            if (item->setChildrenColumns(m_roles) == true)
            {
                connect(item->children(), SIGNAL(roleChanged(TreeModelItem*,int,const QVariant)),
                        this, SLOT(slotRoleChanged(TreeModelItem*,int,const QVariant&)));
                qDebug() << "Tree" << this << "connected to tree" << item->children();
            }

            int addIndex = getFolderIndex(label);
            beginInsertRows(QModelIndex(), addIndex, addIndex);
            m_items.insert(addIndex, item);
            endInsertRows();
            m_itemsPathMap[pathList.at(0)] = item;
        }

        if (pathList.count() == 1)
        {
            if (item->addChild(label, data, m_sorting, "", flags) == true)
            {
                connect(item->children(), SIGNAL(roleChanged(TreeModelItem*,int,const QVariant&)),
                        this, SLOT(slotRoleChanged(TreeModelItem*,int,const QVariant&)));
                qDebug() << "Tree" << this << "connected to tree" << item->children();
            }
        }
        else
        {
            QString newPath = path.mid(path.indexOf("/") + 1);
            if (item->addChild(label, data, m_sorting, newPath, flags) == true)
            {
                connect(item->children(), SIGNAL(roleChanged(TreeModelItem*,int,const QVariant&)),
                        this, SLOT(slotRoleChanged(TreeModelItem*,int,const QVariant&)));
                qDebug() << "Tree" << this << "connected to tree" << item->children();
            }
        }
    }

    return item;
}

void TreeModel::setPathData(QString path, QVariantList data)
{
    if (path.isEmpty())
        return;

    QStringList pathList = path.split("/");
    if (m_itemsPathMap.contains(pathList.at(0)))
    {
        TreeModelItem *item = m_itemsPathMap[pathList.at(0)];
        if (pathList.count() == 1)
        {
            item->setData(data);
        }
        else if (item->hasChildren())
        {
            QString subPath = path.mid(path.indexOf("/") + 1);
            item->children()->setPathData(subPath, data);
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
    if (itemRow < 0 || itemRow >= m_items.count())
        return false;

    TreeModelItem *item = m_items.at(itemRow);

    switch(role)
    {
        case LabelRole:
            return item->label();
        case PathRole:
            return item->path();
        case IsExpandedRole:
            return item->isExpanded();
        case IsSelectedRole:
            return item->isSelected();
        case IsCheckedRole:
            return item->isChecked();
        case ItemsCountRole:
            return m_items.count();
        case HasChildrenRole:
            return item->hasChildren();
        case ChildrenModel:
            return QVariant::fromValue(item->children());
        default:
            return m_items.at(index.row())->data(role - FixedRolesEnd);
    }
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int itemRow = index.row();
    if (itemRow < 0 || itemRow >= m_items.count())
        return false;

    TreeModelItem *item = m_items.at(itemRow);

    switch(role)
    {
        case LabelRole:
            item->setLabel(value.toString());
        break;
        case PathRole:
            item->setPath(value.toString());
        break;
        case IsExpandedRole:
            item->setExpanded(value.toBool());
        break;
        case IsSelectedRole:
        {
            if (value.toInt() > 0)
            {
                item->setSelected(true);
                if (value.toInt() == 1)
                    setSingleSelection(item);
            }
            else
                item->setSelected(false);
        }
        break;
        case IsCheckedRole:
            item->setChecked(value.toBool());
        break;
        default:
            return false;
        break;
    }

    emit roleChanged(item, role, value);
    emit dataChanged(index, index, QVector<int>(1, role));

    return true;
}

void TreeModel::slotRoleChanged(TreeModelItem *item, int role, const QVariant &value)
{
    if (item == NULL)
        return;

    switch(role)
    {
        case IsSelectedRole:
        {
            if (value.toInt() == 1)
                setSingleSelection(item);
        }
        break;
    }

    if (sender() != this)
        emit roleChanged(item, role, value);
}

void TreeModel::setSingleSelection(TreeModelItem *item)
{
    //bool parentSignalSent = false;
    //qDebug() << "Set single selection" << this;
    for (int i = 0; i < m_items.count(); i++)
    {
        TreeModelItem *target = m_items.at(i);

        if (target != item && target->isSelected())
        {
            QModelIndex index = createIndex(i, 0, &i);
            target->setSelected(false);
            emit dataChanged(index, index, QVector<int>(1, IsSelectedRole));
        }

        if (target->hasChildren())
        {
            // if this slot has been called from self or a parent node,
            // then walk down the children path
            if (sender() != target->children())
                target->children()->setSingleSelection(item);
        }
    }
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
    roles[IsExpandedRole] = "isExpanded";
    // The isSelected role is tricky.
    // It always returns true/false through the data() method but
    // to set it via setData(), an integer has to be passed, to distinguish
    // between 3 cases:
    // 0: item de-selection
    // 1: item single (exclusive) selection
    // 2: item multiple selection (when Ctrl-click an item)
    roles[IsSelectedRole] = "isSelected";
    roles[IsCheckedRole] = "isChecked";
    roles[ItemsCountRole] = "itemsCount";
    roles[HasChildrenRole] = "hasChildren";
    roles[ChildrenModel] = "childrenModel";

    int roleStartIdx = FixedRolesEnd;
    for (int i = roleStartIdx, t = 0; i < roleStartIdx + m_roles.count(); i++, t++)
        roles[i] = m_roles.at(t).toLatin1();

    //qDebug() << "Returned" << roles.count() << "roles";

    return roles;
}

