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
#include <QXmlStreamReader>
#include <QtCore/qbuffer.h>

#include "tardis.h"

#include "networkmanager.h"
#include "virtualconsole.h"
#include "fixturemanager.h"
#include "functionmanager.h"
#include "contextmanager.h"
#include "functioneditor.h"
#include "vcwidget.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
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
    , m_busy(false)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    qRegisterMetaType<TardisAction>();

    m_uptime.start();

    connect(m_networkManager, &NetworkManager::actionReady, this, &Tardis::slotProcessNetworkAction);

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
    if (m_doc->loadStatus() == Doc::Loading || m_busy)
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

    m_busy = true;

    while (!done)
    {
        TardisAction action = m_history.takeLast();
        qDebug() << "Undo action" << action.m_action;

        processAction(action);

        /* Check if I am processing a batch of actions or a single one */
        if (m_history.isEmpty() ||
            action.m_timestamp - m_history.last().m_timestamp > TARDIS_ACTION_INTERTIME)
        {
            done = true;
        }
    }

    m_historyCount--;

    m_busy = false;
}

void Tardis::resetHistory()
{
    m_history.clear();
}

void Tardis::run()
{
    m_running = true;

    while (m_running)
    {
        if (m_queueSem.tryAcquire(1, 1000) == false)
        {
            //qDebug() << "No actions to process, history length:" << m_historyCount << "(" << m_history.count() << ")";
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

        /* If there are active network connections, send the action there too */
        if (m_networkManager->connectionsCount())
        {
            quint32 id = UINT_MAX;

            if (action.m_action < FunctionCreate)
            {
                Fixture *fixture = qobject_cast<Fixture *>(action.m_object);
                if (fixture)
                    id = fixture->id();
            }
            else if(action.m_action < VCWidgetCreate)
            {
                Function *function = qobject_cast<Function *>(action.m_object);
                if (function)
                    id = function->id();
            }
            else if(action.m_action < NetAnnounce)
            {
                VCWidget *widget = qobject_cast<VCWidget *>(action.m_object);
                if (widget)
                    id = widget->id();
            }

            QMetaObject::invokeMethod(m_networkManager, "sendAction", Qt::QueuedConnection,
                    Q_ARG(quint32, id),
                    Q_ARG(TardisAction, action));
        }
    }
}

void Tardis::slotProcessNetworkAction(int code, quint32 id, QVariant value)
{
    TardisAction action;
    action.m_action = code;
    action.m_object = NULL;

    // 1- prepare some basic QObject references for the action
    if (code < FunctionCreate)
    {
        Fixture *fixture = m_doc->fixture(id);
        if (fixture)
            action.m_object = fixture;
    }
    else if(code < VCWidgetCreate)
    {
        Function *function = m_doc->function(id);
        if (function)
            action.m_object = function;
    }
    else if(code < NetAnnounce)
    {
        VCWidget *widget = m_virtualConsole->widget(id);
        if (widget)
            action.m_object = widget;
    }
    else
    {
        qDebug() << "This should not happen. Most likely NetworkManager has no implementation for" << code;
        return;
    }

    // 2- Handle creation cases, where an XML fragment is provided
    switch(code)
    {
        case FixtureCreate:
        {
            QBuffer buffer;
            buffer.setData(value.toByteArray());
            buffer.open(QIODevice::ReadOnly | QIODevice::Text);
            QXmlStreamReader xmlReader(&buffer);
            xmlReader.readNextStartElement();
            Fixture::loader(xmlReader, m_doc);
            return;
        }
        break;
        case EFXAddFixture:
        {
            EFX *efx = qobject_cast<EFX *>(action.m_object);
            EFXFixture *ef = new EFXFixture(efx);
            QBuffer buffer;

            buffer.setData(value.toByteArray());
            buffer.open(QIODevice::ReadOnly | QIODevice::Text);
            QXmlStreamReader xmlReader(&buffer);
            xmlReader.readNextStartElement();

            ef->loadXML(xmlReader);
            efx->addFixture(ef);
            return;
        }
        break;

        default:
        break;
    }

    // 3- store value on oldValue, since we're going to call the method used for undoing actions
    action.m_oldValue = value;

    // 4- process the action
    m_busy = true;
    processAction(action);
    m_busy = false;
}

void Tardis::processAction(TardisAction &action)
{
    switch(action.m_action)
    {
        /* *********************** Fixture editing actions ************************ */
        case FixtureCreate:
        {
            m_fixtureManager->deleteFixtures(QVariantList( { action.m_newValue } ));
        }
        break;
        case FixtureSetPosition:
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

        /* *********************** Function editing actions *********************** */
        case FunctionCreate:
        {
            m_functionManager->deleteFunctions(QVariantList( { action.m_newValue } ));
        }
        break;
        case FunctionSetName:
        {
            auto member = std::mem_fn(&Function::setName);
            member(qobject_cast<Function *>(action.m_object), action.m_oldValue.toString());
        }
        break;
        case FunctionSetTempoType:
        {
            auto member = std::mem_fn(&Function::setTempoType);
            member(qobject_cast<Function *>(action.m_object), Function::TempoType(action.m_oldValue.toInt()));
        }
        break;
        case FunctionSetRunOrder:
        {
            auto member = std::mem_fn(&Function::setRunOrder);
            member(qobject_cast<Function *>(action.m_object), Function::RunOrder(action.m_oldValue.toInt()));
        }
        break;
        case FunctionSetDirection:
        {
            auto member = std::mem_fn(&Function::setDirection);
            member(qobject_cast<Function *>(action.m_object), Function::Direction(action.m_oldValue.toInt()));
        }
        break;
        case FunctionSetFadeIn:
        {
            auto member = std::mem_fn(&Function::setFadeInSpeed);
            member(qobject_cast<Function *>(action.m_object), action.m_oldValue.toUInt());
        }
        break;
        case FunctionSetFadeOut:
        {
            auto member = std::mem_fn(&Function::setFadeOutSpeed);
            member(qobject_cast<Function *>(action.m_object), action.m_oldValue.toUInt());
        }
        break;
        case FunctionSetDuration:
        {
            auto member = std::mem_fn(&Function::setDuration);
            member(qobject_cast<Function *>(action.m_object), action.m_oldValue.toUInt());
        }
        break;

        case SceneSetChannelValue:
        case SceneUnsetChannelValue:
        {
            SceneValue scv = action.m_oldValue.value<SceneValue>();
            Scene *scene = qobject_cast<Scene *>(action.m_object);
            if (scene)
                scene->setValue(scv.fxi, scv.channel, scv.value);
        }
        break;

        case ChaserSetStepFadeIn:
        {
            Chaser *chaser = qobject_cast<Chaser *>(action.m_object);
            UIntPair pairValue = action.m_oldValue.value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.fadeIn = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;
        case ChaserSetStepHold:
        {
            Chaser *chaser = qobject_cast<Chaser *>(action.m_object);
            UIntPair pairValue = action.m_oldValue.value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.hold = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;
        case ChaserSetStepFadeOut:
        {
            Chaser *chaser = qobject_cast<Chaser *>(action.m_object);
            UIntPair pairValue = action.m_oldValue.value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.fadeOut = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;
        case ChaserSetStepDuration:
        {
            Chaser *chaser = qobject_cast<Chaser *>(action.m_object);
            UIntPair pairValue = action.m_oldValue.value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.duration = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;

        case EFXSetAlgorithmIndex:
        {
            auto member = std::mem_fn(&EFX::setAlgorithm);
            member(qobject_cast<EFX *>(action.m_object), EFX::Algorithm(action.m_oldValue.toInt()));
        }
        break;
        case EFXSetRelative:
        {
            auto member = std::mem_fn(&EFX::setIsRelative);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toBool());
        }
        break;
        case EFXSetWidth:
        {
            auto member = std::mem_fn(&EFX::setWidth);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXSetHeight:
        {
            auto member = std::mem_fn(&EFX::setHeight);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXSetXOffset:
        {
            auto member = std::mem_fn(&EFX::setXOffset);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXSetYOffset:
        {
            auto member = std::mem_fn(&EFX::setYOffset);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXSetRotation:
        {
            auto member = std::mem_fn(&EFX::setRotation);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXSetStartOffset:
        {
            auto member = std::mem_fn(&EFX::setStartOffset);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXSetXFrequency:
        {
            auto member = std::mem_fn(&EFX::setXFrequency);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXSetYFrequency:
        {
            auto member = std::mem_fn(&EFX::setYFrequency);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXSetXPhase:
        {
            auto member = std::mem_fn(&EFX::setXPhase);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXSetYPhase:
        {
            auto member = std::mem_fn(&EFX::setYPhase);
            member(qobject_cast<EFX *>(action.m_object), action.m_oldValue.toInt());
        }
        break;
        case EFXAddFixture:
        {
            auto member = std::mem_fn(&EFX::removeFixture);
            EFXFixture *ef = (EFXFixture *)action.m_oldValue.value<void *>();
            member(qobject_cast<EFX *>(action.m_object), ef);
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
            qWarning() << "Action" << action.m_action << "not implemented !";
        break;
    }
}

