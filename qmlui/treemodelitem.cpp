#include <QDebug>

#include "treemodelitem.h"
#include "treemodel.h"

TreeModelItem::TreeModelItem(QString label)
{
    m_label = label;
    m_children = NULL;
}

TreeModelItem::~TreeModelItem()
{
    if (hasChildren())
    {
        delete m_children;
        m_children = NULL;
    }
}

QString TreeModelItem::label() const
{
    return m_label;
}

void TreeModelItem::setLabel(QString label)
{
    m_label = label;
}

void TreeModelItem::setData(QStringList data)
{
    m_data = data;
}

QVariant TreeModelItem::data(int index)
{
    qDebug() << "Getting data at" << index << label();
    if (index < 0 || index >= m_data.count())
        return QVariant();

    return m_data.at(index);
}

void TreeModelItem::setChildrenColumns(QStringList columns)
{
    if (m_children == NULL)
    {
        m_children = new TreeModel();
    }
    m_children->setColumnNames(columns);
}

void TreeModelItem::addChild(QString label, QStringList data, QString path)
{
    if (m_children == NULL)
    {
        m_children = new TreeModel();
    }
    m_children->addItem(label, data, path);
}

bool TreeModelItem::hasChildren()
{
    if (m_children != NULL)
        return true;

    return false;
}

TreeModel *TreeModelItem::children()
{
    return m_children;
}

void TreeModelItem::printItem(int tab)
{
    qDebug() << QString("%1%2").arg(QString(tab, QChar(0x20))).arg(label()) << m_data;
}


