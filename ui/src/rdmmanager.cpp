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

#include <QDebug>
#include <QMessageBox>

#include "rdmmanager.h"
#include "outputpatch.h"
#include "qlcioplugin.h"
#include "rdmprotocol.h"
#include "doc.h"

// RDM view column numbers
#define KColumnRDMModel    0
#define KColumnRDMUniverse 1
#define KColumnRDMAddress  2
#define KColumnRDMChannels 3
#define KColumnRDMUID      4

#define MAX_WAIT_COUNT  30

RDMManager::RDMManager(QWidget *parent, Doc *doc)
    : QWidget(parent)
    , m_doc(doc)
{
    setupUi(this);

    m_getInfoButton->setEnabled(false);
    m_readButton->setEnabled(false);

    connect(m_refreshButton, SIGNAL(clicked()), this, SLOT(slotRefresh()));
    connect(m_getInfoButton, SIGNAL(clicked()), this, SLOT(slotGetInfo()));
    connect(m_rdmTree, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectionChanged()));
    connect(m_readButton, SIGNAL(clicked()), this, SLOT(slotReadPID()));
    connect(m_writeButton, SIGNAL(clicked()), this, SLOT(slotWritePID()));
}

RDMManager::~RDMManager()
{
}

void RDMManager::slotRefresh()
{
    m_refreshButton->setEnabled(false);

    // reset any previously collected information
    m_rdmTree->clear();

    m_devFoundLabel->setText("Discovering fixtures...");

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
                        this, SLOT(updateRDMTreeItem(QString, UIDInfo)));
                connect(wt, SIGNAL(requestPopup(QString, QString)),
                        this, SLOT(slotDisplayPopup(QString, QString)));
                connect(wt, SIGNAL(finished()),
                        this, SLOT(slotTaskFinished()));
                wt->runDiscovery(uni->id(), op->output());
            }
        }
    }
}

bool RDMManager::getPluginInfo(quint32 universe, quint32 line, quint32 &universeID, quint32 &outputLine)
{
    Universe *uni = m_doc->inputOutputMap()->universe(universe);
    if (uni == NULL)
    {
        qDebug() << "ERROR. Universe not found!";
        return false;
    }

    OutputPatch *op = NULL;
    for (int i = 0; i < uni->outputPatchesCount(); i++)
    {
        op = uni->outputPatch(i);
        if (op->output() == line)
            break;
    }
    if (op == NULL)
    {
        qDebug() << "ERROR. Output patch not found!";
        return false;
    }

    universeID = uni->id();
    outputLine = op->output();

    return true;
}

void RDMManager::slotGetInfo()
{
    QTreeWidgetItem *item = m_rdmTree->selectedItems().first();
    QString UID = item->text(KColumnRDMUID);
    UIDInfo info = m_uidMap.value(UID);
    quint32 uniID = 0, outLine = 0;

    if (getPluginInfo(info.universe, info.pluginLine, uniID, outLine) == false)
    {
        qDebug() << "ERROR. Cannot get plugin info";
        return;
    }

    RDMWorker *wt = new RDMWorker(m_doc);
    connect(wt, SIGNAL(fixtureInfoReady(QString&)), this, SIGNAL(fixtureInfoReady(QString&)));
    connect(wt, SIGNAL(requestPopup(QString, QString)), this, SLOT(slotDisplayPopup(QString, QString)));
    wt->getUidInfo(uniID, outLine, UID, info);
}

