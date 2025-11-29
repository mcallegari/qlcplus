/*
  Q Light Controller Plus
  rdmmanager.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing software
  distributed under the License is distributed on an "AS IS" BASIS
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QQmlContext>

#include "rdmmanager.h"
#include "outputpatch.h"
#include "qlcioplugin.h"
#include "rdmprotocol.h"
#include "listmodel.h"
#include "doc.h"

#define MAX_WAIT_COUNT      30

RDMManager::RDMManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_discoveryStatus(Idle)
    , m_fixturesFound(0)
{
    m_view->rootContext()->setContextProperty("rdmManager", this);
    qmlRegisterUncreatableType<RDMManager>("org.qlcplus.classes", 1, 0, "RDMManager", "Can't create a RDMManager!");

    m_fixtureList = new ListModel(this);
    QStringList listRoles;
    listRoles << "universe" << "manufacturer" << "model" << "dmxAddress" << "dmxChannels" << "personality" << "UID" << "isSelected";
    m_fixtureList->setRoleNames(listRoles);
}

RDMManager::~RDMManager()
{
}

void RDMManager::startDiscovery()
{
    m_fixturesFound = 0;
    setDiscoveryStatus(Running);

    // go through every universe and launch a RDM discovery for each
    // patched plugin which supports the RDM standard
    foreach (Universe *uni, m_doc->inputOutputMap()->universes())
    {
        for (int i = 0; i < uni->outputPatchesCount(); i++)
        {
            OutputPatch *op = uni->outputPatch(i);
            if (op->plugin()->capabilities() & QLCIOPlugin::RDM)
            {
                RDMWorker *wt = new RDMWorker(m_doc);
                connect(wt, SIGNAL(uidFound(QString, UIDInfo)),
                        this, SLOT(slotUIDFound(QString, UIDInfo)));
                connect(wt, SIGNAL(finished()),
                        this, SLOT(slotTaskFinished()));
                wt->runDiscovery(uni->id(), op->output());
            }
        }
    }
}

void RDMManager::requestDMXAddress(QString UID, int address)
{
    qDebug() << "[RDM] Request address" << address << "for UID" << UID;
    UIDInfo info = m_uidMap.value(UID);
    info.dmxAddress = address;
    info.operation |= SetDMXAddress;
    m_uidMap[UID] = info;
}

void RDMManager::executeOperations()
{
    // Group operations by (universe, plugin line) so each worker talks to 1 line
    QHash<QPair<quint32, quint32>, QMap<QString, UIDInfo>> opsPerPatch;

    for (auto it = m_uidMap.cbegin(); it != m_uidMap.cend(); ++it)
    {
        const QString uid = it.key();
        const UIDInfo &info = it.value();

        if (info.operation == 0)
            continue;

        QPair<quint32, quint32> key(info.universe, info.pluginLine);
        opsPerPatch[key].insert(uid, info);
    }

    if (opsPerPatch.isEmpty())
        return;

    // For every universe/output patch that supports RDM, spawn a worker
    foreach (Universe *uni, m_doc->inputOutputMap()->universes())
    {
        for (int i = 0; i < uni->outputPatchesCount(); i++)
        {
            OutputPatch *op = uni->outputPatch(i);
            if (op == nullptr)
                continue;

            if (!(op->plugin()->capabilities() & QLCIOPlugin::RDM))
                continue;

            QPair<quint32, quint32> key(uni->id(), op->output());
            if (!opsPerPatch.contains(key))
                continue;

            RDMWorker *wt = new RDMWorker(m_doc);
            connect(wt, SIGNAL(uidFound(QString, UIDInfo)),
                    this, SLOT(slotUIDFound(QString, UIDInfo)));
            connect(wt, SIGNAL(finished()),
                    wt, SLOT(deleteLater()));

            // Launch operations for this patch
            wt->runOperations(uni->id(), op->output(), opsPerPatch.value(key));
        }
    }

    // Locally clear the operations bitmask now that they’ve been queued
    for (auto it = m_uidMap.begin(); it != m_uidMap.end(); ++it)
        it.value().operation = 0;

    // Our DMX address values in m_uidMap are already updated by requestDMXAddress
    // so we can refresh the fixture list right away.
    updateFixtureList();
}

int RDMManager::fixturesFound()
{
    return m_fixturesFound;
}

QVariant RDMManager::fixtureList()
{
    return QVariant::fromValue(m_fixtureList);
}

void RDMManager::setDiscoveryStatus(DiscoveryStatus status)
{
    if (status == m_discoveryStatus)
        return;

    m_discoveryStatus = status;
    emit discoveryStatusChanged();
}

void RDMManager::slotUIDFound(QString UID, UIDInfo info)
{
    m_uidMap[UID] = info;
    m_fixturesFound = m_uidMap.count();

    emit fixturesFoundChanged();
}

void RDMManager::slotTaskFinished()
{
    setDiscoveryStatus(Finished);
    updateFixtureList();
}

RDMManager::DiscoveryStatus RDMManager::discoveryStatus() const
{
    return m_discoveryStatus;
}

void RDMManager::updateFixtureList()
{
    m_fixtureList->clear();

    for (auto i = m_uidMap.cbegin(), end = m_uidMap.cend(); i != end; ++i)
    {
        UIDInfo info = i.value();

        QVariantMap fxMap;
        fxMap["universe"] = info.universe;
        fxMap["manufacturer"] = info.manufacturer;
        fxMap["model"] = info.name;
        fxMap["dmxAddress"] = info.dmxAddress;
        fxMap["dmxChannels"] = info.channels;
        fxMap["personality"] = info.personality;
        fxMap["UID"] = i.key();
        fxMap["isSelected"] = false;
        m_fixtureList->addDataMap(fxMap);
    }
    emit fixtureListChanged();
}

/************************************************************************
 * RDM worker implementation
 ************************************************************************/

