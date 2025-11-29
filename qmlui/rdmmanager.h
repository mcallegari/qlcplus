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

#include <QQuickView>
#include <QVariant>
#include <QThread>

class QLCIOPlugin;
class ListModel;
class Doc;

typedef struct
{
    QString manufacturer;
    QString name;
    quint32 universe;
    quint32 pluginLine;
    quint16 dmxAddress;
    quint16 channels;
    quint8 personalityIndex;
    quint8 personalityCount;
    QString personality;
    QVariantMap params;
    int operation;
} UIDInfo;

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
        StateGetFixtureInfo
    };

    /** Start the discovery process and handle it via state machine */
    void runDiscovery(quint32 uni, quint32 line);

    /** Execute a list of operations (SET DMX address, toggle identify, etc.) */
    void runOperations(quint32 uni, quint32 line,
                       const QMap<QString, UIDInfo> &operations);

private:
    void run();

    /** Stop this thread */
    void stop();

    /** Add a RDM command to the queue (first element = command type) */
    void addToQueue(quint8 command, const QVariantList &params);

    /** Send the next queued RDM command, if any */
    void checkQueue();

signals:
    /** Inform the listeners that a new UID has been discovered */
    void uidFound(QString UID, UIDInfo info);

    /** Request the UI to display a popup with the given title/message */
    void requestPopup(QString title, QString message);

private slots:

    /** Slot called whenever a plugin has collected new RDM data */
    void slotRDMDataReady(quint32 universe, quint32 line, QVariantMap data);

private:
    Doc *m_doc;
    bool m_running;

    QLCIOPlugin *m_plugin;
    quint32 m_universe;
    quint32 m_line;

    QStringList m_temporaryUidList;
    QList<QVariantList> m_rdmCommandsQueue;

    /** Map of all the discovered fixtures by UID */
    QMap <QString, UIDInfo> m_uidMap;

    /** Last discovery range we sent on the wire */
    qulonglong m_currentStartUID;
    qulonglong m_currentEndUID;

    /** State variable to sync between a request and its answer */
    RequestState m_requestState;
};

class RDMManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant fixtureList READ fixtureList NOTIFY fixtureListChanged)
    Q_PROPERTY(DiscoveryStatus discoveryStatus READ discoveryStatus WRITE setDiscoveryStatus NOTIFY discoveryStatusChanged FINAL)
    Q_PROPERTY(int fixturesFound READ fixturesFound NOTIFY fixturesFoundChanged FINAL)

public:
    explicit RDMManager(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~RDMManager();

    enum DiscoveryStatus
    {
        Idle = 0,
        Running,
        Finished
    };
    Q_ENUM(DiscoveryStatus)

    enum Operations
    {
        SetDMXAddress  = 1 << 0,
        ToggleIdentify = 1 << 1
    };
    Q_ENUM(Operations)

    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void requestDMXAddress(QString UID, int address);
    Q_INVOKABLE void executeOperations();

    int fixturesFound();

    QVariant fixtureList();

    DiscoveryStatus discoveryStatus() const;
    void setDiscoveryStatus(DiscoveryStatus status);

protected slots:
    void slotUIDFound(QString UID, UIDInfo info);
    void slotTaskFinished();

private:
    void updateFixtureList();

signals:
    void fixtureListChanged();
    void discoveryStatusChanged();
    void fixturesFoundChanged();

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;

    /** Reference to a ListModel representing the RDM fixtures detected,
     *  organized as follows:
     *  universe | manufacturer | model | DMX address | personality | UID | isSelected
     */
    ListModel *m_fixtureList;

    DiscoveryStatus m_discoveryStatus;
    int m_fixturesFound;

    /** Map of all the discovered fixtures by UID */
    QMap <QString, UIDInfo> m_uidMap;
};
#endif
