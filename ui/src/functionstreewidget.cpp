/*
  Q Light Controller Plus
  functionstreewidget.cpp

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

#include <QContextMenuEvent>
#include <QDebug>

#include "functionstreewidget.h"
#include "function.h"
#include "doc.h"

#define COL_NAME 0
#define COL_PATH 1

FunctionsTreeWidget::FunctionsTreeWidget(Doc *doc, QWidget *parent) :
    QTreeWidget(parent)
  , m_doc(doc)
{
    sortItems(COL_NAME, Qt::AscendingOrder);

    QTreeWidgetItem *root = invisibleRootItem();
    root->setFlags(root->flags() & ~Qt::ItemIsDropEnabled);

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                this, SLOT(slotItemChanged(QTreeWidgetItem*)));
}

void FunctionsTreeWidget::updateTree()
{
    blockSignals(true);

    clearTree();

    foreach (Function* function, m_doc->functions())
    {
        if (function->isVisible())
            updateFunctionItem(new QTreeWidgetItem(parentItem(function)), function);
    }

    blockSignals(false);
}

void FunctionsTreeWidget::clearTree()
{
    m_foldersMap.clear();
    clear();
}

void FunctionsTreeWidget::functionNameChanged(quint32 fid)
{
    blockSignals(true);
    Function* function = m_doc->function(fid);
    if (function == NULL)
    {
        blockSignals(false);
        return;
    }

    QTreeWidgetItem* item = functionItem(function);
    if (item != NULL)
        updateFunctionItem(item, function);

    blockSignals(false);
}

QTreeWidgetItem *FunctionsTreeWidget::addFunction(quint32 fid)
{
    Function* function = m_doc->function(fid);
    if (function == NULL || function->isVisible() == false)
        return NULL;

    QTreeWidgetItem* item = functionItem(function);
    if (item != NULL)
        return item;

    blockSignals(true);
    QTreeWidgetItem* parent = parentItem(function);
    item = new QTreeWidgetItem(parent);
    updateFunctionItem(item, function);
    if (parent != NULL)
        function->setPath(parent->text(COL_PATH));
    blockSignals(false);
    return item;
}

void FunctionsTreeWidget::updateFunctionItem(QTreeWidgetItem* item, const Function* function)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(function != NULL);
    item->setText(COL_NAME, function->name());
    item->setIcon(COL_NAME, function->getIcon());
    item->setData(COL_NAME, Qt::UserRole, function->id());
    item->setData(COL_NAME, Qt::UserRole + 1, function->type());
    item->setFlags(item->flags() & ~Qt::ItemIsDropEnabled);
}

QTreeWidgetItem* FunctionsTreeWidget::parentItem(const Function* function)
{
    Q_ASSERT(function != NULL);

    if (function->isVisible() == false)
        return NULL;

    QString basePath = Function::typeToString(function->type());
    if (m_foldersMap.contains(QString(basePath + "/")) == false)
    {
        // Parent item for the given type doesn't exist yet so create one
        QTreeWidgetItem* item = new QTreeWidgetItem(this);
        item->setText(COL_NAME, basePath);
        item->setIcon(COL_NAME, function->getIcon());
        item->setData(COL_NAME, Qt::UserRole, Function::invalidId());
        item->setData(COL_NAME, Qt::UserRole + 1, function->type());
        item->setText(COL_PATH, QString(basePath + "/"));
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);
        m_foldersMap[QString(basePath + "/")] = item;
    }

    QTreeWidgetItem *pItem = folderItem(function->path());

    if (pItem != NULL)
    {
        //qDebug() << "Found item for function:" << function->name() << ", path: " << function->path();
        return pItem;
    }

    return NULL;
}

quint32 FunctionsTreeWidget::itemFunctionId(const QTreeWidgetItem* item) const
{
    if (item == NULL || item->parent() == NULL)
        return Function::invalidId();
    else
    {
        QVariant var = item->data(COL_NAME, Qt::UserRole);
        if (var.isValid() == false)
            return Function::invalidId();

        return var.toUInt();
    }
}

QTreeWidgetItem* FunctionsTreeWidget::functionItem(const Function* function)
{
    Q_ASSERT(function != NULL);

    if (function->isVisible() == false)
        return NULL;

    QTreeWidgetItem* parent = parentItem(function);
    Q_ASSERT(parent != NULL);

    for (int i = 0; i < parent->childCount(); i++)
    {
        QTreeWidgetItem* item = parent->child(i);
        if (itemFunctionId(item) == function->id())
            return item;
    }

    return NULL;
}

/*********************************************************************
 * Tree folders
 *********************************************************************/

