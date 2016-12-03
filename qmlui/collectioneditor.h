/*
  Q Light Controller Plus
  collectioneditor.h

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

#ifndef COLLECTIONEDITOR_H
#define COLLECTIONEDITOR_H

#include "functioneditor.h"

class Collection;
class ListModel;

class CollectionEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QVariant functionsList READ functionsList NOTIFY functionsListChanged)

public:
    CollectionEditor(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Set the ID of the Collection being edited */
    void setFunctionID(quint32 ID);

    QVariant functionsList() const;

    Q_INVOKABLE bool addFunction(quint32 fid, int insertIndex = -1);

    Q_INVOKABLE bool moveFunction(quint32 fid, int newIndex);

    /** @reimp */
    void deleteItems(QVariantList list);

signals:
    void functionsListChanged();

protected:
    void updateFunctionsList();

private:
    /** Reference of the Collection currently being edited */
    Collection *m_collection;

    ListModel *m_functionsList;
};

#endif // COLLECTIONEDITOR_H
