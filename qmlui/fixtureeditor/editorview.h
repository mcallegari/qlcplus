/*
  Q Light Controller Plus
  editorview.h

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

#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QQuickView>

class QLCFixtureDef;

class EditorView : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer NOTIFY manufacturerChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)

public:
    EditorView(QQuickView *view, QLCFixtureDef* fixtureDef, QObject *parent = nullptr);
    ~EditorView();

    /** Get/Set the fixture manufacturer */
    QString manufacturer() const;
    void setManufacturer(QString manufacturer);

    /** Get/Set the fixture model */
    QString model() const;
    void setModel(QString model);

signals:
    void manufacturerChanged(QString manufacturer);
    void modelChanged(QString model);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the definition being edited */
    QLCFixtureDef *m_fixtureDef;
};

#endif // FIXTUREEDITOR_H