RDMWorker::RDMWorker(Doc *doc)
    : m_doc(doc)
    , m_running(false)
    , m_requestState(StateNone)
{
}

RDMWorker::~RDMWorker()
{
    stop();
}

void RDMWorker::stop()
{
    if (m_running == true)
    {
        m_running = false;
        wait();
    }
}

void RDMWorker::addToQueue(quint8 command, const QVariantList &params)
{
    QVariantList cmd;
    cmd << int(command);           // store command type as first element
    for (const QVariant &v : params)
        cmd << v;

    m_rdmCommandsQueue.append(cmd);
/*
    qDebug() << "[RDM] addToQueue cmd" << command
             << "params:" << params
             << "queue size now:" << m_rdmCommandsQueue.size();
*/
}

void RDMWorker::checkQueue()
{
    if (m_rdmCommandsQueue.isEmpty() || m_plugin == nullptr)
        return;

    QVariantList cmd = m_rdmCommandsQueue.takeFirst();
    if (cmd.isEmpty())
        return;

    quint8 command = quint8(cmd.takeFirst().toInt());

    // If this is a discovery branch, remember the range we’re probing
    if (command == DISCOVERY_COMMAND && cmd.size() >= 4)
    {
        // cmd[0] = address, cmd[1] = PID, cmd[2] = startUID, cmd[3] = endUID
        if (cmd[1].toUInt() == PID_DISC_UNIQUE_BRANCH)
        {
            m_currentStartUID = cmd[2].toULongLong();
            m_currentEndUID   = cmd[3].toULongLong();
/*
            qDebug() << "[RDM] set current discovery range:"
                     << QString::number(m_currentStartUID, 16)
                     << "->"
                     << QString::number(m_currentEndUID, 16);
*/
        }
    }

    m_plugin->sendRDMCommand(m_universe, m_line, command, cmd);
    m_requestState = StateWait;
}

void RDMWorker::runDiscovery(quint32 uni, quint32 line)
{
    m_universe = uni;
    m_line = line;

    m_rdmCommandsQueue.clear();

    // Initial full-range discovery
    m_currentStartUID = 0;
    m_currentEndUID = (qulonglong(BROADCAST_ESTA_ID) << 32) +
                      qulonglong(BROADCAST_DEVICE_ID);

    addToQueue(DISCOVERY_COMMAND,
               QVariantList() << RDMProtocol::broadcastAddress()
                              << PID_DISC_UNIQUE_BRANCH
                              << m_currentStartUID << m_currentEndUID);

    m_requestState = StateDiscoveryStart;
    start();
}

