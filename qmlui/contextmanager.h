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

class ContextManager : public QObject
{
    Q_OBJECT
public:
    explicit ContextManager(QQuickView *view, Doc *doc,
                            FixtureManager *fxMgr, FunctionManager *funcMgr,
                            QObject *parent = 0);

    Q_INVOKABLE void enableContext(QString context, bool enable);

    Q_INVOKABLE void detachContext(QString context);

    Q_INVOKABLE void reattachContext(QString context);

    Q_INVOKABLE void setFixtureSelection(quint32 fxID, bool enable);

    Q_INVOKABLE void setRectangleSelection(qreal x, qreal y, qreal width, qreal height);

    Q_INVOKABLE void dumpDmxChannels();

signals:

protected slots:
    void slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z = 0);
    void slotChannelValueChanged(quint32 fxID, quint32 channel, quint8 value);
    void slotChannelTypeValueChanged(int type, quint8 value, quint32 channel = UINT_MAX);
    void slotColorChanged(QColor col, QColor wauv);
    void slotPresetChanged(const QLCChannel *channel, quint8 value);
    void slotUniversesWritten(int idx, const QByteArray& ua);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the Fixture Manager */
    FixtureManager *m_fixtureManager;
    /** Reference to the Function Manager */
    FunctionManager *m_functionManager;
    /** The list of the currently selected Fixture IDs */
    QList<quint32> m_selectedFixtures;
    /** A multihash containing the selected fixtures' capabilities by channel type */
    /** The hash is: int (channel type) , SceneValue (Fixture ID and channel) */
    QMultiHash<int, SceneValue> m_channelsMap;
    /** Reference to a DMX source used to handle scenes design */
    GenericDMXSource* m_source;
    /** Reference to the DMX Preview context */
    MainViewDMX *m_DMXView;
    /** Reference to the 2D Preview context */
    MainView2D *m_2DView;
};

#endif // CONTEXTMANAGER_H
