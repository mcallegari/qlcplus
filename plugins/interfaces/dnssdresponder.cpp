/*
  Q Light Controller Plus
  dnssdresponder.cpp

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

#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QUdpSocket>
#include <QHostInfo>
#include <QVariant>
#include <QDebug>

#include "dnssdresponder.h"

#define MDNS_PORT           5353
#define MDNS_GROUP          "224.0.0.251"

#define TYPE_A              1
#define TYPE_PTR            12
#define TYPE_TXT            16
#define TYPE_SRV            33
#define TYPE_ANY            255

#define CLASS_IN            0x0001
#define CLASS_FLUSH         0x8000 // cache-flush bit (answers)
#define CLASS_UNICAST       0x8000 // unicast-response bit (questions)

#define FLAGS_RESPONSE      0x8000
#define FLAGS_AUTHORITATIVE 0x0400

#define TTL_HOST            120
#define TTL_OTHER           4500
#define TTL_LEGACY          10

#define PROBE_COUNT         3
#define PROBE_INTERVAL      250
#define ANNOUNCE_COUNT      2
#define ANNOUNCE_INTERVAL   1000

/*****************************************************************************
 * Helpers
 *****************************************************************************/

static void appendU16(QByteArray &ba, quint16 value)
{
    ba.append(char(value >> 8));
    ba.append(char(value & 0xFF));
}

static void appendU32(QByteArray &ba, quint32 value)
{
    appendU16(ba, quint16(value >> 16));
    appendU16(ba, quint16(value & 0xFFFF));
}

static quint16 readU16(const QByteArray &ba, int pos)
{
    return quint16((uchar(ba.at(pos)) << 8) | uchar(ba.at(pos + 1)));
}

/** Append an uncompressed DNS name */
static void appendName(QByteArray &ba, const QStringList &labels)
{
    for (const QString &label : labels)
    {
        QByteArray l = label.toUtf8().left(63);
        ba.append(char(l.length()));
        ba.append(l);
    }
    ba.append(char(0));
}

/** Read a (possibly compressed) DNS name at $pos.
 *  On success, $pos is moved right after the name */
static bool readName(const QByteArray &ba, int &pos, QStringList &labels)
{
    int p = pos;
    int jumps = 0;
    bool jumped = false;

    labels.clear();

    while (p < ba.length())
    {
        uchar len = uchar(ba.at(p));

        if (len == 0)
        {
            if (jumped == false)
                pos = p + 1;
            return true;
        }

        if ((len & 0xC0) == 0xC0)
        {
            if (p + 1 >= ba.length() || ++jumps > 16)
                return false;
            if (jumped == false)
                pos = p + 2;
            jumped = true;
            p = ((len & 0x3F) << 8) | uchar(ba.at(p + 1));
            continue;
        }

        if (p + 1 + len > ba.length())
            return false;

        labels.append(QString::fromUtf8(ba.mid(p + 1, len)));
        p += len + 1;
    }

    return false;
}

