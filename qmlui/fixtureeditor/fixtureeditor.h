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
class EditorView;

class FixtureEditor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList editorsList READ editorsList NOTIFY editorsListChanged)

public:
    FixtureEditor(QQuickView *view, Doc *doc, QObject *parent = nullptr);
    ~FixtureEditor();

    /** Create a new editor and an empty fixture definition */
    void createDefinition();

    /** Edit an existing fixture definition */
    void editDefinition(QString manufacturer, QString model);

    /** Returns a list of the created editors */
    QVariantList editorsList() const;

signals:
    void editorsListChanged();

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** The last assigned editor ID */
    int m_lastId;
    /** Map of id / references to the open editors */
    QMap<int, EditorView *>m_editors;
};

#endif // FIXTUREEDITOR_H
