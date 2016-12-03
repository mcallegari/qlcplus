/*
  Q Light Controller Plus
  contextmanager.h

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

#ifndef CONTEXTMANAGER_H
#define CONTEXTMANAGER_H

#include <QObject>
#include <QQuickView>

#include "scenevalue.h"

class Doc;
class MainView2D;
class MainViewDMX;
class FixtureManager;
class FunctionManager;
class GenericDMXSource;
class PreviewContext;

class ContextManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 universeFilter READ universeFilter WRITE setUniverseFilter NOTIFY universeFilterChanged)
    Q_PROPERTY(bool hasSelectedFixtures READ hasSelectedFixtures NOTIFY selectedFixturesChanged)
    Q_PROPERTY(int fixturesRotation READ fixturesRotation WRITE setFixturesRotation)

public:
    explicit ContextManager(QQuickView *view, Doc *doc,
                            FixtureManager *fxMgr, FunctionManager *funcMgr,
                            QObject *parent = 0);
    ~ContextManager();

    /** Register/Unregister a context to the map of known contexts */
    void registerContext(PreviewContext *context);
    void unregisterContext(QString name);

    /** Enable/disable the context with the specified $name.
     *  This sets a flag in the context to know if it is visible
     *  on the screen, so to decide if changes should be applied to it */
    Q_INVOKABLE void enableContext(QString name, bool enable, QQuickItem *item);

    /** Detach/Reattach a context from/to the application main window */
    Q_INVOKABLE void detachContext(QString name);
    Q_INVOKABLE void reattachContext(QString name);

public slots:
    /** Resets the data structures and update the currently enabled views */
    void resetContexts();

    /** Handle a key press from a QQuickView context */
    void handleKeyPress(QKeyEvent *e);

    /** Handle a key release from a QQuickView context */
    void handleKeyRelease(QKeyEvent *e);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;

    /** Reference to a simple PreviewContext representing
     *  the universe grid view, since it doesn't have a dedicated class */
    PreviewContext *m_uniGridView;
    /** Reference to the DMX Preview context */
    MainViewDMX *m_DMXView;
    /** Reference to the 2D Preview context */
    MainView2D *m_2DView;
    /** Reference to the Fixture Manager */
    FixtureManager *m_fixtureManager;
    /** Reference to the Function Manager */
    FunctionManager *m_functionManager;

    QMap <QString, PreviewContext *> m_contextsMap;

    /*********************************************************************
     * Universe filtering
     *********************************************************************/
public:
    /** Get/Set the universe displayed by contexts */
    quint32 universeFilter() const;
    void setUniverseFilter(quint32 universeFilter);

signals:
    void universeFilterChanged(quint32 universeFilter);

private:
    /** The currently displayed universe
      * The value Universe::invalid() means "All universes" */
    quint32 m_universeFilter;

    /*********************************************************************
     * Common fixture helpers
     *********************************************************************/
public:
    Q_INVOKABLE void setFixtureSelection(quint32 fxID, bool enable);

    Q_INVOKABLE void resetFixtureSelection();

    Q_INVOKABLE void toggleFixturesSelection();

    Q_INVOKABLE void setRectangleSelection(qreal x, qreal y, qreal width, qreal height);

    bool hasSelectedFixtures();

    Q_INVOKABLE void setFixturePosition(quint32 fxID, qreal x, qreal y);

    Q_INVOKABLE void setFixturesAlignment(int alignment);

    Q_INVOKABLE void createFixtureGroup();

    int fixturesRotation() const;
    void setFixturesRotation(int degrees);

protected slots:
    void slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z = 0);
    void slotChannelValueChanged(quint32 fxID, quint32 channel, quint8 value);
    void slotChannelTypeValueChanged(int type, quint8 value, quint32 channel = UINT_MAX);
    void slotColorChanged(QColor col, QColor wauv);
    void slotPositionChanged(int type, int degrees);
    void slotPresetChanged(const QLCChannel *channel, quint8 value);

    /** Invoked by the QLC+ engine to inform the UI that the Universe at $idx
     *  has changed */
    void slotUniversesWritten(int idx, const QByteArray& ua);

    /** Invoked when Function editing begins or ends in the Function Manager.
     *  Context Manager doesn't care much about Functions, it just needs
     *  to know if it has to set channel values on the GenericDMXSource or
     *  forward them to the Function Manager */
    void slotFunctionEditingChanged(bool status);

signals:
    void selectedFixturesChanged();

private:
    /** The list of the currently selected Fixture IDs */
    QList<quint32> m_selectedFixtures;

    /** Holds the last rotation value to handle relative changes */
    int m_prevRotation;

    /** A flag indicating if a Function is currently being edited */
    bool m_editingEnabled;

    /** A multihash containing the selected fixtures' capabilities by channel type */
    /** The hash is: int (channel type) , SceneValue (Fixture ID and channel) */
    QMultiHash<int, SceneValue> m_channelsMap;

    /*********************************************************************
     * DMX channels dump
     *********************************************************************/
public:
    Q_INVOKABLE void dumpDmxChannels();

    /** Resets the current values used for dumping or preview */
    Q_INVOKABLE void resetDumpValues();

private:
    /** Reference to a Generic DMX source used to handle Scenes dump */
    GenericDMXSource* m_source;
};

#endif // CONTEXTMANAGER_H
