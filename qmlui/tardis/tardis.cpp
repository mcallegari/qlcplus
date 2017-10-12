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

#include <QMutexLocker>

#include "tardis.h"

#include "virtualconsole.h"
#include "fixturemanager.h"
#include "functionmanager.h"
#include "contextmanager.h"
#include "functioneditor.h"
#include "vcwidget.h"
#include "doc.h"

/* The time in milliseconds to declare an action
 * a duplicate or belonging to a batch of actions */
#define TARDIS_ACTION_INTERTIME     150
/* The maximum number of action a Tardis can hold */
#define TARDIS_MAX_ACTIONS_NUMBER   100

Tardis* Tardis::s_instance = NULL;

Tardis::Tardis(QQuickView *view, Doc *doc, NetworkManager *netMgr,
               FixtureManager *fxMgr, FunctionManager *funcMgr, ContextManager *ctxMgr,
               ShowManager *showMgr, VirtualConsole *vc, QObject *parent)
    : QThread(parent)
    , m_running(false)
    , m_view(view)
    , m_doc(doc)
    , m_networkManager(netMgr)
    , m_fixtureManager(fxMgr)
    , m_functionManager(funcMgr)
    , m_contextManager(ctxMgr)
    , m_showManager(showMgr)
    , m_virtualConsole(vc)
    , m_historyCount(0)
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
    {
        // enqueue the action under protection
        QMutexLocker locker(&m_queueMutex);
        m_actionsQueue.enqueue(action);
    }
    // inform the thread an action is available
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
            /* *********************** Fixture editing actions ************************ */
            case FixtureCreate:
            {
                m_fixtureManager->deleteFixtures(QVariantList( { action.m_newValue } ));
            }
            break;
            case FixturePosition:
            {
                Fixture *fixture = qobject_cast<Fixture *>(action.m_object);
                if (fixture)
                {
                    QVector3D pos = action.m_oldValue.value<QVector3D>();
                    m_contextManager->setFixturePosition(fixture->id(), pos.x(), pos.y(), pos.z());
                }
            }
            break;
            case FixtureSetDumpValue:
            {
                SceneValue scv = action.m_oldValue.value<SceneValue>();
                m_functionManager->setDumpValue(scv.fxi, scv.channel, scv.value, m_contextManager->dmxSource());
            }
            break;
            case FixtureSetChannelValue:
            {
                SceneValue scv = action.m_oldValue.value<SceneValue>();
                m_functionManager->setChannelValue(scv.fxi, scv.channel, scv.value);
            }
            break;

            /* *********************** Function editing actions *********************** */
            case FunctionCreate:
            {
                m_functionManager->deleteFunctions(QVariantList( { action.m_newValue } ));
            }
            break;
            case FunctionSetName:
            {
                Function *f = qobject_cast<Function *>(action.m_object);
                m_functionManager->setEditorFunction(f->id(), true);
                FunctionEditor *editor = m_functionManager->currentEditor();
                if (editor != NULL)
                    editor->setFunctionName(action.m_oldValue.toString());
            }
            break;
            case FunctionSetTempoType:
            {
                Function *f = qobject_cast<Function *>(action.m_object);
                m_functionManager->setEditorFunction(f->id(), true);
                FunctionEditor *editor = m_functionManager->currentEditor();
                if (editor != NULL)
                    editor->setTempoType(action.m_oldValue.toInt());
            }
            break;

            /* ******************* Virtual console editing actions ******************** */

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

    m_historyCount--;

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
            qDebug() << "No actions to process, history length:" << m_historyCount << "(" << m_history.count() << ")";
            continue;
        }

        TardisAction action;
        bool match = false;

        {
            QMutexLocker locker(&m_queueMutex);
            if (m_actionsQueue.isEmpty())
                continue;

            action = m_actionsQueue.dequeue();
        }

        if (m_history.count())
        {
            // scan history from the last item to find a match
            for (int i = m_history.count() - 1; i >= 0; i--)
            {
                if (action.m_timestamp - m_history.at(i).m_timestamp > TARDIS_ACTION_INTERTIME)
                    break;

                if (action.m_action == m_history.at(i).m_action &&
                    action.m_object == m_history.at(i).m_object &&
                    action.m_oldValue == m_history.at(i).m_newValue)
                {
                    qDebug() << "Found match at" << i << action.m_oldValue << m_history.at(i).m_newValue;
                    action.m_oldValue = m_history.at(i).m_oldValue;
                    m_history.replace(i, action);
                    match = true;
                    break;
                }
            }
        }

        if (m_history.isEmpty() || action.m_timestamp - m_history.last().m_timestamp > TARDIS_ACTION_INTERTIME)
            m_historyCount++;

        if (match == false)
            m_history.append(action);

        /* So long and thanks for all the fish */
        if (m_historyCount > TARDIS_MAX_ACTIONS_NUMBER)
            m_history.removeFirst();

        qDebug() << "Got action:" << action.m_action << ", history length:" << m_historyCount << "(" << m_history.count() << ")";
    }
}

