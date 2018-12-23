/*
  Q Light Controller Plus
  fixtureeditor.h

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

#ifndef FIXTUREEDITOR_H
#define FIXTUREEDITOR_H

#include <QQuickView>

class Doc;
class QLCFixtureDef;

class FixtureEditor : public QObject
{
    Q_OBJECT

public:
    FixtureEditor(QQuickView *view, Doc *doc, QObject *parent = nullptr);
    ~FixtureEditor();

    void createDefinition();

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Map of index / references to the definitions being edited */
    QMap<int, QLCFixtureDef *>m_fixtures;
};

#endif // FIXTUREEDITOR_H
