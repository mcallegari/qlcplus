/*
  Q Light Controller Plus
  tardis.cpp

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

#include "tardis.h"

#include "virtualconsole.h"
#include "fixturemanager.h"
#include "vcwidget.h"
#include "doc.h"

/* The time in milliseconds to declare an action
 * a duplicate or belonging to a batch of actions */
#define TARDIS_ACTION_INTERTIME     100
/* The maximum number of action a Tardis can hold */
#define TARDIS_MAX_ACTIONS_NUMBER   100

Tardis* Tardis::s_instance = NULL;

Tardis::Tardis(QQuickView *view, Doc *doc, NetworkManager *netMgr,
               FixtureManager *fxMgr, FunctionManager *funcMgr,
               ShowManager *showMgr, VirtualConsole *vc, QObject *parent)
    : QThread(parent)
    , m_running(false)
    , m_view(view)
    , m_doc(doc)
    , m_networkManager(netMgr)
    , m_fixtureManager(fxMgr)
    , m_functionManager(funcMgr)
    , m_showManager(showMgr)
    , m_virtualConsole(vc)
    , m_undoing(false)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    m_uptime.start();

    start();
}

Tardis::~Tardis()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

Tardis *Tardis::instance()
{
    return s_instance;
}

void Tardis::enqueueAction(int code, QObject *object, QVariant oldVal, QVariant newVal)
{
    if (m_doc->loadStatus() == Doc::Loading || m_undoing)
        return;

    TardisAction action;
    action.m_timestamp = m_uptime.elapsed();
    action.m_action = code;
    action.m_object = object;
    action.m_oldValue = oldVal;
    action.m_newValue = newVal;
    m_actionsQueue.enqueue(action);
    m_queueSem.release();
    m_doc->setModified();
}

void Tardis::undoAction()
{
    if (m_history.isEmpty())
        return;

    bool done = false;

    m_undoing = true;

    while (!done)
    {
        TardisAction action = m_history.takeLast();
        qDebug() << "Undo action" << action.m_action;

        switch(action.m_action)
        {
            case FixtureCreate:
            {
                m_fixtureManager->deleteFixtures(QVariantList( { action.m_newValue } ));
            }
            break;
            case VCWidgetCreate:
            {
                m_virtualConsole->deleteVCWidgets(QVariantList( { action.m_newValue } ));
            }
            break;
            case VCWidgetGeometry:
            {
                auto member = std::mem_fn(&VCWidget::setGeometry);
                member(qobject_cast<VCWidget *>(action.m_object), action.m_oldValue.toRectF());
            }
            break;
            case VCWidgetCaption:
            {
                auto member = std::mem_fn(&VCWidget::setCaption);
                member(qobject_cast<VCWidget *>(action.m_object), action.m_oldValue.toString());
            }
            break;
            case VCWidgetBackgroundColor:
            {
                auto member = std::mem_fn(&VCWidget::setBackgroundColor);
                member(qobject_cast<VCWidget *>(action.m_object), action.m_oldValue.value<QColor>());
            }
            break;
            case VCWidgetBackgroundImage:
            {
                auto member = std::mem_fn(&VCWidget::setBackgroundImage);
                member(qobject_cast<VCWidget *>(action.m_object), action.m_oldValue.toString());
            }
            break;
            case VCWidgetForegroundColor:
            {
                auto member = std::mem_fn(&VCWidget::setForegroundColor);
                member(qobject_cast<VCWidget *>(action.m_object), action.m_oldValue.value<QColor>());
            }
            break;
            case VCWidgetFont:
            {
                auto member = std::mem_fn(&VCWidget::setFont);
                member(qobject_cast<VCWidget *>(action.m_object), action.m_oldValue.value<QFont>());
            }
            break;
            default:
            break;
        }

        /* Check if I am processing a batch of actions or a single one */
        if (m_history.isEmpty() ||
            action.m_timestamp - m_history.last().m_timestamp > TARDIS_ACTION_INTERTIME)
        {
            done = true;
        }
    }

    m_undoing = false;
}

void Tardis::resetHistory()
{
    m_history.clear();
}

void Tardis::run()
{
    m_running = true;

    while(m_running)
    {
        if (m_queueSem.tryAcquire(1, 1000) == false)
        {
            qDebug() << "No actions to process, history length:" << m_history.count();
            continue;
        }

        if (m_actionsQueue.isEmpty())
            continue;

        TardisAction action = m_actionsQueue.dequeue();

        if (m_history.isEmpty() == false)
        {
            TardisAction lastAction = m_history.last();

            /* if the current action code is the same of the last action,
             * and it happened very fast, discard the previous and keep just one */
            if (action.m_action == lastAction.m_action &&
                action.m_object == lastAction.m_object &&
                action.m_timestamp - lastAction.m_timestamp < TARDIS_ACTION_INTERTIME)
            {
                action.m_oldValue = lastAction.m_oldValue;
                m_history.removeLast();
            }
        }

        m_history.append(action);

        /* So long and thanks for all the fish */
        if (m_history.count() > TARDIS_MAX_ACTIONS_NUMBER)
            m_history.removeFirst();

        qDebug() << "Got action:" << action.m_action << ", history length:" << m_history.count();
    }
}

