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
#include "vcframe.h"
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
    , m_historyIndex(-1)
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

void Tardis::enqueueAction(int code, quint32 objID, QVariant oldVal, QVariant newVal)
{
    if (m_busy || m_doc->loadStatus() == Doc::Loading ||
        m_virtualConsole->loadStatus() == VirtualConsole::Loading)
        return;

    TardisAction action;
    action.m_timestamp = m_uptime.elapsed();
    action.m_action = code;
    action.m_objID = objID;
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

    m_busy = true;

    quint64 refTimestamp = m_history.at(m_historyIndex).m_timestamp;

    while (1)
    {
        TardisAction action = m_history.at(m_historyIndex);

        if (refTimestamp - action.m_timestamp > TARDIS_ACTION_INTERTIME)
            break;

        qDebug("Undo action 0x%02X", action.m_action);

        m_historyIndex--;

        int code = processAction(action, true);

        /* If there are active network connections, send the action there too */
        if (m_networkManager->connectionsCount())
        {
            QMetaObject::invokeMethod(m_networkManager, "sendAction", Qt::QueuedConnection,
                    Q_ARG(int, code),
                    Q_ARG(TardisAction, action));
        }

        if (m_historyIndex == -1)
            break;
    }

    qDebug() << "History index:" << m_historyIndex;

    m_busy = false;
}

void Tardis::redoAction()
{
    if (m_history.isEmpty() || m_historyIndex == m_history.count() - 1)
        return;

    bool done = false;

    m_busy = true;

    quint64 refTimestamp = m_history.at(m_historyIndex + 1).m_timestamp;

    while (!done)
    {
        m_historyIndex++;

        TardisAction action = m_history.at(m_historyIndex);
        qDebug("Redo action 0x%02X", action.m_action);

        int code = processAction(action, false);

        /* If there are active network connections, send the action there too */
        if (m_networkManager->connectionsCount())
        {
            QMetaObject::invokeMethod(m_networkManager, "sendAction", Qt::QueuedConnection,
                    Q_ARG(int, code),
                    Q_ARG(TardisAction, action));
        }

        /* Check if I am processing a batch of actions or a single one */
        if (m_historyIndex == m_history.count() - 1 ||
            action.m_timestamp - refTimestamp > TARDIS_ACTION_INTERTIME)
        {
            done = true;
        }
    }

    qDebug() << "History index:" << m_historyIndex;

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

        /* If the history index is halfway, it means I need to remove
         * all the actions after the last undo operation before
         * pushing a new one */
        if (m_historyIndex >= 0 && m_historyIndex != m_history.count())
        {
            int count = m_history.count();
            qint64 refTimestamp = m_history.last().m_timestamp;

            for (int i = m_historyIndex + 1; i < count; i++)
            {
                if (refTimestamp - m_history.last().m_timestamp > TARDIS_ACTION_INTERTIME)
                {
                    refTimestamp = m_history.last().m_timestamp;
                    m_historyCount--;
                }
                m_history.removeLast();

            }
        }

        if (m_history.count())
        {
            // scan history from the last item to find a match
            for (int i = m_history.count() - 1; i >= 0; i--)
            {
                if (action.m_timestamp - m_history.at(i).m_timestamp > TARDIS_ACTION_INTERTIME)
                    break;

                if (action.m_action == m_history.at(i).m_action &&
                    action.m_objID == m_history.at(i).m_objID &&
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
        {
            qint64 refTimestamp = m_history.first().m_timestamp;
            while (m_history.first().m_timestamp - refTimestamp < TARDIS_ACTION_INTERTIME)
                m_history.removeFirst();
            m_historyCount = TARDIS_MAX_ACTIONS_NUMBER;
        }

        m_historyIndex = m_history.count() - 1;

        qDebug("Got action: 0x%02X, history length: %d (%d)", action.m_action, m_historyCount, m_history.count());

        /* If there are active network connections, send the action there too */
        if (m_networkManager->connectionsCount())
        {
            QMetaObject::invokeMethod(m_networkManager, "sendAction", Qt::QueuedConnection,
                    Q_ARG(int, action.m_action),
                    Q_ARG(TardisAction, action));
        }
    }
}

QByteArray Tardis::actionToByteArray(int code, quint32 objID, QVariant data)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    switch(code)
    {
        case FixtureCreate:
        case FixtureDelete:
        {
            Fixture *fixture = qobject_cast<Fixture *>(m_doc->fixture(objID));
            if (fixture)
                fixture->saveXML(&xmlWriter);
        }
        break;
        case FixtureGroupCreate:
        case FixtureGroupDelete:
        {
            FixtureGroup *group = qobject_cast<FixtureGroup *>(m_doc->fixtureGroup(objID));
            if (group)
                group->saveXML(&xmlWriter);
        }
        break;
        case FunctionCreate:
        case FunctionDelete:
        {
            Function *function = qobject_cast<Function *>(m_doc->function(objID));
            if (function)
                function->saveXML(&xmlWriter);
        }
        break;
        case ChaserAddStep:
        case ChaserRemoveStep:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(objID));
            ChaserStep *step = chaser->stepAt(data.toInt());
            step->saveXML(&xmlWriter, data.toInt(), chaser->type() == Function::SequenceType ? true : false);
        }
        break;
        case EFXAddFixture:
        case EFXRemoveFixture:
        {
            // EFXFixture reference is stored on data, so let's C-cast the QVariant value
            EFXFixture *fixture = (EFXFixture *)data.value<void *>();
            fixture->saveXML(&xmlWriter);
        }
        break;
        case VCWidgetCreate:
        {
            VCWidget *widget = qobject_cast<VCWidget *>(m_virtualConsole->widget(objID));
            if (widget)
                widget->saveXML(&xmlWriter);
        }
        case VCWidgetDelete:
        {
            VCWidget *widget = qobject_cast<VCWidget *>(m_virtualConsole->widget(objID));
            if (widget)
                widget->saveXML(&xmlWriter);
        }
        break;
        default:
            qWarning() << "Buffered action" << code << "not implemented !";
        break;
    }

    return buffer.buffer();
}

