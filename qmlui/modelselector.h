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

class ListModel;

class ModelSelector : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int itemsCount READ itemsCount NOTIFY itemsCountChanged)

public:
    ModelSelector(QObject *parent = 0);
    ~ModelSelector();

    /** Add an entry to the selected items list.
     *  If $multiSelection is false, every previous item in the list will be
     *  deselected. */
    Q_INVOKABLE void selectItem(int index, ListModel *model, int keyModifiers);

    /** Reset the currently active selection */
    Q_INVOKABLE void resetSelection(ListModel *model);

    /** Return the list of the currently selected
     *  item indices, as list of QVariant */
    Q_INVOKABLE QVariantList itemsList();

    /** Return the number of items currently selected */
    int itemsCount() const;

private:
    void selectSingleItem(int index, ListModel *model);

signals:
    void itemsCountChanged(int itemsCount);
    void itemSelectionChanged(int itemIndex, bool selected);

private:
    /** List of the currently selected item indices */
    QList <quint32> m_selectedIndices;

    /** The rpviously selected item index */
    int m_previousIndex;

    /** The number of items currently selected (e.g. Functions, Fixtures, etc..) */
    int m_itemsCount;
};

#endif // MODELSELECTOR_H
