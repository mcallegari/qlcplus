/*
  Q Light Controller Plus - Test Unit
  qmluistubs.cpp

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

/*
 * Minimal replacements for the QML UI classes that the Virtual Console
 * sources reference but that would otherwise pull in the whole application
 * (2D/3D views, networking, fixture/function managers...).
 * Only the symbols actually used by the VC code are implemented here.
 */

#include "contextmanager.h"
#include "fixturemanager.h"
#include "fixtureutils.h"
#include "tardis.h"
#include "app.h"

/*****************************************************************************
 * Tardis: no undo/redo history, no network forwarding
 *****************************************************************************/

Tardis *Tardis::s_instance = nullptr;

Tardis::Tardis(QQuickView *view, Doc *doc, NetworkManager *netMgr,
               FixtureManager *fxMgr, FunctionManager *funcMgr, ContextManager *ctxMgr, SimpleDesk *sDesk,
               ShowManager *showMgr, VirtualConsole *vc, QObject *parent)
    : QThread(parent)
    , m_running(false)
    , m_view(view)
    , m_doc(doc)
    , m_networkManager(netMgr)
    , m_fixtureManager(fxMgr)
    , m_functionManager(funcMgr)
    , m_contextManager(ctxMgr)
    , m_simpleDesk(sDesk)
    , m_showManager(showMgr)
    , m_virtualConsole(vc)
    , m_historyIndex(-1)
    , m_historyCount(0)
    , m_busy(false)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;
}

Tardis::~Tardis()
{
    s_instance = nullptr;
}

Tardis *Tardis::instance()
{
    return s_instance;
}

void Tardis::enqueueAction(int code, quint32 objID, QVariant oldVal, QVariant newVal)
{
    Q_UNUSED(code)
    Q_UNUSED(objID)
    Q_UNUSED(oldVal)
    Q_UNUSED(newVal)
}

void Tardis::undoAction()
{
}

void Tardis::redoAction()
{
}

QByteArray Tardis::actionToByteArray(int code, quint32 objID, QVariant data)
{
    Q_UNUSED(code)
    Q_UNUSED(objID)
    Q_UNUSED(data)
    return QByteArray();
}

void Tardis::run()
{
}

void Tardis::slotProcessNetworkAction(int code, quint32 id, QVariant value)
{
    Q_UNUSED(code)
    Q_UNUSED(id)
    Q_UNUSED(value)
}

/*****************************************************************************
 * ContextManager: VC pages are not registered anywhere
 *****************************************************************************/

void ContextManager::registerContext(PreviewContext *context)
{
    Q_UNUSED(context)
}

void ContextManager::unregisterContext(QString name)
{
    Q_UNUSED(name)
}

/*****************************************************************************
 * FixtureManager: the fixture tree models are left empty
 *****************************************************************************/

void FixtureManager::updateGroupsTree(Doc *doc, TreeModel *treeModel, QString searchFilter,
                                      int showFlags, QList<SceneValue> checkedChannels)
{
    Q_UNUSED(doc)
    Q_UNUSED(treeModel)
    Q_UNUSED(searchFilter)
    Q_UNUSED(showFlags)
    Q_UNUSED(checkedChannels)
}

/*****************************************************************************
 * FixtureUtils: same bit layout as qmlui/fixtureutils.cpp
 *****************************************************************************/

#define FIXTURE_HEAD_BITS   8
#define FIXTURE_LINKED_BITS 8

quint32 FixtureUtils::itemFixtureID(quint32 itemID)
{
    return (itemID >> (FIXTURE_HEAD_BITS + FIXTURE_LINKED_BITS));
}

quint16 FixtureUtils::itemLinkedIndex(quint32 itemID)
{
    return (itemID & ((1 << FIXTURE_LINKED_BITS) - 1));
}

/*****************************************************************************
 * App: the test view is a plain QQuickView, so qobject_cast<App *>
 * always fails and VirtualConsole::pixelDensity() returns 0
 *****************************************************************************/

const QMetaObject App::staticMetaObject = QQuickView::staticMetaObject;

qreal App::pixelDensity() const
{
    return 0;
}
