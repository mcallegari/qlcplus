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
class GenericDMXSource;

class ContextManager : public QObject
{
    Q_OBJECT
public:
    explicit ContextManager(QQuickView *view, Doc *doc,
                            FixtureManager *fxMgr, QObject *parent = 0);

    Q_INVOKABLE void activateContext(QString context);

    Q_INVOKABLE void setFixtureSelection(quint32 fxID, bool enable);

signals:

protected slots:
    void slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z = 0);
    void slotChannelTypeValueChanged(int type, quint8 value);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the Fixture Manager */
    FixtureManager *m_fixtureManager;
    /** A multihash containg the fixture capabilities by channel type */
    QMultiHash<int, SceneValue> m_channelsMap;
    /** Reference to a DMX source used to handle scenes design */
    GenericDMXSource* m_source;
    /** Reference to the DMX Preview context */
    MainViewDMX *m_DMXView;
    /** Reference to the 2D Preview context */
    MainView2D *m_2DView;
};

#endif // CONTEXTMANAGER_H
