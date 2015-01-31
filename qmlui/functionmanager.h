/*
  Q Light Controller Plus
  functionmanager.h

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

#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include <QStringList>
#include <QQuickView>
#include <QObject>

#include "treemodel.h"

class Doc;

class FunctionManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant functionsList READ functionsList NOTIFY functionsListChanged)

public:
    FunctionManager(QQuickView *view, Doc *doc, QObject *parent = 0);

    QVariant functionsList();

signals:
    void functionsListChanged();

protected slots:
    void slotDocLoaded();

private:
    QQuickView *m_view;
    Doc *m_doc;
    TreeModel *m_functionTree;
};

#endif // FUNCTIONMANAGER_H
