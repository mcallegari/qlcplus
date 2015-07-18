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

#include <QQuickView>
#include <QQuickItem>
#include <QObject>

#include "scenevalue.h"

class Doc;
class Scene;
class GenericDMXSource;

class SceneEditor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList fixtures READ fixtures NOTIFY fixturesChanged)
    Q_PROPERTY(QString sceneName READ sceneName WRITE setSceneName NOTIFY sceneNameChanged)

public:
    SceneEditor(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Set the ID of the Scene to edit */
    void setSceneID(quint32 id);
    /** Return the ID of the current Scene being edited */
    quint32 sceneID() const;

    /** Return a QVariant list of references to the Fixtures
     *  involved in the Scene editing */
    QVariantList fixtures();

    /** Return the name of the currently edited Scene */
    QString sceneName() const;

    /** Set the name of the currently edited Scene */
    void setSceneName(QString sceneName);

    /** Enable/disable the preview of the current Scene.
     *  In this editor, the preview is done with a GenericDMXSource */
    void setPreview(bool enable);

    /** Method called by QML to inform the SceneEditor that
     *  SceneFixtureConsole has been loaded/unloaded. */
    Q_INVOKABLE void sceneConsoleLoaded(bool status);

    /** QML invokable method that returns if the Scene has the
     *  requested $fixture's $channel */
    Q_INVOKABLE bool hasChannel(quint32 fxID, quint32 channel);

    /** QML invokable method that returns the values of the
     *  requested $fixture's $channel */
    Q_INVOKABLE double channelValue(quint32 fxID, quint32 channel);

    Q_INVOKABLE void setChannelValue(quint32 fxID, quint32 channel, uchar value);

    Q_INVOKABLE void unsetChannel(quint32 fxID, quint32 channel);

    Q_INVOKABLE void setFixtureSelection(quint32 fxID);

private:
    void updateFixtureList();

signals:
    void fixturesChanged();
    void sceneNameChanged();

private:
    /** Reference of the QML view */
    QQuickView *m_view;
    /** Reference of the project workspace */
    Doc *m_doc;
    /** Reference of the Scene currently being edited */
    Scene *m_scene;
    /** A list of the $m_scene Fixture IDs for fast lookup */
    QList<quint32> m_fixtureIDs;
    /** A QML-readable list of references to Fixtures used in $m_scene */
    QVariantList m_fixtures;
    /** A reference to the SceneFixtureConsole when loaded */
    QQuickItem *m_sceneConsole;
    /** Reference to a DMX source used to edit a Scene */
    GenericDMXSource* m_source;
};

#endif // SCENEEDITOR_H