void FunctionsTreeWidget::addFolder()
{
    blockSignals(true);
    if (selectedItems().isEmpty())
    {
        blockSignals(false);
        return;
    }

    QTreeWidgetItem *item = selectedItems().first();
    if (item->text(COL_PATH).isEmpty())
        item = item->parent();

    int type = item->data(COL_NAME, Qt::UserRole + 1).toInt();

    QString fullPath = item->text(COL_PATH);
    if (fullPath.endsWith('/') == false)
        fullPath.append("/");

    QString newName = "New folder";

    int folderCount = 1;

    while (m_foldersMap.contains(fullPath + newName))
    {
        newName = "New Folder " + QString::number(folderCount++);
    }

    fullPath += newName;

    QTreeWidgetItem *folder = new QTreeWidgetItem(item);
    folder->setText(COL_NAME, newName);
    folder->setIcon(COL_NAME, QIcon(":/folder.png"));
    folder->setData(COL_NAME, Qt::UserRole, Function::invalidId());
    folder->setData(COL_NAME, Qt::UserRole + 1, type);
    folder->setText(COL_PATH, fullPath);
    folder->setFlags(folder->flags() | Qt::ItemIsDropEnabled | Qt::ItemIsEditable);

    m_foldersMap[fullPath] = folder;
    item->setExpanded(true);

    blockSignals(false);

    scrollToItem(folder, QAbstractItemView::PositionAtCenter);
}

void FunctionsTreeWidget::deleteFolder(QTreeWidgetItem *item)
{
    if (item == NULL)
        return;

    QList<QTreeWidgetItem*> childrenList;
    for (int i = 0; i < item->childCount(); i++)
        childrenList.append(item->child(i));

    QListIterator <QTreeWidgetItem*> it(childrenList);
    while (it.hasNext() == true)
    {
        QTreeWidgetItem *child = it.next();
        quint32 fid = child->data(COL_NAME, Qt::UserRole).toUInt();
        if (fid != Function::invalidId())
        {
            m_doc->deleteFunction(fid);
            delete child;
        }
        else
            deleteFolder(child);
    }

    QString name = item->text(COL_PATH);

    if (m_foldersMap.contains(name))
        m_foldersMap.remove(name);

    delete item;
}

QTreeWidgetItem *FunctionsTreeWidget::folderItem(QString name)
{
    if (selectedItems().count() > 0)
    {
        QString currFolder = selectedItems().first()->text(COL_PATH);
        if (currFolder.contains(name) && m_foldersMap.contains(currFolder))
            return m_foldersMap[currFolder];
    }

    if (m_foldersMap.contains(name))
        return m_foldersMap[name];

    if (name.endsWith('/'))
        return NULL;

    qDebug() << "Folder" << name << "doesn't exist. Creating it...";

    QTreeWidgetItem *parentNode = NULL;
    int type = Function::Undefined;
    QString fullPath;
    QStringList levelsList = name.split("/");
    foreach (QString level, levelsList)
    {
        // the first round is a category node. Just retrieve the item pointer
        // and the type, then skip it.
        if (fullPath.isEmpty())
        {
            if (m_foldersMap.contains(level + "/"))
            {
                parentNode = m_foldersMap[level + "/"];
                type = parentNode->data(COL_NAME, Qt::UserRole + 1).toInt();
            }
            fullPath = level;
            continue;
        }

        fullPath.append("/");
        fullPath.append(level);

        // create only missing levels
        if (m_foldersMap.contains(fullPath) == false)
        {
            QTreeWidgetItem *folder = new QTreeWidgetItem(parentNode);
            folder->setText(COL_NAME, level);
            folder->setIcon(COL_NAME, QIcon(":/folder.png"));
            folder->setData(COL_NAME, Qt::UserRole, Function::invalidId());
            folder->setData(COL_NAME, Qt::UserRole + 1, type);
            folder->setText(COL_PATH, fullPath);
            folder->setFlags(folder->flags() | Qt::ItemIsDropEnabled | Qt::ItemIsEditable);

            m_foldersMap[fullPath] = folder;
            parentNode = folder;
        }
        else
            parentNode = m_foldersMap[fullPath];
    }

    return m_foldersMap[name];
}

