/*
  Q Light Controller Plus
  sceneeditor.cpp

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

#include "sceneeditor.h"
#include "scenevalue.h"
#include "scene.h"
#include "doc.h"

SceneEditor::SceneEditor(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_scene(NULL)
{

}

void SceneEditor::setSceneID(quint32 id)
{
    m_scene = qobject_cast<Scene *>(m_doc->function(id));

    updateFixtureList();
}

QVariantList SceneEditor::fixtures()
{
    return m_fixtures;
}

QString SceneEditor::sceneName() const
{
    if (m_scene == NULL)
        return "";
    return m_scene->name();
}

void SceneEditor::setSceneName(QString sceneName)
{
    if (m_scene == NULL)
        return;

    if (m_scene->name() == sceneName)
        return;

    m_scene->setName(sceneName);
    emit sceneNameChanged();
}

void SceneEditor::updateFixtureList()
{
    if(m_scene == NULL)
        return;

    QList<quint32> fxIDs;
    m_fixtures.clear();

    foreach(SceneValue value, m_scene->values())
    {
        if (fxIDs.contains(value.fxi) == false)
        {
            Fixture *fixture = m_doc->fixture(value.fxi);
            if(fixture == NULL)
                continue;
            m_fixtures.append(QVariant::fromValue(fixture));
            fxIDs.append(value.fxi);
        }
    }
    emit fixturesChanged();
}

