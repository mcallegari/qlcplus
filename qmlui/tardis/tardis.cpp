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

    bool done = false;

    m_busy = true;

    while (!done)
    {
        TardisAction action = m_history.takeLast();
        qDebug("Undo action 0x%02X", action.m_action);

        processAction(action);

        /* If there are active network connections, send the action there too */
        if (m_networkManager->connectionsCount())
        {
            QMetaObject::invokeMethod(m_networkManager, "sendAction", Qt::QueuedConnection,
                    Q_ARG(quint32, action.m_objID),
                    Q_ARG(TardisAction, action));
        }

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
            m_history.removeFirst();

        qDebug("Got action: 0x%02X, history length: %d (%d)", action.m_action, m_historyCount, m_history.count());

        /* If there are active network connections, send the action there too */
        if (m_networkManager->connectionsCount())
        {
            QMetaObject::invokeMethod(m_networkManager, "sendAction", Qt::QueuedConnection,
                    Q_ARG(quint32, action.m_objID),
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
        case FunctionCreate:
        case FunctionDelete:
        {
            Function *function = qobject_cast<Function *>(m_doc->function(objID));
            if (function)
                function->saveXML(&xmlWriter);
        }
        break;
        case ChaserAddStep:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(objID));
            ChaserStep *step = chaser->stepAt(data.toInt());
            step->saveXML(&xmlWriter, data.toInt(), chaser->type() == Function::SequenceType ? true : false);
        }
        break;
        case EFXAddFixture:
        {
            // EFXFixture reference is stored on data, so let's C-cast the QVariant value
            EFXFixture *fixture = (EFXFixture *)data.value<void *>();
            fixture->saveXML(&xmlWriter);
        }
        break;
        case VCWidgetCreate:
        {
            VCWidget *widget = qobject_cast<VCWidget *>(m_virtualConsole->widget(data.toUInt()));
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

bool Tardis::processBufferedAction(TardisAction &action, QVariant &value)
{
    if (value.type() != QVariant::ByteArray)
        return false;

    QBuffer buffer;
    buffer.setData(value.toByteArray());
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    qDebug() << "Data to process:" << value.toString();

    switch(action.m_action)
    {
        case FixtureCreate:
        {
            Fixture::loader(xmlReader, m_doc);
        }
        break;
        case FixtureDelete:
        {
            m_fixtureManager->deleteFixtures(QVariantList( { action.m_objID } ));
        }
        break;
        case FunctionCreate:
        {
            Function::loader(xmlReader, m_doc);
        }
        break;
        case FunctionDelete:
        {
            m_functionManager->deleteFunctions(QVariantList( { action.m_objID } ));
        }
        break;
        case ChaserAddStep:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(action.m_objID));
            ChaserStep step;
            int stepNumber = -1;

            if (step.loadXML(xmlReader, stepNumber) == true)
                chaser->addStep(step, stepNumber);
        }
        break;
        case EFXAddFixture:
        {
            EFX *efx = qobject_cast<EFX *>(m_doc->function(action.m_objID));
            EFXFixture *ef = new EFXFixture(efx);

            ef->loadXML(xmlReader);
            efx->addFixture(ef);
        }
        break;
        case VCWidgetCreate:
        {
            VCFrame *frame = qobject_cast<VCFrame *>(m_virtualConsole->widget(action.m_objID));
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
    TardisAction action;
    action.m_action = code;
    action.m_objID = id;

    // Handle creation cases, where an XML fragment is provided
    if (processBufferedAction(action, value))
        return;

    // store value on oldValue, since we're going to call the method used for undoing actions
    action.m_oldValue = value;

    // process the action
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
        case FixtureDelete:
        {
            action.m_action = FixtureCreate; // reverse the action
            processBufferedAction(action, action.m_oldValue);
        }
        break;
        case FixtureSetPosition:
        {
            Fixture *fixture = m_doc->fixture(action.m_objID);
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
        case FunctionDelete:
        {
            action.m_action = FunctionCreate; // reverse the action
            processBufferedAction(action, action.m_oldValue);
        }
        break;
        case FunctionSetName:
        {
            auto member = std::mem_fn(&Function::setName);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), action.m_oldValue.toString());
        }
        break;
        case FunctionSetTempoType:
        {
            auto member = std::mem_fn(&Function::setTempoType);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), Function::TempoType(action.m_oldValue.toInt()));
        }
        break;
        case FunctionSetRunOrder:
        {
            auto member = std::mem_fn(&Function::setRunOrder);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), Function::RunOrder(action.m_oldValue.toInt()));
        }
        break;
        case FunctionSetDirection:
        {
            auto member = std::mem_fn(&Function::setDirection);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), Function::Direction(action.m_oldValue.toInt()));
        }
        break;
        case FunctionSetFadeIn:
        {
            auto member = std::mem_fn(&Function::setFadeInSpeed);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), action.m_oldValue.toUInt());
        }
        break;
        case FunctionSetFadeOut:
        {
            auto member = std::mem_fn(&Function::setFadeOutSpeed);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), action.m_oldValue.toUInt());
        }
        break;
        case FunctionSetDuration:
        {
            auto member = std::mem_fn(&Function::setDuration);
            member(qobject_cast<Function *>(m_doc->function(action.m_objID)), action.m_oldValue.toUInt());
        }
        break;

        case SceneSetChannelValue:
        case SceneUnsetChannelValue:
        {
            SceneValue scv = action.m_oldValue.value<SceneValue>();
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
            UIntPair pairValue = action.m_oldValue.value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.fadeIn = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;
        case ChaserSetStepHold:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(action.m_objID));
            UIntPair pairValue = action.m_oldValue.value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.hold = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;
        case ChaserSetStepFadeOut:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(action.m_objID));
            UIntPair pairValue = action.m_oldValue.value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.fadeOut = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;
        case ChaserSetStepDuration:
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(action.m_objID));
            UIntPair pairValue = action.m_oldValue.value<UIntPair>(); // index on first, time on second
            ChaserStep step = chaser->steps().at(pairValue.first);
            step.duration = pairValue.second;
            chaser->replaceStep(step, pairValue.first);
        }
        break;

        case EFXAddFixture:
        {
            auto member = std::mem_fn(&EFX::removeFixture);
            EFXFixture *ef = (EFXFixture *)action.m_oldValue.value<void *>();
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), ef);
        }
        break;
        case EFXSetAlgorithmIndex:
        {
            auto member = std::mem_fn(&EFX::setAlgorithm);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), EFX::Algorithm(action.m_oldValue.toInt()));
        }
        break;
        case EFXSetRelative:
        {
            auto member = std::mem_fn(&EFX::setIsRelative);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toBool());
        }
        break;
        case EFXSetWidth:
        {
            auto member = std::mem_fn(&EFX::setWidth);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;
        case EFXSetHeight:
        {
            auto member = std::mem_fn(&EFX::setHeight);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;
        case EFXSetXOffset:
        {
            auto member = std::mem_fn(&EFX::setXOffset);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;
        case EFXSetYOffset:
        {
            auto member = std::mem_fn(&EFX::setYOffset);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;
        case EFXSetRotation:
        {
            auto member = std::mem_fn(&EFX::setRotation);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;
        case EFXSetStartOffset:
        {
            auto member = std::mem_fn(&EFX::setStartOffset);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;
        case EFXSetXFrequency:
        {
            auto member = std::mem_fn(&EFX::setXFrequency);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;
        case EFXSetYFrequency:
        {
            auto member = std::mem_fn(&EFX::setYFrequency);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;
        case EFXSetXPhase:
        {
            auto member = std::mem_fn(&EFX::setXPhase);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;
        case EFXSetYPhase:
        {
            auto member = std::mem_fn(&EFX::setYPhase);
            member(qobject_cast<EFX *>(m_doc->function(action.m_objID)), action.m_oldValue.toInt());
        }
        break;

        /* ******************* Virtual console editing actions ******************** */

        case VCWidgetCreate:
        {
            m_virtualConsole->deleteVCWidgets(QVariantList( { action.m_newValue } ));
        }
        break;
        case VCWidgetDelete:
        {
            action.m_action = VCWidgetCreate; // reverse the action
            processBufferedAction(action, action.m_oldValue);
        }
        break;
        case VCWidgetGeometry:
        {
            auto member = std::mem_fn(&VCWidget::setGeometry);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), action.m_oldValue.toRectF());
        }
        break;
        case VCWidgetCaption:
        {
            auto member = std::mem_fn(&VCWidget::setCaption);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), action.m_oldValue.toString());
        }
        break;
        case VCWidgetBackgroundColor:
        {
            auto member = std::mem_fn(&VCWidget::setBackgroundColor);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), action.m_oldValue.value<QColor>());
        }
        break;
        case VCWidgetBackgroundImage:
        {
            auto member = std::mem_fn(&VCWidget::setBackgroundImage);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), action.m_oldValue.toString());
        }
        break;
        case VCWidgetForegroundColor:
        {
            auto member = std::mem_fn(&VCWidget::setForegroundColor);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), action.m_oldValue.value<QColor>());
        }
        break;
        case VCWidgetFont:
        {
            auto member = std::mem_fn(&VCWidget::setFont);
            member(qobject_cast<VCWidget *>(m_virtualConsole->widget(action.m_objID)), action.m_oldValue.value<QFont>());
        }
        break;
        default:
            qWarning() << "Action" << action.m_action << "not implemented !";
        break;
    }
}