void RDMManager::slotReadPID()
{
    QTreeWidgetItem *item = m_rdmTree->selectedItems().first();
    QString UID = item->text(KColumnRDMUID);
    UIDInfo info = m_uidMap.value(UID);
    quint32 uniID = 0, outLine = 0;
    QVariantList params;

    if (getPluginInfo(info.universe, info.pluginLine, uniID, outLine) == false)
    {
        qDebug() << "ERROR. Cannot get plugin info";
        return;
    }

    m_pidResult->clear();
    QString args = m_pidArgsEdit->text().toLower();
    bool ok;

    if (args.length())
    {
        switch(m_dataTypeCombo->currentIndex())
        {
            case ByteArg:
                params.append(uchar(1));
                if (args.startsWith("0x"))
                    params.append(uchar(args.mid(2).toUShort(&ok, 16)));
                else
                    params.append(uchar(args.toUShort()));
            break;
            case ShortArg:
                params.append(uchar(2));
                if (args.startsWith("0x"))
                    params.append(args.mid(2).toUShort(&ok, 16));
                else
                    params.append(args.toShort());
            break;
            case LongArg:
                params.append(uchar(4));
                if (args.startsWith("0x"))
                    params.append(quint32(args.mid(2).toULong(&ok, 16)));
                else
                    params.append(quint32(args.toULong()));
            break;
            case ArrayArg:
                params.append(uchar(99));
                foreach (QString arg, args.split(","))
                    params.append(uchar(arg.toUShort(&ok, 16)));
            break;
        }
    }

    RDMWorker *wt = new RDMWorker(m_doc);
    connect(wt, SIGNAL(requestPopup(QString, QString)), this, SLOT(slotDisplayPopup(QString, QString)));
    connect(wt, SIGNAL(pidInfoReady(QString)), this, SLOT(slotUpdatePidInfo(QString)));
    wt->handlePID(uniID, outLine, UID, m_pidEdit->text(), params, false);
}

void RDMManager::slotWritePID()
{
    QTreeWidgetItem *item = m_rdmTree->selectedItems().first();
    QString UID = item->text(KColumnRDMUID);
    UIDInfo info = m_uidMap.value(UID);
    quint32 uniID = 0, outLine = 0;
    QVariantList params;

    if (getPluginInfo(info.universe, info.pluginLine, uniID, outLine) == false)
    {
        qDebug() << "ERROR. Cannot get plugin info";
        return;
    }

    m_pidResult->clear();

    if (m_pidArgsEdit->text().length())
    {
        QStringList argList = m_pidArgsEdit->text().split(",");
        bool ok;

        if (m_dataTypeCombo->currentIndex() == ArrayArg)
        {
            QByteArray baArg;

            params.append(uchar(99)); // special size for array

            for (int i = 0; i < argList.count(); i++)
                baArg.append(QByteArray::fromHex(argList.at(i).toUtf8()));

            params.append(baArg);
        }
        else
        {
            for (int i = 0; i < argList.count(); i++)
            {
                QString arg = argList.at(i);

                switch(m_dataTypeCombo->currentIndex())
                {
                    case ByteArg:
                        params.append(uchar(1));
                        if (arg.toLower().startsWith("0x"))
                            params.append(uchar(arg.mid(2).toUShort(&ok, 16)));
                        else
                            params.append(uchar(arg.toUShort()));
                    break;
                    case ShortArg:
                        params.append(uchar(2));
                        if (arg.toLower().startsWith("0x"))
                            params.append(arg.mid(2).toShort(&ok, 16));
                        else
                            params.append(arg.toShort());
                    break;
                    case LongArg:
                        params.append(uchar(4));
                        if (arg.toLower().startsWith("0x"))
                            params.append(quint32(arg.mid(2).toULong(&ok, 16)));
                        else
                            params.append(quint32(arg.toULong()));
                    break;
                }
            }
        }
    }

    RDMWorker *wt = new RDMWorker(m_doc);
    connect(wt, SIGNAL(requestPopup(QString, QString)), this, SLOT(slotDisplayPopup(QString, QString)));
    connect(wt, SIGNAL(pidInfoReady(QString)), this, SLOT(slotUpdatePidInfo(QString)));
    wt->handlePID(uniID, outLine, UID, m_pidEdit->text(), params, true);
}

void RDMManager::slotSelectionChanged()
{
    int selectedCount = m_rdmTree->selectedItems().size();

    m_getInfoButton->setEnabled(selectedCount ? true : false);
    m_readButton->setEnabled(selectedCount ? true : false);
}

void RDMManager::slotUpdatePidInfo(QString info)
{
    m_pidResult->setText(info);
}

void RDMManager::slotDisplayPopup(QString title, QString message)
{
    QMessageBox::information(this, title, message);
    m_refreshButton->setEnabled(true);
}

void RDMManager::slotTaskFinished()
{
    m_refreshButton->setEnabled(true);
}

