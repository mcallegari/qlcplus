/*
  Q Light Controller Plus
  sceneeditor.h

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

#ifndef SCENEEDITOR_H
#define SCENEEDITOR_H

#include <QObject>
#include <QQuickView>

#include "scenevalue.h"

class Doc;
class Scene;

class SceneEditor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList fixtures READ fixtures NOTIFY fixturesChanged)
    Q_PROPERTY(QString sceneName READ sceneName WRITE setSceneName NOTIFY sceneNameChanged)

public:
    SceneEditor(Doc *doc, QObject *parent = 0);

    void setSceneID(quint32 id);

    QVariantList fixtures();

    QString sceneName() const;
    void setSceneName(QString sceneName);

private:
    void updateFixtureList();

signals:
    void fixturesChanged();
    void sceneNameChanged();

private:
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the Scene currently being edited */
    Scene *m_scene;
    /** A list of references to Fixtures used in $m_scene */
    QVariantList m_fixtures;
};

#endif // SCENEEDITOR_H
