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

#include <QDebug>

#include "genericdmxsource.h"
#include "sceneeditor.h"
#include "scenevalue.h"
#include "scene.h"
#include "doc.h"

SceneEditor::SceneEditor(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_scene(NULL)
    , m_source(NULL)
{
    m_source = new GenericDMXSource(m_doc);
}

void SceneEditor::setSceneID(quint32 id)
{
    QQuickItem *bottomPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("bottomPanelItem"));

    if (id == Function::invalidId())
    {
        m_scene = NULL;
        m_source->unsetAll();
        //m_source->setOutputEnabled(false);
        m_fixtures.clear();
        if (bottomPanel != NULL)
            bottomPanel->setProperty("visible", false);
        return;
    }
    m_scene = qobject_cast<Scene *>(m_doc->function(id));

    updateFixtureList();
    if (bottomPanel != NULL)
    {
        bottomPanel->setProperty("visible", true);
        bottomPanel->setProperty("editorSource", "qrc:/SceneFixtureConsole.qml");
    }
}

quint32 SceneEditor::sceneID() const
{
    if (m_scene != NULL)
        return m_scene->id();

    return Function::invalidId();
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

void SceneEditor::setPreview(bool enable)
{
    qDebug() << "[SceneEditor] set preview" << enable;
    if (enable == true)
    {
        foreach(SceneValue sv, m_scene->values())
            m_source->set(sv.fxi, sv.channel, sv.value);
    }
    else
        m_source->unsetAll();

    m_source->setOutputEnabled(enable);
}

bool SceneEditor::hasChannel(quint32 fxID, quint32 channel)
{
    if (m_scene == NULL)
        return false;

    return m_scene->checkValue(SceneValue(fxID, channel));
}

double SceneEditor::channelValue(quint32 fxID, quint32 channel)
{
    if (m_scene == NULL)
        return 0;

    return (double)m_scene->value(fxID, channel);
}

void SceneEditor::updateFixtureList()
{
    if(m_scene == NULL)
        return;

    QList<quint32> fxIDs;
    m_fixtures.clear();

    foreach(SceneValue sv, m_scene->values())
    {
        if (fxIDs.contains(sv.fxi) == false)
        {
            Fixture *fixture = m_doc->fixture(sv.fxi);
            if(fixture == NULL)
                continue;
            m_fixtures.append(QVariant::fromValue(fixture));
            fxIDs.append(sv.fxi);
        }
    }
    emit fixturesChanged();
}