void RDMManager::updateRDMTreeItem(QString UID, UIDInfo info)
{
    QTreeWidgetItem *item = NULL;

    qDebug() << "Got info for UID" << UID;

    m_uidMap[UID] = info;

    for (int i = 0; i < m_rdmTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *tlItem = m_rdmTree->topLevelItem(i);
        QString itemUID = tlItem->text(KColumnRDMUID);
        if (itemUID == UID)
        {
            item = tlItem;
            break;
        }
    }

    if (item == NULL)
    {
        item = new QTreeWidgetItem(m_rdmTree);
        item->setText(KColumnRDMUID, UID);
    }

    item->setText(KColumnRDMModel, QString ("%1 - %2").arg(info.manufacturer).arg(info.name));
    item->setText(KColumnRDMUniverse, QString::number(info.universe + 1));
    item->setText(KColumnRDMAddress,  QString::number(info.dmxAddress));
    item->setText(KColumnRDMChannels, QString::number(info.channels));

    m_rdmTree->header()->resizeSections(QHeaderView::ResizeToContents);

    if (m_rdmTree->topLevelItemCount())
        m_devFoundLabel->setText(QString("Fixtures found: %1").arg(m_rdmTree->topLevelItemCount()));
    else
        m_devFoundLabel->setText("No fixtures found");
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

void RDMWorker::runDiscovery(quint32 uni, quint32 line)
{
    m_universe = uni;
    m_line = line;

    DiscoveryInfo info;
    info.startUID = 0;
    info.endUID = (qulonglong(BROADCAST_ESTA_ID) << 32) + qulonglong(BROADCAST_DEVICE_ID);
    m_discoveryList.append(info);

    m_requestState = StateDiscoveryStart;
    start();
}

void RDMWorker::getUidInfo(quint32 uni, quint32 line, QString UID, UIDInfo &info)
{
    m_universe = uni;
    m_line = line;
    m_uidMap[UID] = info;

    QPalette pal;
    QColor hlBack(pal.color(QPalette::Highlight));
    QColor hlText(pal.color(QPalette::HighlightedText));

    // initialize the fixture information
    m_fixtureInfo = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
    m_fixtureInfo += "<HTML><HEAD></HEAD><STYLE>";
    m_fixtureInfo += QString(".hilite {" \
                             "	background-color: %1;" \
                             "	color: %2;" \
                             "	font-size: x-large;" \
                             "}").arg(hlBack.name()).arg(hlText.name());
    m_fixtureInfo += QString(".subhi {" \
                             "	background-color: %1;" \
                             "	color: %2;" \
                             "	font-weight: bold;" \
                             "}").arg(hlBack.name()).arg(hlText.name());
    m_fixtureInfo += QString(".emphasis {" \
                             "	font-weight: bold;" \
                             "}");
    m_fixtureInfo += QString(".tiny {"\
                             "   font-size: small;" \
                             "}");
    m_fixtureInfo += "</STYLE>";

    QString title("<TR CLASS='hilite'><TD COLSPAN='2'>%1</TD><TD COLSPAN='2' ALIGN='right'>UID: %3</TD></TR>");
    QString genInfo("<TR><TD CLASS='emphasis'>%1</TD><TD COLSPAN='3'>%2</TD></TR>");

    /********************************************************************
     * General info
     ********************************************************************/

    m_fixtureInfo += "<TABLE COLS='4' WIDTH='100%'>";

    // Fixture title
    m_fixtureInfo += title.arg(info.name).arg(UID);

    // Manufacturer
    m_fixtureInfo += genInfo.arg(tr("Manufacturer")).arg(info.manufacturer);
    m_fixtureInfo += genInfo.arg(tr("Model")).arg(info.name);
    //info += genInfo.arg(tr("Mode")).arg(m_fixtureMode->name());
    m_fixtureInfo += genInfo.arg(tr("Type")).arg(info.params.value("TYPE").toString());

    // Universe
    m_fixtureInfo += genInfo.arg(tr("Universe")).arg(info.universe + 1);

    // Address
    QString range = QString("%1 - %2").arg(info.dmxAddress).arg(info.dmxAddress + info.channels);
    m_fixtureInfo += genInfo.arg(tr("Address Range")).arg(range);

    // Channels
    m_fixtureInfo += genInfo.arg(tr("Channels")).arg(info.channels);

    QString header("<TR CLASS='hilite'><TD COLSPAN='4'>%1</TD></TR>");
    m_fixtureInfo += header.arg(tr("Personalities"));

    m_requestState = StatePersonalities;
    start();
}

void RDMWorker::handlePID(quint32 uni, quint32 line, QString UID, QString pid, QVariantList args, bool write)
{
    m_universe = uni;
    m_line = line;
    UIDInfo info;
    bool ok;

    if (pid.toLower().startsWith("0x"))
        info.dmxAddress = pid.mid(2).toUInt(&ok, 16);
    else
        info.dmxAddress = pid.toUInt(&ok, 10);

    if (ok == false)
    {
        emit requestPopup("Error", "Invalid PID entered!");
        return;
    }

    if (args.length())
    {
        for (int i = 0; i < args.count(); i++)
            info.params.insert(QString::number(i), args.at(i));
    }

    m_uidMap[UID] = info;

    if (write)
        m_requestState = StateWriteSinglePid;
    else
        m_requestState = StateReadSinglePid;

    start();
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
                m_requestState = StateDiscoveryContinue;
                //m_plugin->sendRDMCommand(m_universe, m_line, DISCOVERY_COMMAND,
                //                         QVariantList() << RDMProtocol::broadcastAddress() << PID_DISC_UN_MUTE);
            }
            break;
            case StateDiscoveryContinue:
            {
                waitCount = 0;

                if (m_discoveryList.isEmpty())
                {
                    m_requestState = StateDiscoveryEnd;
                    break;
                }

                DiscoveryInfo info = m_discoveryList.first();
                qDebug() << "Discovery - CONTINUE between" << QString::number(info.startUID, 16)
                         << "and" << QString::number(info.endUID, 16);
                m_plugin->sendRDMCommand(m_universe, m_line, DISCOVERY_COMMAND,
                                         QVariantList() << RDMProtocol::broadcastAddress()
                                                        << PID_DISC_UNIQUE_BRANCH << info.startUID << info.endUID);

                m_requestState = StateWait;
            }
            break;
            case StateDiscoveryEnd:
            {
                if (m_uidMap.isEmpty())
                    emit requestPopup("Warning", "No RDM devices found");
                m_requestState = StateNone;
            }
            break;
            case StatePersonalities:
            {
                waitCount = 0;
                QString UID = m_uidMap.firstKey();
                m_requestState = StatePersonalityInfo;
                qDebug() << "Requesting personalities of UID" << UID;
                bool result = m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                                       QVariantList() << UID << PID_DMX_PERSONALITY);
                if (result == false)
                {
                    requestPopup("Error", "RDM command failed");
                    m_requestState = StateNone;
                }
            }
            break;
            case StateReadSinglePid:
            {
                waitCount = 0;
                UIDInfo info = m_uidMap.first();
                QString UID = m_uidMap.firstKey();
                m_requestState = StateWaitPidInfo;
                qDebug().nospace().noquote() << "Read PID 0x" << QString::number(info.dmxAddress, 16);
                QVariantList args;
                args << UID;
                args << info.dmxAddress; // actually the PID to read
                for (QVariantMap::const_iterator it = info.params.begin(); it != info.params.end(); ++it)
                    args << it.value(); // add parameters

                uchar command = info.dmxAddress < 0x04 ? DISCOVERY_COMMAND : GET_COMMAND;
                bool result = m_plugin->sendRDMCommand(m_universe, m_line, command, args);
                if (result == false)
                {
                    requestPopup("Error", "RDM command failed");
                    m_requestState = StateNone;
                }
            }
            break;
            case StateWriteSinglePid:
            {
                waitCount = 0;
                UIDInfo info = m_uidMap.first();
                QString UID = m_uidMap.firstKey();
                m_requestState = StateWaitPidInfo;
                qDebug().nospace().noquote() << "Write PID 0x" << QString::number(info.dmxAddress, 16);
                QVariantList args;
                args << UID;
                args << info.dmxAddress; // actually the PID to write
                for (QVariantMap::const_iterator it = info.params.begin(); it != info.params.end(); ++it)
                    args << it.value(); // forward parameters

                bool result = m_plugin->sendRDMCommand(m_universe, m_line, SET_COMMAND, args);
                if (result == false)
                {
                    requestPopup("Error", "RDM command failed");
                    m_requestState = StateNone;
                }
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

void RDMWorker::stop()
{
    if (m_running == true)
    {
        m_running = false;
        wait();
    }
}

void RDMWorker::slotRDMDataReady(quint32 universe, quint32 line, QVariantMap data)
{
    //qDebug() << "Got signal from universe" << universe << ", line" << line;
    //qDebug() << "RDM map" << data;

    if (m_requestState == StateWaitPidInfo)
    {
        QString pidInfo;
        for (QVariantMap::const_iterator it = data.begin(); it != data.end(); ++it)
            pidInfo += QString("<b>%1</b>: %2<br>").arg(it.key()).arg(it.value().toString());

        emit pidInfoReady(pidInfo);
        m_requestState = StateNone;
        return;
    }

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
                m_uidMap[UID] = info;

                m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                         QVariantList() << UID << PID_DEVICE_MODEL_DESCRIPTION);

                m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                         QVariantList() << UID << PID_MANUFACTURER_LABEL);

                m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                         QVariantList() << UID << PID_DEVICE_INFO);
            }
        }
    }
    else if (data.contains("DISCOVERY_ERRORS"))
    {
        if (m_discoveryList.isEmpty())
            return;

        // Discovery errors mean collisions and/or bad checksum.
        // Split the current range into two branches
        DiscoveryInfo currentRange = m_discoveryList.first();
        DiscoveryInfo lowerRange, upperRange;

        qulonglong midPosition = ((currentRange.startUID & (0x0000800000000000-1)) +
                                  (currentRange.endUID & (0x0000800000000000-1))) / 2
                                + ((currentRange.endUID & 0x0000800000000000) ? 0x0000400000000000 : 0)
                                + ((currentRange.startUID & 0x0000800000000000) ? 0x0000400000000000 :0);
        lowerRange.startUID = currentRange.startUID;
        lowerRange.endUID = midPosition;
        upperRange.startUID = midPosition + 1;
        upperRange.endUID = currentRange.endUID;

        qDebug() << "Discovery errors detected" << data.value("DISCOVERY_ERRORS").toInt();
        //qDebug() << "Add lower range" << QString::number(lowerRange.startUID, 16) << "-" << QString::number(lowerRange.endUID, 16);
        //qDebug() << "Add upper range" << QString::number(upperRange.startUID, 16) << "-" << QString::number(upperRange.endUID, 16);
        m_discoveryList.removeFirst();
        m_discoveryList.prepend(lowerRange);
        m_discoveryList.prepend(upperRange);
        m_requestState = StateDiscoveryContinue;
    }
    else if (data.contains("DISCOVERY_NO_REPLY"))
    {
        if (m_discoveryList.isEmpty())
            return;

        // No reply means a dead branch. Remove it and continue on other branches.
        qDebug() << "Discovery: no reply";
        m_discoveryList.removeFirst();
        if (m_discoveryList.isEmpty())
            m_requestState = StateDiscoveryEnd;
        else
            m_requestState = StateDiscoveryContinue;
    }
    else if (data.contains("UID_INFO"))
    {
        QString UID = data.value("UID_INFO").toString();
        UIDInfo info = m_uidMap.value(UID);
        for (QVariantMap::const_iterator it = data.begin(); it != data.end(); ++it)
        {
            QString key = it.key();

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
            else if (key == "DMX_START_ADDRESS")
            {
                info.dmxAddress = it.value().toUInt();
                m_uidMap[UID] = info;
                emit uidFound(UID, info);
                if (m_discoveryList.isEmpty())
                {
                    m_requestState = StateDiscoveryEnd;
                }
                else
                {
                    //m_discoveryList.removeFirst();
                    m_requestState = StateDiscoveryContinue;
                }
            }
            else if (key == "PERS_COUNT")
            {
                for (quint16 p = 0; p < it.value().toUInt(); p++)
                    m_requestList.append(p + 1);
            }
            else if (key == "PERS_DESC")
            {
                quint16 cIndex = info.params.value("PERS_CURRENT").toUInt();
                quint16 pIndex = data.value("PERS_INDEX").toUInt();
                quint16 channels = data.value("PERS_CHANNELS").toUInt();
                QString label = it.value().toString();

                m_fixtureInfo += QString("<TR><TD CLASS='emphasis'>%1 %2 %3</TD><TD>%4</TD>")
                                .arg(tr("Personality")).arg(pIndex)
                                .arg(cIndex == pIndex ? tr("(Selected)") : "").arg(label);
                m_fixtureInfo += QString("<TD CLASS='emphasis'>%1</TD><TD>%2</TD></TR>")
                                        .arg(tr("Channels")).arg(channels);
            }
            else if (key == "SLOT_LIST")
            {
                QVariant var = it.value();
                if (m_requestList.isEmpty())
                {
                    QString header("<TR CLASS='hilite'><TD COLSPAN='4'>%1</TD></TR>");
                    m_fixtureInfo += header.arg(tr("Channel list"));
                }

                m_requestList.append(var.value<QVector<quint16>>());

                // check if channels don't fit in a single reply
                if (info.params.value("Response") == RDMProtocol::responseToString(RESPONSE_TYPE_ACK_OVERFLOW))
                {
                    m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                             QVariantList() << UID << PID_SLOT_INFO);
                }
                else
                {
                    m_requestState = StateSlots;
                }
            }
            else if (key == "SLOT_DESC")
            {
                quint16 slotId = data.value("SLOT_ID").toUInt();
                QString label = it.value().toString();

                qDebug() << "SLOT label" << label;

                m_fixtureInfo += QString("<TR><TD CLASS='emphasis'>%1 %2</TD><TD COLSPAN='3'>%3</TD></TR>")
                                         .arg(tr("Channel")).arg(slotId + 1).arg(label);
            }
            else if (key == "PID_LIST")
            {
                QVariant var = it.value();
                m_requestList = var.value<QVector<quint16>>();

                // remove PIDs handled elsewhere
                m_requestList.removeAll(PID_DMX_PERSONALITY);
                m_requestList.removeAll(PID_DMX_PERSONALITY_DESCRIPTION);
                m_requestList.removeAll(PID_SLOT_INFO);
                m_requestList.removeAll(PID_SLOT_DESCRIPTION);
                std::sort(m_requestList.begin(), m_requestList.end());

                QString header("<TR CLASS='hilite'><TD COLSPAN='4'>%1</TD></TR>");
                m_fixtureInfo += header.arg(tr("Supported PIDs"));
                m_requestState = StateSupportedPids;
            }
            else if (key == "PID")
            {
                if (m_requestState == StateSupportedPids)
                {
                    quint16 pid = it.value().toUInt();
                    if (pid < 0x8000 && pid != PID_PARAMETER_DESCRIPTION)
                    {
                        QString sPid = QString("%1").arg(pid, 4, 16, QChar('0'));
                        m_fixtureInfo += QString("<TR><TD CLASS='emphasis' COLSPAN='4'>PID: 0x%1 (%2)</TD></TR>")
                                .arg(sPid.toUpper()).arg(RDMProtocol::pidToString(pid));
                    }                }
                else if (m_requestState == StateReadSinglePid)
                {
                    // TODO
                }
            }
            else if (key == "PID_DESC")
            {
                if (m_requestState == StateSupportedPids)
                {
                    quint16 pid = data.value("PID_INFO").toUInt();
                    QString sPid = QString("%1").arg(pid, 4, 16, QChar('0'));
                    m_fixtureInfo += QString("<TR><TD CLASS='emphasis' COLSPAN='4'>PID: 0x%1 (%2)</TD></TR>")
                            .arg(sPid.toUpper()).arg(it.value().toString());
                }
                else if (m_requestState == StateReadSinglePid)
                {
                    // TODO
                }
            }
            else
            {
                info.params.insert(it.key(), it.value());
            }

            //qDebug() << it.key() << it.value();

            m_uidMap[UID] = info;
        }

        if (m_requestState == StatePersonalityInfo)
        {
            if (m_requestList.isEmpty() == false)
            {
                quint16 idx = m_requestList.takeFirst();
                m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                         QVariantList() << UID << PID_DMX_PERSONALITY_DESCRIPTION << 1 << idx);
            }
            else
            {
                m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                       QVariantList() << UID << PID_SLOT_INFO);
            }
        }
        else if (m_requestState == StateSlots)
        {
            if (m_requestList.isEmpty() == false)
            {
                quint16 slotId = m_requestList.takeFirst();
                m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                         QVariantList() << UID << PID_SLOT_DESCRIPTION << 2 << slotId);
            }
            else
            {
                m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                         QVariantList() << UID << PID_SUPPORTED_PARAMETERS);
            }
        }
        else if (m_requestState == StateSupportedPids)
        {
            if (m_requestList.isEmpty() == false)
            {
                quint16 pid = m_requestList.takeFirst();

                if (pid >= 0x8000)
                    m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                             QVariantList() << UID << PID_PARAMETER_DESCRIPTION << 2 << pid);
                else
                    m_plugin->sendRDMCommand(m_universe, m_line, GET_COMMAND,
                                             QVariantList() << UID << pid);
            }
            else
            {
                emit fixtureInfoReady(m_fixtureInfo);
                m_requestState = StateNone;
            }
        }
    }
}
