/*
  Q Light Controller Plus
  tardis.h

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

#ifndef TARDIS_H
#define TARDIS_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>
#include <QQuickView>
#include <QElapsedTimer>

#include "tardisactions.h"

class FixtureManager;
class FunctionManager;
class ContextManager;
class VirtualConsole;
class NetworkManager;
class ShowManager;
class Doc;

class Tardis : public QThread
{
    Q_OBJECT

public:
    explicit Tardis(QQuickView *view, Doc *doc, NetworkManager *netMgr,
                    FixtureManager *fxMgr, FunctionManager *funcMgr,
                    ContextManager *ctxMgr, ShowManager *showMgr, VirtualConsole *vc,
                    QObject *parent = 0);

    ~Tardis();

    /** Get the singleton instance */
    static Tardis* instance();

    /** Build a TardisAction with the provided data and enqueue it
     *  to be processed by the Tardis thread */
    void enqueueAction(int code, QObject *object, QVariant oldVal, QVariant newVal);

    /** Undo an action or a batch of actions taken from history */
    Q_INVOKABLE void undoAction();

    void processAction(TardisAction &action);

    /** Reset the actions history */
    void resetHistory();

    /** @reimp */
    void run(); // thread run function

protected slots:
    void slotProcessNetworkAction(int code, quint32 id, QVariant value);

private:
    /** The singleton Tardis instance */
    static Tardis* s_instance;
    /** Thread running status flag */
    bool m_running;

    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the Network Manager */
    NetworkManager *m_networkManager;

    /** Reference to the Fixture Manager */
    FixtureManager *m_fixtureManager;
    /** Reference to the Function Manager */
    FunctionManager *m_functionManager;
    /** Reference to the Context Manager */
    ContextManager *m_contextManager;
    /** Reference to the Show Manager */
    ShowManager *m_showManager;
    /** Reference to the Virtual Console */
    VirtualConsole *m_virtualConsole;

    /** Time reference since application starts */
    QElapsedTimer m_uptime;

    /** A inter-thread queue to desynchronize actions processing */
    QQueue<TardisAction> m_actionsQueue;

    /** Synchronization variables between threads */
    QMutex m_queueMutex;
    QSemaphore m_queueSem;

    /** The actual history of actions */
    QList<TardisAction> m_history;

    /** Count the actions (or batch of actions) recorded */
    int m_historyCount;

    /** Flag to prevent actions looping */
    bool m_busy;
};

#endif /* TARDIS_H */