void FunctionsTreeWidget::slotItemChanged(QTreeWidgetItem *item)
{
    blockSignals(true);
    qDebug() << "[FunctionsTreeWidget] TREE item changed";
    if (item->text(COL_PATH).isEmpty())
    {
        blockSignals(false);
        return;
    }

    QTreeWidgetItem *parent = item->parent();
    if (parent != NULL)
    {
        QString fullPath = parent->text(COL_PATH);

        if (fullPath.endsWith('/') == false)
            fullPath.append("/");
        fullPath.append(item->text(COL_NAME));

        m_foldersMap.remove(item->text(COL_PATH));
        item->setText(COL_PATH, fullPath);
        m_foldersMap[fullPath] = item;
        slotUpdateChildrenPath(item);
    }
    blockSignals(false);
}

void FunctionsTreeWidget::slotUpdateChildrenPath(QTreeWidgetItem *root)
{
    if (root->childCount() == 0)
        return;
    for (int i = 0; i < root->childCount(); i++)
    {
        QTreeWidgetItem *child = root->child(i);

        // child can be a function node or another folder
        QString path = child->text(COL_PATH);
        if (path.isEmpty()) // function node
        {
            quint32 fid = child->data(COL_NAME, Qt::UserRole).toUInt();
            Function *func = m_doc->function(fid);
            if (func != NULL)
                func->setPath(root->text(COL_PATH));
        }
        else
        {
            slotItemChanged(child);
        }
    }
}

void FunctionsTreeWidget::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);

    m_draggedItems = selectedItems(); //itemAt(event->pos());
}


void FunctionsTreeWidget::dropEvent(QDropEvent *event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTreeWidgetItem *dropItem = itemAt(event->pos());
#else
    QTreeWidgetItem *dropItem = itemAt(event->position().toPoint());
#endif
    if (m_draggedItems.count() == 0 || dropItem == NULL)
        return;

    QVariant var = dropItem->data(COL_NAME, Qt::UserRole + 1);
    if (var.isValid() == false)
        return;

    int dropType = var.toInt();
    //QString folderName = dropItem->text(COL_PATH);

    foreach (QTreeWidgetItem *item, m_draggedItems)
    {
        quint32 dragFID = item->data(COL_NAME, Qt::UserRole).toUInt();
        Function *dragFunc = m_doc->function(dragFID);
        if (dragFunc != NULL && dragFunc->type() == dropType)
        {
            QTreeWidget::dropEvent(event);
            quint32 fid = item->data(COL_NAME, Qt::UserRole).toUInt();
            Function *func = m_doc->function(fid);
            if (func != NULL)
                func->setPath(dropItem->text(COL_PATH));
        }
        else
        {
            // m_draggedItem is a folder
            int dragType = item->data(COL_NAME, Qt::UserRole + 1).toInt();
            if (dragType == dropType)
                QTreeWidget::dropEvent(event);
            slotItemChanged(item);
        }
    }

    m_draggedItems.clear();
}
