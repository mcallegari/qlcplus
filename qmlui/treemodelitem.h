/*
  Q Light Controller Plus
  treemodelitem.h

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

#ifndef TREEMODELITEM_H
#define TREEMODELITEM_H

#include <QObject>
#include <QStringList>

class TreeModel;

class TreeModelItem: public QObject
{
    Q_OBJECT

public:
    TreeModelItem(QString label, QObject *parent = 0);
    ~TreeModelItem();

    /** Get/Set the item display label */
    QString label() const;
    void setLabel(QString label);

    /** Get/Set the item path, to be used to rebuild
      * the folder structure */
    QString path() const;
    void setPath(QString path);

    /** Set all the item flags at once */
    void setFlags(int flags);

    /** Get/Set a single flag of the item */
    void setFlag(int flag, bool enable);
    int flags() const;

    /** Get a user custom field with $index as variant */
    QVariant data(int index);

    /** Get all the user custom fields as a variant list */
    QVariantList data();

    /** Set the list of user custom fields */
    void setData(QVariantList data);

    /** Set the value for a specific role */
    void setRoleData(int index, QVariant value);

    /** Set the children column names, if this item has children */
    bool setChildrenColumns(QStringList columns);

    /** Add a child to this item. Typically this means the item is a folder */
    bool addChild(QString label, QVariantList data, bool sorting = false, QString path = QString(), int flags = 0);

    /** Return if the item has children. Typically for folders */
    bool hasChildren();

    /** Return a reference to the item's children */
    TreeModel *children();

    void printItem(int tab = 0);

private:
    QString m_label;
    QString m_path;
    int m_flags;
    QVariantList m_data;
    TreeModel *m_children;
};

#endif // TREEMODELITEM_H