bool Tardis::processBufferedAction(int action, quint32 objID, QVariant &value)
{
    if (value.type() != QVariant::ByteArray)
    {
        qWarning("Action 0x%02X is not buffered !", action);
        return false;
    }

    QBuffer buffer;
    buffer.setData(value.toByteArray());
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    qDebug() << "Data to process:" << value.toString();

    switch(action)
    {
        case FixtureCreate:
        {
            Fixture::loader(xmlReader, m_doc);
        }
        break;
        case FixtureDelete:
        {
            m_fixtureManager->deleteFixtures(QVariantList( { objID } ));
        }
        break;
        case FixtureGroupCreate:
        {
            FixtureGroup::loader(xmlReader, m_doc);
        }
        break;
        case FunctionCreate:
        {
            Function::loader(xmlReader, m_doc);
        }
        break;
        case FunctionDelete:
        {
            m_functionManager->deleteFunctions(QVariantList( { objID } ));
        }
        break;
        case ChaserAddStep:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(objID));
            ChaserStep step;
            int stepNumber = -1;

            if (step.loadXML(xmlReader, stepNumber) == true)
                chaser->addStep(step, stepNumber);
        }
        break;
        case EFXAddFixture:
        {
            EFX *efx = qobject_cast<EFX *>(m_doc->function(objID));
            EFXFixture *ef = new EFXFixture(efx);

            ef->loadXML(xmlReader);
            efx->addFixture(ef);
        }
        break;
        case EFXRemoveFixture:
        {
            EFX *efx = qobject_cast<EFX *>(m_doc->function(objID));
            EFXFixture *ef = new EFXFixture(efx);

            ef->loadXML(xmlReader);
            efx->removeFixture(ef->head().fxi, ef->head().head);
            delete ef;
        }
        break;
        case VCWidgetCreate:
        {
            VCFrame *frame = qobject_cast<VCFrame *>(m_virtualConsole->widget(objID));
            if (frame)
                frame->loadWidgetXML(xmlReader, true);
        }
        break;
        case VCWidgetDelete:
        {
            QXmlStreamAttributes attrs = xmlReader.attributes();

            if (attrs.hasAttribute(KXMLQLCVCWidgetID))
                m_virtualConsole->deleteVCWidgets(QVariantList( { attrs.value(KXMLQLCVCWidgetID).toUInt() } ));
        }
        break;

        default:
            // This action was either not buffered or not implemented
            return false;
        break;
    }

    return true;
}

