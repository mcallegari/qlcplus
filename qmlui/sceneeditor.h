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

#include "functioneditor.h"
#include "scenevalue.h"

class Doc;
class Scene;
class ListModel;
class GenericDMXSource;

class SceneEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QVariant fixtureList READ fixtureList NOTIFY fixtureListChanged)
    Q_PROPERTY(QVariant componentList READ componentList NOTIFY componentListChanged)
    Q_PROPERTY(int selectedChannelCount READ selectedChannelCount NOTIFY selectedChannelCountChanged)

public:
    SceneEditor(QQuickView *view, Doc *doc, QObject *parent = nullptr);
    ~SceneEditor();

    /** Set the ID of the Scene to edit */
    void setFunctionID(quint32 id);

    /** Return a QVariant list of references to the Fixtures
     *  involved in the Scene editing */
    QVariant fixtureList() const;

    /** Return a QVariant list of all the references to components
     *  involved in the Scene editing */
    QVariant componentList() const;

    /** Enable/disable the preview of the current Scene.
     *  In this editor, the preview is done with a GenericDMXSource */
    void setPreviewEnabled(bool enable);

    /** Method called by QML to inform the SceneEditor that
     *  SceneFixtureConsole has been loaded/unloaded. */
    Q_INVOKABLE void sceneConsoleLoaded(bool status);

    Q_INVOKABLE void registerFixtureConsole(int index, QQuickItem *item);
    Q_INVOKABLE void unRegisterFixtureConsole(int index);

    /** QML invokable method that returns if the Scene has the
     *  requested $fixture's $channel */
    Q_INVOKABLE bool hasChannel(quint32 fxID, quint32 channel);

    /** QML invokable method that returns the value of the
     *  requested $fixture's $channel */
    Q_INVOKABLE double channelValue(quint32 fxID, quint32 channel);

    /** Remove a channel with the provided $fxID and $channel
     *  from the Scene currently being edited */
    Q_INVOKABLE void unsetChannel(quint32 fxID, quint32 channel);

    /** Set a Fixture selection by ID to scroll the UI to the
     *  related FixtureConsole item */
    Q_INVOKABLE void setFixtureSelection(quint32 fxID);

    /** Add/remove a channel from the clipboard selection, to allow
     *  the paste-to-all functionality */
    Q_INVOKABLE void setChannelSelection(quint32 fxID, quint32 channel, bool selected);

    /** Return the number of channels currently selected for paste-to-all */
    int selectedChannelCount();

    /** Add a component with of given type
     *  e.g. FixtureGroup, Fixture, Palette */
    Q_INVOKABLE void addComponent(int type, quint32 id);

    /** Paste all the values selected with setChannelSelection
     *  to all the fixture of the same type and mode */
    Q_INVOKABLE void pasteToAllFixtureSameType();

    /** @reimp */
    void deleteItems(QVariantList list);

protected slots:
    void slotSceneValueChanged(SceneValue scv);
    void slotAliasChanged();

private:
    void addFixtureToList(quint32 fid);
    void updateLists();
    void setCacheChannelValue(SceneValue scv);
    void cacheChannelValues();

signals:
    void fixtureListChanged();
    void componentListChanged();
    void selectedChannelCountChanged();

private:
    /** Reference of the Scene currently being edited */
    Scene *m_scene;

    /** A list of the $m_scene Fixture IDs for fast lookup */
    QList<quint32> m_fixtureIDs;

    /** A QML-readable list of references to Fixtures used in $m_scene */
    ListModel *m_fixtureList;

    /** A QML-readable list of all the components used by the Scene
     *  (Fixture groups, Fixtures, Palettes) */
    ListModel *m_componentList;

    /** A reference to the SceneFixtureConsole when loaded */
    QQuickItem *m_sceneConsole;

    /** Keep a track of the registered Fixture consoles in a Scene Console,
     *  to rapidly set a channel value */
    QMap<int, QQuickItem *> m_fxConsoleMap;

    /** Pre-cache initial channel values including palettes.
     *  arranged as <fixture ID, channel values array> */
    QMap<quint32, QByteArray> m_channelsCache;

    QList<SceneValue> m_selectedChannels;

    /** Reference to a DMX source used to edit a Scene */
    GenericDMXSource *m_source;
};

#endif // SCENEEDITOR_H