void RDMWorker::runOperations(quint32 uni, quint32 line,
                              const QMap<QString, UIDInfo> &operations)
{
    m_universe = uni;
    m_line = line;

    m_rdmCommandsQueue.clear();

    // Take a local copy of the map we need to operate on
    m_uidMap = operations;

    // Build the queue from the operation bitmask
    for (auto it = m_uidMap.begin(); it != m_uidMap.end(); ++it)
    {
        const QString uid = it.key();
        UIDInfo &info = it.value();

        // 1) Set DMX address
        if (info.operation & RDMManager::SetDMXAddress)
        {
            // PID_DMX_START_ADDRESS should be defined in rdmprotocol.h
            addToQueue(SET_COMMAND,
                       QVariantList() << uid
                                      << PID_DMX_START_ADDRESS << 2
                                      << quint16(info.dmxAddress));
        }

        // 2) Toggle identify
        if (info.operation & RDMManager::ToggleIdentify)
        {
            // Use UIDInfo::params["identify"] as current state (default = false)
            bool curIdentify = info.params.value("identify", false).toBool();
            bool newIdentify = !curIdentify;

            addToQueue(SET_COMMAND,
                       QVariantList() << uid
                                      << PID_IDENTIFY_DEVICE << 1
                                      << (newIdentify ? 1 : 0));

            info.params["identify"] = newIdentify;
        }

        // Clear operation bits after queuing
        info.operation = 0;
    }

    // Nothing to do? Just exit
    if (m_rdmCommandsQueue.isEmpty())
    {
        m_requestState = StateNone;
        return;
    }

    // Reuse the same phase/state used for GETs: drive the queue until empty
    m_requestState = StateGetFixtureInfo;
    start();
}

void RDMWorker::slotRDMDataReady(quint32 universe, quint32 line, QVariantMap data)
{
    //qDebug() << "Got signal from universe" << universe << ", line" << line;
    //qDebug() << "RDM map" << data;

    // check the signal reason
    if (data.contains("DISCOVERY_COUNT"))
    {
        int count = data.value("DISCOVERY_COUNT").toInt();
        for (int i = 0; i < count; i++)
        {
            QString UID = data.value(QString("UID-%1").arg(i)).toString();
            if (m_uidMap.contains(UID) == false)
            {
                UIDInfo info;
                info.universe = universe;
                info.pluginLine = line;
                info.operation = 0;
                m_uidMap[UID] = info;
                emit uidFound(UID, info);
            }
        }

        if (m_rdmCommandsQueue.isEmpty())
            m_requestState = StateDiscoveryEnd;
        else
            m_requestState = StateDiscoveryContinue;
    }
    else if (data.contains("DISCOVERY_ERRORS"))
    {
        // Discovery errors mean collisions and/or bad checksum.
        // Split the *current* range into two branches and enqueue them.

        if (m_currentStartUID >= m_currentEndUID)
        {
            // Nothing meaningful to split; just move on
            if (m_rdmCommandsQueue.isEmpty())
                m_requestState = StateDiscoveryEnd;
            else
                m_requestState = StateDiscoveryContinue;
            return;
        }

        qulonglong start = m_currentStartUID;
        qulonglong end   = m_currentEndUID;

        qulonglong midPosition =
            ((start & (0x0000800000000000 - 1)) +
             (end   & (0x0000800000000000 - 1))) / 2
            + ((end   & 0x0000800000000000) ? 0x0000400000000000 : 0)
            + ((start & 0x0000800000000000) ? 0x0000400000000000 : 0);

        qulonglong lowerStart = start;
        qulonglong lowerEnd   = midPosition;
        qulonglong upperStart = midPosition + 1;
        qulonglong upperEnd   = end;

        // add the two new branches to lookup
        addToQueue(DISCOVERY_COMMAND,
                   QVariantList() << RDMProtocol::broadcastAddress()
                                  << PID_DISC_UNIQUE_BRANCH
                                  << upperStart << upperEnd);

        addToQueue(DISCOVERY_COMMAND,
                   QVariantList() << RDMProtocol::broadcastAddress()
                                  << PID_DISC_UNIQUE_BRANCH
                                  << lowerStart << lowerEnd);

        m_requestState = StateDiscoveryContinue;
    }
    else if (data.contains("DISCOVERY_NO_REPLY"))
    {
        // No reply means a dead branch. Just continue with queued branches.
        qDebug() << "Discovery: no reply";

        if (m_rdmCommandsQueue.isEmpty())
            m_requestState = StateDiscoveryEnd;
        else
            m_requestState = StateDiscoveryContinue;
    }
    else if (data.contains("UID_INFO"))
    {
        QString UID = data.value("UID_INFO").toString();
        UIDInfo info = m_uidMap.value(UID);
        bool infoUpdated = false;

        //qDebug() << "INFO RECEIVED FOR" << UID << "data" << data;

        for (QVariantMap::const_iterator it = data.begin(); it != data.end(); ++it)
        {
            const QString key = it.key();

            if (key == "MODEL_NAME")
            {
                info.name = it.value().toString();
            }
            else if (key == "MANUFACTURER")
            {
                info.manufacturer = it.value().toString();
            }
            else if (key == "DMX_CHANNELS")
            {
                info.channels = it.value().toUInt();
            }
            else if (key == "PERSONALITY_INDEX")
            {
                info.personalityIndex = it.value().toUInt();
                addToQueue(GET_COMMAND,
                           QVariantList() << UID << PID_DMX_PERSONALITY_DESCRIPTION << 1 << info.personalityIndex);
            }
            else if (key == "PERSONALITY_COUNT")
            {
                info.personalityCount = it.value().toUInt();
            }
            else if (key == "DMX_START_ADDRESS")
            {
                info.dmxAddress = it.value().toUInt();
            }
            else if (key == "PERS_DESC")
            {
                info.personality = it.value().toString();
                infoUpdated = true;
            }
        }

        // Store updated info
        m_uidMap[UID] = info;

        // Only notify UI when we have the full basic info (DMX address)
        if (infoUpdated)
            emit uidFound(UID, info);

        // Decide next state for the worker:
        // - if there are still queued commands, continue GetFixtureInfo phase
        // - otherwise we’re done with all GETs
        if (m_rdmCommandsQueue.isEmpty())
            m_requestState = StateNone;          // all GETs done
        else
            m_requestState = StateGetFixtureInfo; // continue info phase
    }

}

