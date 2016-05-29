/*
  Q Light Controller Plus
  modelselector.h

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

#ifndef MODELSELECTOR_H
#define MODELSELECTOR_H

#include <QQuickItem>
#include <QObject>

typedef struct
{
    quint32 m_ID;
    QQuickItem *m_item;
} selectedItem;

class ModelSelector : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int nodesCount READ nodesCount NOTIFY nodesCountChanged)
    Q_PROPERTY(int itemsCount READ itemsCount NOTIFY itemsCountChanged)

public:
    ModelSelector(QObject *parent = 0);
    ~ModelSelector();

    /** Add an entry to the selected items list.
     *  If $multiSelection is false, every previous item in the list will be
     *  deselected. */
    Q_INVOKABLE void selectItem(quint32 id, QQuickItem *item, bool multiSelection);

    /** Restore the item pointer of an entry previously added to the selected
     *  items list. This is needed cause QML ListView destroys invisible items */
    Q_INVOKABLE void validateItem(quint32 id, QQuickItem *item);

    /** Invalidate the pointer of m_item in the selected items list.
     *  This is needed cause QML ListView destroys invisible items */
    Q_INVOKABLE void invalidateItem(QQuickItem *item);

    Q_INVOKABLE QVariantList itemsList();
    Q_INVOKABLE void resetSelection();

    int nodesCount() const;
    int itemsCount() const;

signals:
    void nodesCountChanged(int nodesCount);
    void itemsCountChanged(int itemsCount);

private:
    /** List of the currently selected items */
    QList <selectedItem> m_selectedItems;

    /** The number of nodes currently selected (e.g. folders) */
    int m_nodesCount;
    /** The number of items currently selected (e.g. Functions, Fixtures, etc..) */
    int m_itemsCount;
};

#endif // MODELSELECTOR_H
