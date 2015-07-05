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

    QString label() const;
    void setLabel(QString label);

    void setData(QStringList data);

    QVariant data(int index);

    void setChildrenColumns(QStringList columns);

    void addChild(QString label, QStringList data, bool sorting = false, QString path = QString());

    bool hasChildren();

    TreeModel *children();

    void printItem(int tab = 0);

private:
    QString m_label;
    QStringList m_data;
    TreeModel *m_children;
};

#endif // TREEMODELITEM_H