void Tardis::slotProcessNetworkAction(int code, quint32 id, QVariant value)
{
    // Handle creation cases, where an XML fragment is provided
    if (processBufferedAction(code, id, value))
        return;

    TardisAction action;
    action.m_action = code;
    action.m_objID = id;
    action.m_newValue = value;

    // process the action
    m_busy = true;
    processAction(action, false);
    m_busy = false;
}

int Tardis::processAction(TardisAction &action, bool undo)
{
    QVariant *value = undo ? &action.m_oldValue : &action.m_newValue;

    switch(action.m_action)
    {
        /* *********************** Fixture editing actions ************************ */
        case FixtureCreate:
        {
            processBufferedAction(undo ? FixtureDelete : FixtureCreate, action.m_objID, action.m_newValue);
            return undo ? FixtureDelete : FixtureCreate;
        }
        break;
        case FixtureDelete:
        {
            processBufferedAction(undo ? FixtureCreate : FixtureDelete, action.m_objID, action.m_oldValue);
            return undo ? FixtureCreate : FixtureDelete;
        }
        break;
        case FixtureMove:
        {
            m_fixtureManager->moveFixture(action.m_objID, value->toUInt());
        }
        break;
        case FixtureSetPosition:
        {
            Fixture *fixture = m_doc->fixture(action.m_objID);
            if (fixture)
            {
                QVector3D pos = value->value<QVector3D>();
                m_contextManager->setFixturePosition(fixture->id(), pos.x(), pos.y(), pos.z());
            }
        }
        break;
        case FixtureSetDumpValue:
        {
            SceneValue scv = value->value<SceneValue>();
            m_functionManager->setDumpValue(scv.fxi, scv.channel, scv.value, m_contextManager->dmxSource());
        }
        break;

        /* *********************** Function editing actions *********************** */
        case FunctionCreate:
        {
            processBufferedAction(undo ? FunctionDelete : FunctionCreate, action.m_objID, action.m_newValue);
            return undo ? FunctionDelete : FunctionCreate;
        }
        break;
        case FunctionDelete:
        {
            processBufferedAction(undo ? FunctionCreate : FunctionDelete, action.m_objID, action.m_oldValue);
            return undo ? FunctionCreate : FunctionDelete;
        }
        break;
        case FunctionSetName:
        {
            auto member = std::mem_fn(&Function::setName);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), value->toString());
        }
        break;
        case FunctionSetTempoType:
        {
            auto member = std::mem_fn(&Function::setTempoType);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), Function::TempoType(value->toInt()));
        }
        break;
        case FunctionSetRunOrder:
        {
            auto member = std::mem_fn(&Function::setRunOrder);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), Function::RunOrder(value->toInt()));
        }
        break;
        case FunctionSetDirection:
        {
            auto member = std::mem_fn(&Function::setDirection);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), Function::Direction(value->toInt()));
        }
        break;
        case FunctionSetFadeIn:
        {
            auto member = std::mem_fn(&Function::setFadeInSpeed);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), value->toUInt());
        }
        break;
        case FunctionSetFadeOut:
        {
            auto member = std::mem_fn(&Function::setFadeOutSpeed);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), value->toUInt());
        }
        break;
        case FunctionSetDuration:
        {
            auto member = std::mem_fn(&Function::setDuration);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), value->toUInt());
        }
        break;

        case SceneSetChannelValue:
        case SceneUnsetChannelValue:
        {
            SceneValue scv = value->value<SceneValue>();
            Scene *scene = qobject_cast<Scene *>(m_doc->function(action.m_objID));
            if (scene)
                scene->setValue(scv.fxi, scv.channel, scv.value);
        }
        break;

        case ChaserAddStep:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(action.m_objID));
            chaser->removeStep(action.m_newValue.toInt());
        }
        break;
        case ChaserSetStepFadeIn:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(action.m_objID));
            UIntPair pairValue = value->value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.fadeIn = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;
        case ChaserSetStepHold:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(action.m_objID));
            UIntPair pairValue = value->value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.hold = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;
        case ChaserSetStepFadeOut:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(action.m_objID));
            UIntPair pairValue = value->value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.fadeOut = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;
        case ChaserSetStepDuration:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(action.m_objID));
            UIntPair pairValue = value->value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.duration = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;

        case EFXAddFixture:
        {
            processBufferedAction(undo ? EFXRemoveFixture : EFXAddFixture, action.m_objID, action.m_newValue);
            return undo ? EFXRemoveFixture : EFXAddFixture;
        }
        break;
        case EFXRemoveFixture:
        {
            processBufferedAction(undo ? EFXAddFixture : EFXRemoveFixture, action.m_objID, action.m_oldValue);
            return undo ? EFXAddFixture : EFXRemoveFixture;
        }
        break;
        case EFXSetAlgorithmIndex:
        {
            auto member = std::mem_fn(&EFX::setAlgorithm);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), EFX::Algorithm(value->toInt()));
        }
        break;
        case EFXSetRelative:
        {
            auto member = std::mem_fn(&EFX::setIsRelative);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toBool());
        }
        break;
        case EFXSetWidth:
        {
            auto member = std::mem_fn(&EFX::setWidth);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;
        case EFXSetHeight:
        {
            auto member = std::mem_fn(&EFX::setHeight);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;
        case EFXSetXOffset:
        {
            auto member = std::mem_fn(&EFX::setXOffset);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;
        case EFXSetYOffset:
        {
            auto member = std::mem_fn(&EFX::setYOffset);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;
        case EFXSetRotation:
        {
            auto member = std::mem_fn(&EFX::setRotation);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;
        case EFXSetStartOffset:
        {
            auto member = std::mem_fn(&EFX::setStartOffset);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;
        case EFXSetXFrequency:
        {
            auto member = std::mem_fn(&EFX::setXFrequency);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;
        case EFXSetYFrequency:
        {
            auto member = std::mem_fn(&EFX::setYFrequency);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;
        case EFXSetXPhase:
        {
            auto member = std::mem_fn(&EFX::setXPhase);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;
        case EFXSetYPhase:
        {
            auto member = std::mem_fn(&EFX::setYPhase);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), value->toInt());
        }
        break;

        /* ******************* Virtual console editing actions ******************** */

        case VCWidgetCreate:
        {
            processBufferedAction(undo ? VCWidgetDelete : VCWidgetCreate, action.m_objID, action.m_newValue);
            return undo ? VCWidgetDelete : VCWidgetCreate;
        }
        break;
        case VCWidgetDelete:
        {
            processBufferedAction(undo ? VCWidgetCreate : VCWidgetDelete, action.m_objID, action.m_oldValue);
            return undo ? VCWidgetCreate : VCWidgetDelete;
        }
        break;
        case VCWidgetGeometry:
        {
            auto member = std::mem_fn(&VCWidget::setGeometry);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), value->toRectF());
        }
        break;
        case VCWidgetCaption:
        {
            auto member = std::mem_fn(&VCWidget::setCaption);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), value->toString());
        }
        break;
        case VCWidgetBackgroundColor:
        {
            auto member = std::mem_fn(&VCWidget::setBackgroundColor);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), value->value<QColor>());
        }
        break;
        case VCWidgetBackgroundImage:
        {
            auto member = std::mem_fn(&VCWidget::setBackgroundImage);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), value->toString());
        }
        break;
        case VCWidgetForegroundColor:
        {
            auto member = std::mem_fn(&VCWidget::setForegroundColor);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), value->value<QColor>());
        }
        break;
        case VCWidgetFont:
        {
            auto member = std::mem_fn(&VCWidget::setFont);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), value->value<QFont>());
        }
        break;
        default:
            qWarning() << "Action" << action.m_action << "not implemented !";
        break;
    }

    return action.m_action;
}


