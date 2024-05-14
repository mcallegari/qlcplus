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
    , m_checkable(false)
{

}

TreeModel::~TreeModel()
{
    //qDebug() << "!!! WARNING TreeModel destroyed WARNING !!!";
    clear();
}

QChar TreeModel::separator()
{
    return QLatin1Char('`');
}

void TreeModel::clear()
{
    int itemsCount = m_items.count();
    if (itemsCount == 0)
        return;

    for (int i = 0; i < itemsCount; i++)
    {
        TreeModelItem *item = m_items.takeLast();
        if (item->hasChildren())
            item->children()->clear();
        beginRemoveRows(QModelIndex(), 0, itemsCount - 1);
        delete item;
        endRemoveRows();
    }
    m_items.clear();
    m_itemsPathMap.clear();
}

void TreeModel::setColumnNames(QStringList names)
{
    m_roles = names;
}

void TreeModel::enableSorting(bool enable)
{
    m_sorting = enable;
}

void TreeModel::setCheckable(bool enable)
{
    m_checkable = enable;
}

void TreeModel::setSingleSelection(TreeModelItem *item)
{
    //bool parentSignalSent = false;
    //qDebug() << "Set single selection" << this;
    for (int i = 0; i < m_items.count(); i++)
    {
        TreeModelItem *target = m_items.at(i);

        if (target != item && target->flags() & Selected)
        {
            QModelIndex index = createIndex(i, 0, &i);
            target->setFlag(Selected, false);
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

TreeModelItem *TreeModel::addItem(QString label, QVariantList data, QString path, int flags)
{
    //qDebug() << "Adding item" << label << path;

    TreeModelItem *item = nullptr;

    // fewer roles are allowed, while exceeding are probably a mistake
    if (data.count() > m_roles.count())
        qDebug() << "Item roles exceeds tree roles!" << data.count() << m_roles.count();

    if (m_checkable)
        flags |= Checkable;

    if (path.isEmpty())
    {
        /* This is the case of a 'leaf' child */
        item = new TreeModelItem(label);
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        item->setData(data);
        item->setFlags(flags);
        if (flags & EmptyNode)
        {
            item->setPath(label);
            m_itemsPathMap[label] = item;
        }
        int addIndex = getItemInsertIndex(label);
        beginInsertRows(QModelIndex(), addIndex, addIndex);
        m_items.insert(addIndex, item);
        endInsertRows();
    }
    else
    {
        QStringList pathList = path.split(TreeModel::separator());
        if (m_itemsPathMap.contains(pathList.at(0)))
        {
            item = m_itemsPathMap.value(pathList.at(0), nullptr);
        }
        else
        {
            item = new TreeModelItem(pathList.at(0));
            item->setPath(pathList.at(0));
            item->setFlags(flags);
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
            if (item->setChildrenColumns(m_roles) == true)
            {
                connect(item->children(), SIGNAL(roleChanged(TreeModelItem*,int,const QVariant)),
                        this, SLOT(slotRoleChanged(TreeModelItem*,int,const QVariant&)));
                qDebug() << "Tree" << this << "connected to tree" << item->children();
            }

            int addIndex = getNodeInsertIndex(label);
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
            QString newPath = path.mid(path.indexOf(TreeModel::separator()) + 1);
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

TreeModelItem *TreeModel::itemAtPath(QString path)
{
    if (path.isEmpty())
        return nullptr;

    QStringList pathList = path.split(TreeModel::separator());

    if (pathList.count() == 1)
    {
        int index = 0;
        for (index = 0; index < m_items.count(); index++)
        {
            if (m_items.at(index)->label() == path)
                return m_items.at(index);
        }

        if (index == m_items.count())
            return nullptr;
    }

    TreeModelItem *item = m_itemsPathMap.value(pathList.at(0), nullptr);
    if (item == nullptr)
        return nullptr;

    QString subPath = path.mid(path.indexOf(TreeModel::separator()) + 1);
    return item->children()->itemAtPath(subPath);
}

bool TreeModel::removeItem(QString path)
{
    if (path.isEmpty())
        return false;

    qDebug() << "Removing item with path:" << path;

    QStringList pathList = path.split(TreeModel::separator());

    if (pathList.count() == 1)
    {
        int index = 0;
        for (index = 0; index < m_items.count(); index++)
        {
            if (m_items.at(index)->label() == path)
                break;
        }

        if (index == m_items.count())
            return false;

        beginRemoveRows(QModelIndex(), index, index);
        m_itemsPathMap.remove(path);
        delete m_items.at(index);
        m_items.removeAt(index);
        endRemoveRows();
    }
    else
    {
        TreeModelItem *item = m_itemsPathMap.value(pathList.at(0), nullptr);
        if (item == nullptr)
            return false;

        QString subPath = path.mid(path.indexOf(TreeModel::separator()) + 1);
        item->children()->removeItem(subPath);
    }

    return true;
}

void TreeModel::setItemRoleData(QString path, const QVariant &value, int role)
{
    if (path.isEmpty())
        return;

    //qDebug() << "Looking for item with path:" << path;

    QStringList pathList = path.split(TreeModel::separator());

    if (pathList.count() == 1)
    {
        int index = 0;
        for (index = 0; index < m_items.count(); index++)
        {
            if (m_items.at(index)->label() == pathList.at(0))
                break;
        }

        if (index == m_items.count())
            return;

        QModelIndex mIndex = createIndex(index, 0, &index);
        setData(mIndex, value, role);
    }
    else
    {
        TreeModelItem *item = m_itemsPathMap.value(pathList.at(0), nullptr);
        if (item == nullptr)
            return;

        QString subPath = path.mid(path.indexOf(TreeModel::separator()) + 1);
        item->children()->setItemRoleData(subPath, value, role);
    }
}

void TreeModel::setItemRoleData(TreeModelItem *item, const QVariant &value, int role)
{
    if (item == nullptr)
        return;

    int index = m_items.indexOf(item);
    if (index == -1)
        return;

    QModelIndex mIndex = createIndex(index, 0, &index);
    setData(mIndex, value, role);
}

QList<TreeModelItem *> TreeModel::items()
{
    return m_items;
}

void TreeModel::setPathData(QString path, QVariantList data)
{
    if (path.isEmpty())
        return;

    QStringList pathList = path.split(TreeModel::separator());
    TreeModelItem *item = m_itemsPathMap.value(pathList.at(0), nullptr);
    if (item == nullptr)
        return;

    if (pathList.count() == 1)
    {
        item->setData(data);
    }
    else if (item->hasChildren())
    {
        QString subPath = path.mid(path.indexOf(TreeModel::separator()) + 1);
        item->children()->setPathData(subPath, data);
    }
}

int TreeModel::roleIndex(QString role)
{
    return roleNames().key(role.toLatin1(), -1);
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
            return (item->flags() & Expanded) ? true : false;
        case IsSelectedRole:
            return (item->flags() & Selected) ? true : false;
        case IsCheckableRole:
            return (item->flags() & Checkable) ? true : false;
        case IsCheckedRole:
            return (item->flags() & Checked) ? true : false;
        case IsDraggableRole:
            return (item->flags() & Draggable) ? true : false;
        case ItemsCountRole:
            return m_items.count();
        case HasChildrenRole:
            return item->hasChildren() || (item->flags() & EmptyNode);
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

    //qDebug() << "Setting role" << role << "on row" << itemRow << "with value" << value;

    switch(role)
    {
        case LabelRole:
            item->setLabel(value.toString());
        break;
        case PathRole:
            item->setPath(value.toString());
        break;
        case IsExpandedRole:
            item->setFlag(Expanded, value.toBool());
        break;
        case IsSelectedRole:
        {
            if (value.toInt() > 0)
            {
                item->setFlag(Selected, true);
                if (value.toInt() == 1)
                    setSingleSelection(item);
            }
            else
                item->setFlag(Selected, false);
        }
        break;
        case IsCheckableRole:
            item->setFlag(Checkable, value.toBool());
        break;
        case IsCheckedRole:
            item->setFlag(Checked, value.toBool());
        break;
        case IsDraggableRole:
            item->setFlag(Draggable, value.toBool());
        break;
        default:
            item->setRoleData(role - FixedRolesEnd, value);
        break;
    }

    emit roleChanged(item, role, value);
    emit dataChanged(index, index, QVector<int>(1, role));

    return true;
}

void TreeModel::slotRoleChanged(TreeModelItem *item, int role, const QVariant &value)
{
    if (item == nullptr)
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

void TreeModel::printTree(int tab)
{
    for (TreeModelItem *item : m_items)
    {
        item->printItem(tab);
        if (item->hasChildren())
            item->children()->printTree(tab + 2);
    }
}

int TreeModel::getItemInsertIndex(QString label)
{
    if (m_sorting == true)
    {
        int index = 0;
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
    return rowCount();
}

int TreeModel::getNodeInsertIndex(QString label)
{
    if (m_sorting == true)
    {
        int index = 0;
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
    return rowCount();
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
    roles[IsCheckableRole] = "isCheckable";
    roles[IsCheckedRole] = "isChecked";
    roles[IsDraggableRole] = "isDraggable";
    roles[ItemsCountRole] = "itemsCount";
    roles[HasChildrenRole] = "hasChildren";
    roles[ChildrenModel] = "childrenModel";

    int roleStartIdx = FixedRolesEnd;
    for (int i = roleStartIdx, t = 0; i < roleStartIdx + m_roles.count(); i++, t++)
        roles[i] = m_roles.at(t).toLatin1();

    //qDebug() << "Returned" << roles.count() << "roles";

    return roles;
}

