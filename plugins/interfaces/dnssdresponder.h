/*
  Q Light Controller Plus
  dnssdresponder.h

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

#ifndef DNSSDRESPONDER_H
#define DNSSDRESPONDER_H

#include <QNetworkInterface>
#include <QStringList>
#include <QObject>
#include <QTimer>
#include <QMap>

class QUdpSocket;
class QNetworkDatagram;

/**
 * Minimal mDNS (RFC 6762) / DNS-SD (RFC 6763) responder, based on Qt Network only.
 * It advertises a single service (IPv4) on the local network, so that clients
 * can discover it via Bonjour/Avahi/Zeroconf.
 *
 * Usage:
 *   m_dnssd = new DNSSDResponder(this);
 *   m_dnssd->registerService("QLC+", "_os2l._tcp", 9996);
 *   ...
 *   m_dnssd->unregisterService();
 */
class DNSSDResponder : public QObject
{
    Q_OBJECT

public:
    DNSSDResponder(QObject *parent = nullptr);
    ~DNSSDResponder();

    /** Start advertising a service.
     *  @param name the service instance name, as displayed by clients
     *  @param type the service type, e.g. "_os2l._tcp"
     *  @param port the port where the service is listening
     *  @param txt optional key/value pairs for the TXT record */
    bool registerService(const QString &name, const QString &type, quint16 port,
                         const QMap<QString, QString> &txt = QMap<QString, QString>());

    /** Stop advertising the service, sending goodbye packets */
    void unregisterService();

    /** The service name in use. It might differ from the requested
     *  one if a name conflict was detected on the network */
    QString serviceName() const;

signals:
    /** Emitted when the service has been renamed after a conflict */
    void serviceNameChanged(const QString &name);

private slots:
    void slotReadyRead();
    void slotTimeout();

private:
    enum State { Idle, Probing, Announcing, Running };
    enum PacketType { Probe, Announce, Goodbye };
    enum RecordIndex { ServicesRec = 0, PtrRec, SrvRec, TxtRec, ARec };

    struct Record
    {
        QStringList name;
        quint16 type;
        quint32 ttl;
        bool unique;
        QByteArray rdata;
    };

    struct Interface
    {
        QNetworkInterface iface;
        QNetworkAddressEntry entry;
    };

    QStringList instanceName() const;
    QList<Record> records(const QHostAddress &address) const;
    int findInterface(const QNetworkDatagram &dg) const;

    void startProbing();
    void sendMulticast(PacketType type);
    void processQuery(const QNetworkDatagram &dg, int ifIndex);
    void processResponse(const QByteArray &data);

    static void appendRecord(QByteArray &ba, const Record &rec, int ttl, bool legacy);
    static QByteArray buildPacket(quint16 id, quint16 qdCount, const QByteArray &questions,
                                  const QList<Record> &answers, const QList<Record> &authority,
                                  const QList<Record> &additional, int ttl = -1, bool legacy = false);

private:
    QUdpSocket *m_socket;
    QTimer m_timer;
    State m_state;
    int m_step;

    QString m_baseName;
    QString m_name;
    QStringList m_type;
    QStringList m_host;
    quint16 m_port;
    QByteArray m_txt;
    int m_conflicts;

    QList<Interface> m_interfaces;
};

#endif