static bool sameName(const QStringList &a, const QStringList &b)
{
    if (a.count() != b.count())
        return false;

    for (int i = 0; i < a.count(); i++)
        if (a.at(i).compare(b.at(i), Qt::CaseInsensitive) != 0)
            return false;

    return true;
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DNSSDResponder::DNSSDResponder(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_state(Idle)
    , m_step(0)
    , m_port(0)
    , m_conflicts(0)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

DNSSDResponder::~DNSSDResponder()
{
    unregisterService();
}

bool DNSSDResponder::registerService(const QString &name, const QString &type, quint16 port,
                                     const QMap<QString, QString> &txt)
{
    unregisterService();

    m_baseName = m_name = name;
    m_conflicts = 0;
    m_port = port;

    m_type = type.split('.');
    m_type.removeAll(QString());
    if (m_type.isEmpty() || m_type.last().compare("local", Qt::CaseInsensitive) != 0)
        m_type.append("local");

    // use a dedicated host name, to avoid interfering with the system mDNS responder
    QString host = QHostInfo::localHostName().section('.', 0, 0);
    m_host = QStringList() << QString("qlcplus-%1").arg(host) << "local";

    m_txt.clear();
    QMapIterator<QString, QString> it(txt);
    while (it.hasNext())
    {
        it.next();
        QByteArray entry = QString("%1=%2").arg(it.key(), it.value()).toUtf8().left(255);
        m_txt.append(char(entry.length()));
        m_txt.append(entry);
    }
    if (m_txt.isEmpty())
        m_txt.append(char(0));

    m_interfaces.clear();
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces())
    {
        QNetworkInterface::InterfaceFlags flags = iface.flags();
        if (!(flags & QNetworkInterface::IsUp) || !(flags & QNetworkInterface::IsRunning) ||
            !(flags & QNetworkInterface::CanMulticast) || (flags & QNetworkInterface::IsLoopBack))
            continue;

        for (const QNetworkAddressEntry &entry : iface.addressEntries())
        {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            Interface intf;
            intf.iface = iface;
            intf.entry = entry;
            m_interfaces.append(intf);
            break;
        }
    }

    if (m_interfaces.isEmpty())
    {
        qWarning() << "[DNS-SD] No suitable network interface found";
        return false;
    }

    m_socket = new QUdpSocket(this);
    if (m_socket->bind(QHostAddress::AnyIPv4, MDNS_PORT,
                       QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
    {
        qWarning() << "[DNS-SD] Cannot bind port" << MDNS_PORT << ":" << m_socket->errorString();
        delete m_socket;
        m_socket = nullptr;
        return false;
    }

    for (const Interface &intf : std::as_const(m_interfaces))
    {
        if (m_socket->joinMulticastGroup(QHostAddress(MDNS_GROUP), intf.iface) == false)
            qWarning() << "[DNS-SD] Cannot join multicast group on" << intf.iface.name();
    }

    m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 255);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

    startProbing();

    return true;
}

void DNSSDResponder::unregisterService()
{
    if (m_socket == nullptr)
        return;

    m_timer.stop();

    if (m_state == Announcing || m_state == Running)
        sendMulticast(Goodbye);

    m_socket->close();
    delete m_socket;
    m_socket = nullptr;
    m_state = Idle;
}

QString DNSSDResponder::serviceName() const
{
    return m_name;
}

/*****************************************************************************
 * Records
 *****************************************************************************/

QStringList DNSSDResponder::instanceName() const
{
    return QStringList(m_name) + m_type;
}

QList<DNSSDResponder::Record> DNSSDResponder::records(const QHostAddress &address) const
{
    QList<Record> list;
    Record rec;

    // ServicesRec: _services._dns-sd._udp.local PTR <type>
    rec.name = QStringList() << "_services" << "_dns-sd" << "_udp" << "local";
    rec.type = TYPE_PTR;
    rec.ttl = TTL_OTHER;
    rec.unique = false;
    rec.rdata.clear();
    appendName(rec.rdata, m_type);
    list.append(rec);

    // PtrRec: <type> PTR <instance>
    rec.name = m_type;
    rec.rdata.clear();
    appendName(rec.rdata, instanceName());
    list.append(rec);

    // SrvRec: <instance> SRV 0 0 <port> <host>
    rec.name = instanceName();
    rec.type = TYPE_SRV;
    rec.ttl = TTL_HOST;
    rec.unique = true;
    rec.rdata.clear();
    appendU16(rec.rdata, 0);
    appendU16(rec.rdata, 0);
    appendU16(rec.rdata, m_port);
    appendName(rec.rdata, m_host);
    list.append(rec);

    // TxtRec: <instance> TXT <key=value...>
    rec.type = TYPE_TXT;
    rec.ttl = TTL_OTHER;
    rec.rdata = m_txt;
    list.append(rec);

    // ARec: <host> A <address>
    rec.name = m_host;
    rec.type = TYPE_A;
    rec.ttl = TTL_HOST;
    rec.rdata.clear();
    appendU32(rec.rdata, address.toIPv4Address());
    list.append(rec);

    return list;
}

void DNSSDResponder::appendRecord(QByteArray &ba, const Record &rec, int ttl, bool legacy)
{
    quint32 recTTL = ttl < 0 ? rec.ttl : quint32(ttl);
    if (legacy)
        recTTL = qMin(recTTL, quint32(TTL_LEGACY));

    appendName(ba, rec.name);
    appendU16(ba, rec.type);
    appendU16(ba, CLASS_IN | ((rec.unique && !legacy) ? CLASS_FLUSH : 0));
    appendU32(ba, recTTL);
    appendU16(ba, quint16(rec.rdata.length()));
    ba.append(rec.rdata);
}

QByteArray DNSSDResponder::buildPacket(quint16 id, quint16 qdCount, const QByteArray &questions,
                                       const QList<Record> &answers, const QList<Record> &authority,
                                       const QList<Record> &additional, int ttl, bool legacy)
{
    QByteArray ba;

    // probes are queries, everything else is a response
    appendU16(ba, id);
    appendU16(ba, answers.isEmpty() ? 0 : (FLAGS_RESPONSE | FLAGS_AUTHORITATIVE));
    appendU16(ba, qdCount);
    appendU16(ba, quint16(answers.count()));
    appendU16(ba, quint16(authority.count()));
    appendU16(ba, quint16(additional.count()));
    ba.append(questions);

    for (const Record &rec : answers)
        appendRecord(ba, rec, ttl, legacy);
    for (const Record &rec : authority)
        appendRecord(ba, rec, ttl, legacy);
    for (const Record &rec : additional)
        appendRecord(ba, rec, ttl, legacy);

    return ba;
}

/*****************************************************************************
 * Probing / announcing
 *****************************************************************************/

void DNSSDResponder::startProbing()
{
    m_state = Probing;
    m_step = 0;
    // random initial delay to avoid collisions with other probing hosts
    m_timer.start(int(QRandomGenerator::global()->bounded(PROBE_INTERVAL)));
}

void DNSSDResponder::slotTimeout()
{
    if (m_state == Probing)
    {
        if (m_step < PROBE_COUNT)
        {
            sendMulticast(Probe);
            m_step++;
            m_timer.start(PROBE_INTERVAL);
            return;
        }
        m_state = Announcing;
        m_step = 0;
    }

    if (m_state == Announcing)
    {
        sendMulticast(Announce);
        if (++m_step < ANNOUNCE_COUNT)
        {
            m_timer.start(ANNOUNCE_INTERVAL);
        }
        else
        {
            m_state = Running;
            qDebug() << "[DNS-SD] Announced" << m_name << "on port" << m_port;
        }
    }
}

void DNSSDResponder::sendMulticast(PacketType type)
{
    for (const Interface &intf : std::as_const(m_interfaces))
    {
        QList<Record> recs = records(intf.entry.ip());
        QByteArray packet;

        if (type == Probe)
        {
            QByteArray question;
            appendName(question, instanceName());
            appendU16(question, TYPE_ANY);
            appendU16(question, CLASS_IN | CLASS_UNICAST);
            packet = buildPacket(0, 1, question, QList<Record>(),
                                 QList<Record>() << recs.at(SrvRec) << recs.at(TxtRec), QList<Record>());
        }
        else if (type == Announce)
        {
            packet = buildPacket(0, 0, QByteArray(), recs, QList<Record>(), QList<Record>());
        }
        else
        {
            // the host record is left out, as it might be in use by other services
            recs.removeAt(ARec);
            packet = buildPacket(0, 0, QByteArray(), recs, QList<Record>(), QList<Record>(), 0);
        }

        m_socket->setMulticastInterface(intf.iface);
        m_socket->writeDatagram(packet, QHostAddress(MDNS_GROUP), MDNS_PORT);
    }
}

/*****************************************************************************
 * Incoming packets
 *****************************************************************************/

int DNSSDResponder::findInterface(const QNetworkDatagram &dg) const
{
    for (int i = 0; i < m_interfaces.count(); i++)
        if (int(dg.interfaceIndex()) == m_interfaces.at(i).iface.index())
            return i;

    // fallback when the interface index is not available
    for (int i = 0; i < m_interfaces.count(); i++)
    {
        const QNetworkAddressEntry &entry = m_interfaces.at(i).entry;
        if (dg.senderAddress().isInSubnet(entry.ip(), entry.prefixLength()))
            return i;
    }

    return -1;
}

void DNSSDResponder::slotReadyRead()
{
    while (m_socket != nullptr && m_socket->hasPendingDatagrams())
    {
        QNetworkDatagram dg = m_socket->receiveDatagram();
        QByteArray data = dg.data();

        if (data.length() < 12)
            continue;

        if (readU16(data, 2) & FLAGS_RESPONSE)
        {
            processResponse(data);
        }
        else
        {
            int ifIndex = findInterface(dg);
            if (ifIndex >= 0)
                processQuery(dg, ifIndex);
        }
    }
}

void DNSSDResponder::processQuery(const QNetworkDatagram &dg, int ifIndex)
{
    // do not answer until the service name is confirmed unique
    if (m_state != Announcing && m_state != Running)
        return;

    const QByteArray data = dg.data();
    quint16 qdCount = readU16(data, 4);
    bool legacy = dg.senderPort() != MDNS_PORT;
    bool unicast = legacy;
    int pos = 12;

    QList<Record> recs = records(m_interfaces.at(ifIndex).entry.ip());
    QList<int> answers, additional;
    QByteArray questions;

    for (int q = 0; q < qdCount; q++)
    {
        QStringList name;
        if (readName(data, pos, name) == false || pos + 4 > data.length())
            return;

        quint16 type = readU16(data, pos);
        quint16 cls = readU16(data, pos + 2);
        pos += 4;

        if (cls & CLASS_UNICAST)
            unicast = true;

        // legacy unicast replies must repeat the questions
        appendName(questions, name);
        appendU16(questions, type);
        appendU16(questions, cls & ~CLASS_UNICAST);

        bool any = (type == TYPE_ANY);

        if (sameName(name, recs.at(ServicesRec).name) && (any || type == TYPE_PTR))
        {
            answers << ServicesRec;
        }
        else if (sameName(name, m_type) && (any || type == TYPE_PTR))
        {
            answers << PtrRec;
            additional << SrvRec << TxtRec << ARec;
        }
        else if (sameName(name, instanceName()))
        {
            if (any || type == TYPE_SRV)
                answers << SrvRec;
            if (any || type == TYPE_TXT)
                answers << TxtRec;
            additional << ARec;
        }
        else if (sameName(name, m_host) && (any || type == TYPE_A))
        {
            answers << ARec;
        }
    }

    if (answers.isEmpty())
        return;

    QList<Record> anList, arList;
    for (int i = ServicesRec; i <= ARec; i++)
    {
        if (answers.contains(i))
            anList << recs.at(i);
        else if (additional.contains(i))
            arList << recs.at(i);
    }

    if (legacy)
    {
        QByteArray packet = buildPacket(readU16(data, 0), qdCount, questions,
                                        anList, QList<Record>(), arList, -1, true);
        m_socket->writeDatagram(packet, dg.senderAddress(), quint16(dg.senderPort()));
    }
    else if (unicast)
    {
        QByteArray packet = buildPacket(0, 0, QByteArray(), anList, QList<Record>(), arList);
        m_socket->writeDatagram(packet, dg.senderAddress(), MDNS_PORT);
    }
    else
    {
        QByteArray packet = buildPacket(0, 0, QByteArray(), anList, QList<Record>(), arList);
        m_socket->setMulticastInterface(m_interfaces.at(ifIndex).iface);
        m_socket->writeDatagram(packet, QHostAddress(MDNS_GROUP), MDNS_PORT);
    }
}

void DNSSDResponder::processResponse(const QByteArray &data)
{
    if (m_state == Idle)
        return;

    int pos = 12;
    int qdCount = readU16(data, 4);
    int rrCount = readU16(data, 6) + readU16(data, 8) + readU16(data, 10);
    QStringList name;

    for (int q = 0; q < qdCount; q++)
    {
        if (readName(data, pos, name) == false)
            return;
        pos += 4;
    }

    for (int r = 0; r < rrCount; r++)
    {
        if (readName(data, pos, name) == false || pos + 10 > data.length())
            return;

        quint16 type = readU16(data, pos);
        quint32 ttl = (quint32(readU16(data, pos + 4)) << 16) | readU16(data, pos + 6);
        int rdLength = readU16(data, pos + 8);
        int rdPos = pos + 10;
        pos = rdPos + rdLength;

        if (pos > data.length())
            return;

        if (type != TYPE_SRV || ttl == 0 || rdLength < 7 || sameName(name, instanceName()) == false)
            continue;

        // someone else is claiming our service name: compare port and target host
        QStringList target;
        int targetPos = rdPos + 6;
        if (readName(data, targetPos, target) == false)
            return;

        if (readU16(data, rdPos + 4) == m_port && sameName(target, m_host))
            continue;

        m_conflicts++;
        m_name = QString("%1 (%2)").arg(m_baseName).arg(m_conflicts + 1);
        qDebug() << "[DNS-SD] Name conflict detected. Renaming service to" << m_name;
        emit serviceNameChanged(m_name);

        startProbing();
        return;
    }
}
