
#include <QDebug>

#include "treemodel.h"
#include "treemodelitem.h"

TreeModel::TreeModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

TreeModel::~TreeModel()
{
    clear();
}

void TreeModel::clear()
{
    int itemsCount = m_items.count();
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

void TreeModel::addItem(QString label, QStringList data, QString path)
{
    qDebug() << "Adding item" << label << path;

    if (data.count() != m_roles.count())
        qDebug() << "Adding an item with a different number of roles" << data.count() << m_roles.count();

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    if (path.isEmpty())
    {
        TreeModelItem *item = new TreeModelItem(label);
        item->setData(data);
        m_items.append(item);
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
            item->setChildrenColumns(m_roles);
            m_items.append(item);
            m_itemsPathMap[pathList.at(0)] = item;
        }

        if (pathList.count() == 1)
            item->addChild(label, data);
        else
        {
            QString newPath = path.mid(path.indexOf("/") + 1);
            item->addChild(label, data, newPath);
        }
    }
    endInsertRows();
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

QHash<int, QByteArray> TreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LabelRole] = "label";
    roles[ItemsCountRole] = "itemsCount";
    roles[HasChildrenRole] = "hasChildren";
    roles[ChildrenModel] = "childrenModel";

    int roleStartIdx = FixedRolesEnd;
    for (int i = roleStartIdx, t = 0; i < roleStartIdx + m_roles.count(); i++, t++)
        roles[i] = m_roles.at(t).toLatin1();

    qDebug() << "Returned" << roles.count() << "roles";

    return roles;
}

