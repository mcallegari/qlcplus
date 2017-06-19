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
#include "listmodel.h"
#include "scene.h"
#include "doc.h"

SceneEditor::SceneEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_scene(NULL)
    , m_sceneConsole(NULL)
    , m_source(NULL)
{
    m_view->rootContext()->setContextProperty("sceneEditor", this);
    m_source = new GenericDMXSource(m_doc);
    m_fixtureList = new ListModel(this);
    QStringList listRoles;
    listRoles << "fxRef" << "isSelected";
    m_fixtureList->setRoleNames(listRoles);
}

SceneEditor::~SceneEditor()
{
    m_view->rootContext()->setContextProperty("sceneEditor", NULL);
    QQuickItem *bottomPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("bottomPanelItem"));
    if (bottomPanel != NULL)
        bottomPanel->setProperty("visible", false);

    delete m_source;
    delete m_fixtureList;
}

void SceneEditor::setFunctionID(quint32 id)
{
    QQuickItem *bottomPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("bottomPanelItem"));

    if (id == Function::invalidId())
    {
        disconnect(m_scene, &Scene::valueChanged, this, &SceneEditor::slotSceneValueChanged);
        m_scene = NULL;
        m_source->unsetAll();
        m_source->setOutputEnabled(false);
        m_fixtureList->clear();
        m_fixtureIDs.clear();
        if (bottomPanel != NULL)
            bottomPanel->setProperty("visible", false);
        return;
    }
    m_scene = qobject_cast<Scene *>(m_doc->function(id));

    connect(m_scene, &Scene::valueChanged, this, &SceneEditor::slotSceneValueChanged);

    updateFixtureList();
    if (bottomPanel != NULL)
    {
        bottomPanel->setProperty("visible", true);
        bottomPanel->setProperty("editorSource", "qrc:/SceneFixtureConsole.qml");
    }
    FunctionEditor::setFunctionID(id);
}

QVariant SceneEditor::fixtureList() const
{
    return QVariant::fromValue(m_fixtureList);
}

void SceneEditor::setPreviewEnabled(bool enable)
{
    if (m_previewEnabled == enable)
        return;

    qDebug() << "[SceneEditor] set preview" << enable;

    if (enable == true)
    {
        foreach(SceneValue sv, m_scene->values())
            m_source->set(sv.fxi, sv.channel, sv.value);
    }
    else
        m_source->unsetAll();

    m_source->setOutputEnabled(enable);
    m_previewEnabled = enable;
    emit previewEnabledChanged(enable);
}

void SceneEditor::sceneConsoleLoaded(bool status)
{
    if (status == false)
    {
        m_sceneConsole = NULL;
        m_fxConsoleMap.clear();
    }
    else
    {
        m_sceneConsole = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("sceneFixtureConsole"));
    }
}

void SceneEditor::registerFixtureConsole(int index, QQuickItem *item)
{
    qDebug() << "[SceneEditor] Fixture console registered at index" << index;
    m_fxConsoleMap[index] = item;
}

void SceneEditor::unRegisterFixtureConsole(int index)
{
    qDebug() << "[SceneEditor] Fixture console unregistered at index" << index;
    m_fxConsoleMap.take(index);
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

void SceneEditor::setChannelValue(quint32 fxID, quint32 channel, uchar value)
{
    if (m_scene == NULL)
        return;

    bool blindMode = false;

    if (m_source->isOutputEnabled() == false)
        blindMode = true;

    m_scene->setValue(SceneValue(fxID, channel, value), blindMode, false);

    if (m_fixtureIDs.contains(fxID) == false)
        updateFixtureList();
    else
    {
        if (m_sceneConsole)
        {
            int fxIndex = m_fixtureIDs.indexOf(fxID);
            if (m_fxConsoleMap.contains(fxIndex))
            {
                QMetaObject::invokeMethod(m_fxConsoleMap[fxIndex], "setChannelValue",
                        Q_ARG(QVariant, channel),
                        Q_ARG(QVariant, value));
            }
        }
    }
    if (blindMode == false)
        m_source->set(fxID, channel, value);
}

void SceneEditor::slotSceneValueChanged(SceneValue scv)
{
    qDebug() << "slotSceneValueChanged---- " << scv;
    bool blindMode = false;

    if (m_source->isOutputEnabled() == false)
        blindMode = true;

    if (m_sceneConsole)
    {
        int fxIndex = m_fixtureIDs.indexOf(scv.fxi);
        if (m_fxConsoleMap.contains(fxIndex))
        {
            QMetaObject::invokeMethod(m_fxConsoleMap[fxIndex], "setChannelValue",
                    Q_ARG(QVariant, scv.channel),
                    Q_ARG(QVariant, scv.value));
        }
    }

    if (blindMode == false)
        m_source->set(scv.fxi, scv.channel, scv.value);
}

void SceneEditor::unsetChannel(quint32 fxID, quint32 channel)
{
    if (m_scene == NULL || m_fixtureIDs.contains(fxID) == false)
        return;

    m_scene->unsetValue(fxID, channel);
    if (m_source->isOutputEnabled() == true)
        m_source->unset(fxID, channel);
}

void SceneEditor::setFixtureSelection(quint32 fxID)
{
    if (m_scene == NULL || m_sceneConsole == NULL ||
        m_fixtureIDs.contains(fxID) == false)
            return;

    int fxIndex = m_fixtureIDs.indexOf(fxID);
    QMetaObject::invokeMethod(m_sceneConsole, "scrollToItem",
                              Q_ARG(QVariant, fxIndex));
}

void SceneEditor::updateFixtureList()
{
    if(m_scene == NULL)
        return;

    m_fixtureIDs.clear();
    m_fixtureList->clear();

    foreach(SceneValue sv, m_scene->values())
    {
        if (m_fixtureIDs.contains(sv.fxi) == false)
        {
            Fixture *fixture = m_doc->fixture(sv.fxi);
            if(fixture == NULL)
                continue;

            QVariantMap fxMap;
            fxMap.insert("fxRef", QVariant::fromValue(fixture));
            fxMap.insert("isSelected", false);
            m_fixtureList->addDataMap(fxMap);

            m_fixtureIDs.append(sv.fxi);
        }
    }
    emit fixtureListChanged();
}

