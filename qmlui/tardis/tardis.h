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
#include <QSemaphore>
#include <QQuickView>
#include <QElapsedTimer>

#include "tardisactions.h"

class FixtureManager;
class FunctionManager;
class VirtualConsole;
class NetworkManager;
class ShowManager;
class Doc;

typedef struct
{
    int m_action;
    qint64 m_timestamp;
    QObject *m_object;
    QVariant m_oldValue;
    QVariant m_newValue;
} TardisAction;

class Tardis : public QThread
{
public:
    explicit Tardis(QQuickView *view, Doc *doc, NetworkManager *netMgr,
                    FixtureManager *fxMgr, FunctionManager *funcMgr,
                    ShowManager *showMgr, VirtualConsole *vc,
                    QObject *parent = 0);

    ~Tardis();

    /** Get the singleton instance */
    static Tardis* instance();

    void enqueueAction(int code, QObject *object, QVariant oldVal, QVariant newVal);

    void undoAction();

    void resetHistory();

    /** @reimp */
    void run(); //thread run function

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
    /** Reference to the Show Manager */
    ShowManager *m_showManager;
    /** Reference to the Virtual Console */
    VirtualConsole *m_virtualConsole;

    /** Time reference since application starts */
    QElapsedTimer m_uptime;

    /** A inter-thread queue to desynchronize actions processing */
    QQueue<TardisAction> m_actionsQueue;

    /** Synchronization semaphore to monitor actions queue data */
    QSemaphore m_queueSem;

    /** The actual history of actions */
    QList<TardisAction> m_history;

    bool m_undoing;
};

#endif /* TARDIS_H */