void RDMWorker::run()
{
    int waitCount = 0;

    Universe *uni = m_doc->inputOutputMap()->universe(m_universe);
    if (uni == NULL)
    {
        qDebug() << "ERROR. Universe not found!";
        return;
    }

    OutputPatch *op = NULL;
    for (int i = 0; i < uni->outputPatchesCount(); i++)
    {
        op = uni->outputPatch(i);
        if (op->output() == m_line)
            break;
    }
    if (op == NULL)
    {
        qDebug() << "ERROR. Output patch not found!";
        return;
    }
    m_plugin = op->plugin();

    connect(m_plugin, SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)),
            this, SLOT(slotRDMDataReady(quint32, quint32, QVariantMap)));

    m_running = true;
    while (m_running == true)
    {
        switch (m_requestState)
        {
        case StateNone:
        {
            // Nothing to do. Terminate the thread
            m_running = false;
        }
        break;
        case StateDiscoveryStart:
        {
            waitCount = 0;
            // Kick off the first queued discovery command
            checkQueue();
        }
        break;
        case StateDiscoveryContinue:
        {
            waitCount = 0;

            if (m_rdmCommandsQueue.isEmpty())
            {
                // Nothing more to probe
                m_requestState = StateDiscoveryEnd;
                break;
            }

            // Next queued discovery (or later, other RDM operations)
            checkQueue();
        }
        break;
        case StateDiscoveryEnd:
        {
            if (m_uidMap.isEmpty())
            {
                emit requestPopup("Warning", "No RDM devices found");
                m_requestState = StateNone;
            }
            else
            {
                // Queue basic info requests for all discovered UIDs
                for (auto it = m_uidMap.cbegin(); it != m_uidMap.cend(); ++it)
                {
                    const QString uid = it.key();

                    addToQueue(GET_COMMAND,
                               QVariantList() << uid << PID_DEVICE_MODEL_DESCRIPTION);

                    addToQueue(GET_COMMAND,
                               QVariantList() << uid << PID_MANUFACTURER_LABEL);

                    addToQueue(GET_COMMAND,
                               QVariantList() << uid << PID_DEVICE_INFO);
                }

                // Start the fixture info phase
                m_requestState = StateGetFixtureInfo;
            }
        }
        break;
        case StateGetFixtureInfo:
        {
            waitCount = 0;

            if (m_rdmCommandsQueue.isEmpty())
            {
                // All GETs done
                m_requestState = StateNone;
                break;
            }

            // Send next queued GET command
            checkQueue();
        }
        break;
        default:
        {
            //qDebug() << "[RDM] ....WAIT....";
            msleep(50);
            waitCount++;
            if (m_requestState == StateWait && waitCount == MAX_WAIT_COUNT)
            {
                qDebug() << "Exit for timeout...";
                emit requestPopup("Warning", "Process timed out");
                m_running = false;
            }
        }
        break;
        }
    }

    disconnect(m_plugin, SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)),
               this, SLOT(slotRDMDataReady(quint32, quint32, QVariantMap)));

    qDebug() << "Terminating RDM worker thread";
}
