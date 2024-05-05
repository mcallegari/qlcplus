/*
  Q Light Controller Plus
  rdmmanager.h

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

#ifndef RDMMANAGER_H
#define RDMMANAGER_H

#include <QThread>

#include "ui_rdmmanager.h"

class QLCIOPlugin;
class Doc;

typedef struct
{
    QString manufacturer;
    QString name;
    quint32 universe;
    quint32 pluginLine;
    quint16 dmxAddress;
    quint16 channels;
    QVariantMap params;
} UIDInfo;

typedef struct
{
    qulonglong startUID;
    qulonglong endUID;
} DiscoveryInfo;

class RDMWorker : public QThread
{
    Q_OBJECT

public:
    RDMWorker(Doc *doc);
    ~RDMWorker();

    enum RequestState
    {
        StateNone,
        StateWait,
        StateDiscoveryStart,
        StateDiscoveryContinue,
        StateDiscoveryEnd,
        StatePersonalities,
        StatePersonalityInfo,
        StateSlots,
        StateSupportedPids,
        StateReadSinglePid,
        StateWriteSinglePid,
        StateWaitPidInfo
    };

    /** Start the discovery process and handle it vai state machine */
    void runDiscovery(quint32 uni, quint32 line);

    /** Request the given UID information and handle results via state machine */
    void getUidInfo(quint32 uni, quint32 line, QString UID, UIDInfo &info);

    void handlePID(quint32 uni, quint32 line, QString UID, QString pid, QVariantList args, bool write);

private:
    void run();

    /** Stop this thread */
    void stop();

private slots:

    /** Slot called whenever a plugin has collected new RDM data */
    void slotRDMDataReady(quint32 universe, quint32 line, QVariantMap data);

signals:
    /** Inform the listeners that a new UID has been discovered */
    void uidFound(QString UID, UIDInfo info);

    void fixtureInfoReady(QString &info);

    void pidInfoReady(QString info);

    void requestPopup(QString title, QString message);

private:
    Doc *m_doc;
    bool m_running;

    QLCIOPlugin *m_plugin;
    quint32 m_universe;
    quint32 m_line;

    /** Map of all the discovered fixtures by UID */
    QMap <QString, UIDInfo> m_uidMap;

    /** FIFO for discovery binary search */
    QList<DiscoveryInfo> m_discoveryList;

    /** State variable to sync between a request and its answer */
    RequestState m_requestState;

    QVector<quint16> m_requestList;

    /** String containing HTML formatted
     *  information of a fixture */
    QString m_fixtureInfo;
};

class RDMManager : public QWidget, public Ui_RDMManager
{
    Q_OBJECT

public:
    explicit RDMManager(QWidget *parent, Doc* doc);
    ~RDMManager();

    enum PidDataType
    {
        ByteArg = 0,
        ShortArg,
        LongArg,
        ArrayArg
    };

private:
    bool getPluginInfo(quint32 universe, quint32 line, quint32 &universeID, quint32 &outputLine);

private slots:
    /** Triggers the RDM discovery on all universes */
    void slotRefresh();

    /** Triggers the download of all the supported
     *  PID and information from the first selected fixture */
    void slotGetInfo();

    /** Read a single PID and return the result */
    void slotReadPID();

    /** Write a single PID and return the result */
    void slotWritePID();

    void updateRDMTreeItem(QString UID, UIDInfo info);

    void slotSelectionChanged();

    void slotUpdatePidInfo(QString info);

    void slotDisplayPopup(QString title, QString message);

    void slotTaskFinished();

signals:
    void fixtureInfoReady(QString &info);

private:
    /** Reference to the main Doc object */
    Doc *m_doc;

    /** Map of all the discovered fixtures by UID */
    QMap <QString, UIDInfo> m_uidMap;
};

#endif // RDMMANAGER_H
