/*
 * Copyright (c) 2016 Alex Spataru <alex_spataru@outlook.com>
 *
 * This file is part of qMDNS, which is released under the MIT license.
 * For more information, please read the LICENSE file in the root directory
 * of this project.
 */

#include "qMDNS.h"

#include <QHostInfo>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QNetworkInterface>

#ifdef Q_OS_LINUX
    #include <cstring>
    #include <asm/types.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <linux/netlink.h>
    #include <linux/rtnetlink.h>
    #include <unistd.h>
#endif

/*
 * DNS port and mutlicast addresses
 */
const quint16 MDNS_PORT = 5353;
const QHostAddress IPV6_ADDRESS = QHostAddress ("FF02::FB");
const QHostAddress IPV4_ADDRESS = QHostAddress ("224.0.0.251");

/*
 * mDNS/DNS operation flags
 */
const quint16 kQR_Query       = 0x0000;
const quint16 kQR_Response    = 0x8000;
const quint16 kRecordA        = 0x0001;
const quint16 kRecordAAAA     = 0x001C;
const quint16 kNsecType       = 0x002F;
const quint16 kFQDN_Enquiry   = 0x0005;
const quint16 kFQDN_Separator = 0x0000;
const quint16 kFQDN_Length    = 0xC00C;
const quint16 kIN_BitFlush    = 0x8001;
const quint16 kIN_Normal      = 0x0001;

/*
 * DNS query properties
 */
const quint16 kQuery_QDCOUNT = 0x02;
const quint16 kQuery_ANCOUNT = 0x00;
const quint16 kQuery_NSCOUNT = 0x00;
const quint16 kQuery_ARCOUNT = 0x00;

/*
 * DNS response properties
 */
const quint16 kResponse_QDCOUNT = 0x00;
const quint16 kResponse_ANCOUNT = 0x01;
const quint16 kResponse_NSCOUNT = 0x00;
const quint16 kResponse_ARCOUNT = 0x02;

/* Packet constants */
const int MIN_LENGTH = 13;
const int IPI_LENGTH = 10;
const int IP4_LENGTH = IPI_LENGTH + 4;
const int IP6_LENGTH = IPI_LENGTH + 16;

/**
 * Encondes the 16-bit \a number as two 8-bit numbers in a byte array
 */
QByteArray ENCODE_16_BIT (quint16 number) {
    QByteArray data;
    data.append ((number & 0xff00) >> 8);
    data.append ((number & 0xff));
    return data;
}

/**
 * Encodes the 32-bit \a number as four 8-bit numbers
 */
QByteArray ENCODE_32_BIT (quint32 number) {
    QByteArray data;
    data.append ((number & 0xff000000UL) >> 24);
    data.append ((number & 0x00ff0000UL) >> 16);
    data.append ((number & 0x0000ff00UL) >>  8);
    data.append ((number & 0x000000ffUL));
    return data;
}

/**
 * Obtains the 16-bit number stored in the \a upper and \a lower 8-bit numbers
 */
quint16 DECODE_16_BIT (quint8 upper, quint8 lower) {
    return (quint16) ((upper << 8) | lower);
}

/**
 * Binds the given \a socket to the given \a address and \a port.
 * Under GNU/Linux, this function implements a workaround of QTBUG-33419.
 */
bool BIND (QUdpSocket* socket, const QHostAddress& address, const int port) {
    if (!socket)
        return false;

#ifdef Q_OS_LINUX
    int reuse = 1;
    int domain = PF_UNSPEC;

    if (address.protocol() == QAbstractSocket::IPv4Protocol)
        domain = PF_INET;
    else if (address.protocol() == QAbstractSocket::IPv6Protocol)
        domain = PF_INET6;

    socket->setSocketDescriptor (::socket (domain, SOCK_DGRAM, 0),
                                 QUdpSocket::UnconnectedState);

    setsockopt (socket->socketDescriptor(), SOL_SOCKET, SO_REUSEADDR,
                &reuse, sizeof (reuse));
#endif

    return socket->bind (address, port,
                         QUdpSocket::ShareAddress |
                         QUdpSocket::ReuseAddressHint);
}

qMDNS::qMDNS() {
    /* Set default TTL to 4500 seconds */
    m_ttl = 4500;

    /* Initialize sockets */
    m_IPv4Socket = nullptr;
    m_IPv6Socket = nullptr;
    initSockets();

#ifdef Q_OS_LINUX
    m_NetlinkSocket = openNetlinkSocket(this);
    if (!m_NetlinkSocket)
        qWarning() << "Could not open RTNETLINK socket";
    /* bruh */
    connect (m_NetlinkSocket, &QUdpSocket::readyRead, this, &qMDNS::onRouteEvent);
#endif
}

qMDNS::~qMDNS() {
    delete m_IPv4Socket;
    delete m_IPv6Socket;
#ifdef Q_OS_LINUX
    delete m_NetlinkSocket;
#endif
}

/**
 * Returns the only running instance of this class
 */
qMDNS* qMDNS::getInstance() {
    static qMDNS instance;
    return &instance;
}

/**
 * Returns the mDNS name assigned to the client computer
 */
QString qMDNS::hostName() const {
    return m_hostName;
}

/**
 * Ensures that the given \a string is a valid mDNS/DNS address.
 */
QString qMDNS::getAddress (const QString& string) {
    QString address = string;

    if (!string.endsWith (".local") && !string.contains ("."))
        address = string + ".local";

    if (string.endsWith ("."))
        return "";

    return address;
}

/**
 * Changes the TTL send to other computers in the mDNS network
 */
void qMDNS::setTTL (const quint32 ttl) {
    m_ttl = ttl;
}

/**
 * Performs a mDNS lookup to find the given host \a name.
 * If \a preferIPv6 is set to \c true, then this function will generate a
 * packet that requests an AAAA-type Resource Record instead of an A-type
 * Resource Record.
 */
void qMDNS::lookup (const QString& name) {
    /* The host name is empty, abort lookup */
    if (name.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Empty host name specified";
        return;
    }

    /* Ensure that we host name is a valid DNS address */
    QString address = getAddress (name);
    if (address.isEmpty())
        return;

    /* Check if we are dealing with a normal DNS address */
    if (!address.endsWith (".local", Qt::CaseInsensitive)) {
        QHostInfo::lookupHost (address, this, SIGNAL (hostFound (QHostInfo)));
        return;
    }

    /* Perform a mDNS lookup */
    else {
        QByteArray data;

        /* Get the host name and domain */
        QString host = address.split (".").first();
        QString domain = address.split (".").last();

        /* Check that domain length is valid */
        if (host.length() > 255) {
            qWarning() << Q_FUNC_INFO << host << "is too long!";
            return;
        }

        /* Create header & flags */
        data.append (ENCODE_16_BIT (0));
        data.append (ENCODE_16_BIT (kQR_Query));
        data.append (ENCODE_16_BIT (kQuery_QDCOUNT));
        data.append (ENCODE_16_BIT (kQuery_ANCOUNT));
        data.append (ENCODE_16_BIT (kQuery_NSCOUNT));
        data.append (ENCODE_16_BIT (kQuery_ARCOUNT));

        /* Add name data */
        data.append (host.length());
        data.append (host.toUtf8());

        /* Add domain data */
        data.append (domain.length());
        data.append (domain.toUtf8());

        /* Add FQDN/TLD separator */
        data.append ((char) kFQDN_Separator);

        /* Add IPv4 record type */
        data.append (ENCODE_16_BIT (kRecordA));
        data.append (ENCODE_16_BIT (kIN_Normal));

        /* Add FQDN length */
        data.append (ENCODE_16_BIT (kFQDN_Length));

        /* Add IPv6 record type */
        data.append (ENCODE_16_BIT (kRecordAAAA));
        data.append (ENCODE_16_BIT (kIN_Normal));

        /* Send the datagram */
        sendPacket (data);
    }
}

/**
 * Changes the host name of the client computer
 */
void qMDNS::setHostName (const QString& name) {
    if (name.contains (".") && !name.endsWith (".local")) {
        qWarning() << "Invalid domain name";
        return;
    }

    m_hostName = getAddress (name);
}

/**
 * Called when we receive data from a mDNS client on the network.
 */
void qMDNS::onReadyRead() {
    QByteArray data;
    QUdpSocket* socket = qobject_cast<QUdpSocket*> (sender());
    QHostAddress senderAddress;

    /* Read data from the socket */
    if (socket) {
        while (socket->hasPendingDatagrams()) {
            data.resize (socket->pendingDatagramSize());
            socket->readDatagram (data.data(), data.size(), &senderAddress);
        }
    }

    /* Packet is a valid mDNS datagram */
    if (data.length() > MIN_LENGTH) {
        quint16 flag = DECODE_16_BIT (data.at (2), data.at (3));

        if (flag == kQR_Query)
        {
            if (readQuery (data) == 0)
            {
                QList<QHostAddress> addrList = { senderAddress };
                QHostInfo info;
                info.setAddresses(addrList);
                emit hostFound(info);
            }
        }

        else if (flag >= kQR_Response)
            readResponse (data);
    }
}

#ifdef Q_OS_LINUX
/**
 * Called when we receive an event on our RTNETLINK socket.
 */
void qMDNS::onRouteEvent() {
    bool reinitializeSockets = false;
    QByteArray data;

    while (m_NetlinkSocket->hasPendingDatagrams()) {
        data.resize(m_NetlinkSocket->pendingDatagramSize());
        m_NetlinkSocket->readDatagram(data.data(), data.size());

        unsigned int toRead = data.size();
        for (struct nlmsghdr *header = (struct nlmsghdr*)data.data();
            NLMSG_OK (header, toRead); header = NLMSG_NEXT (header, toRead)) {
            if (header->nlmsg_type == NLMSG_ERROR ||
                header->nlmsg_type == NLMSG_DONE) {
                break;
            }

            if (header->nlmsg_type == RTM_NEWLINK) {
                reinitializeSockets = true;
            }
        }
    }

    if (reinitializeSockets) {
        qInfo() << "reinitializing sockets";
        initSockets();
    }
}
#endif

/**
 * Reads the given query \a data and instructs the class to send a response
 * packet if the query is looking for the host name assigned to this computer.
 */
int qMDNS::readQuery(const QByteArray& data) {
    /* Query packet is invalid */
    if (data.length() < MIN_LENGTH)
        return -1;

    /* Get the lengths of the host name and domain */
    int n = 12;
    int hostLength = data.at (n);
    int domainLength = data.at (n + hostLength + 1);

    /* Read the host name until we stumble with the domain length character */
    QString name;
    int h = n + 1;
    while (data.at (h) != (char) domainLength) {
        name.append (data.at (h));
        ++h;
    }

    /* Read domain length until we stumble with the FQDN/TLD separator */
    QString domain;
    int d = n + hostLength + 2;
    while (data.at (d) != kFQDN_Separator) {
        if (data.at(d) == kFQDN_Enquiry)
            domain.append ('.');
        else
            domain.append (data.at (d));
        ++d;
    }

    /* Construct the full host name (name + domain) */
    QString host = getAddress (name + "." + domain);

    /* The query packet wants to know more about us */
    if (host.toLower() == hostName().toLower())
        sendResponse (DECODE_16_BIT (data.at (0), data.at (1)));

    return 0;
}

/**
 * Sends the given \a data to both the IPv4 and IPv6 mDNS multicast groups
 */
void qMDNS::sendPacket (const QByteArray& data) {
    if (!data.isEmpty()) {
        m_IPv4Socket->writeDatagram (data, IPV4_ADDRESS, MDNS_PORT);
        m_IPv6Socket->writeDatagram (data, IPV6_ADDRESS, MDNS_PORT);
    }
}

/**
 * Reads the given \a data of a response packet and obtains:
 * - The remote host name
 * - The remote IPv4
 * - The remote IPv6
 */
void qMDNS::readResponse (const QByteArray& data) {
    if (data.length() < MIN_LENGTH)
        return;

    QString host = getHostNameFromResponse (data);
    QList<QHostAddress> addresses = getAddressesFromResponse (data, host);

    if (!host.isEmpty() && !addresses.isEmpty()) {
        QHostInfo info;
        info.setHostName (host);
        info.setAddresses (addresses);
        info.setError (QHostInfo::NoError);

        emit hostFound (info);
    }
}

/**
 * Sends a response packet with:
 * - Our mDNS host name
 * - Our IPv4 address
 * - Our IPv6 address
 */
void qMDNS::sendResponse (const quint16 query_id) {
    if (!hostName().isEmpty() && hostName().endsWith (".local")) {
        QByteArray data;

        /* Get the host name and domain */
        QString host = hostName().split (".").first();
        QString domain = hostName().split (".").last();

        /* Get local IPs */
        quint32 ipv4 = 0;
        QList<QIPv6Address> ipv6;
        foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
            if (!address.isLoopback()) {
                if (address.protocol() == QAbstractSocket::IPv4Protocol)
                    ipv4 = (ipv4 == 0 ? address.toIPv4Address() : ipv4);

                if (address.protocol() == QAbstractSocket::IPv6Protocol)
                    ipv6.append (address.toIPv6Address());
            }
        }

        /* Check that domain length is valid */
        if (host.length() > 255) {
            qWarning() << Q_FUNC_INFO << host << "is too long!";
            return;
        }

        /* Create header and flags */
        data.append (ENCODE_16_BIT (query_id));
        data.append (ENCODE_16_BIT (kQR_Response));
        data.append (ENCODE_16_BIT (kResponse_QDCOUNT));
        data.append (ENCODE_16_BIT (kResponse_ANCOUNT));
        data.append (ENCODE_16_BIT (kResponse_NSCOUNT));
        data.append (ENCODE_16_BIT (kResponse_ARCOUNT));

        /* Add name data */
        data.append (host.length());
        data.append (host.toUtf8());

        /* Add domain data and FQDN/TLD separator */
        data.append (domain.length());
        data.append (domain.toUtf8());
        data.append ((char) kFQDN_Separator);

        /* Add IPv4 address header */
        data.append (ENCODE_16_BIT (kRecordA));
        data.append (ENCODE_16_BIT (kIN_BitFlush));
        data.append (ENCODE_32_BIT (m_ttl));
        data.append (ENCODE_16_BIT (sizeof (ipv4)));

        /* Add IPv4 bytes */
        data.append (ENCODE_32_BIT (ipv4));

        /* Add FQDN offset */
        data.append (ENCODE_16_BIT (kFQDN_Length));

        /* Add IPv6 addresses */
        foreach (QIPv6Address ip, ipv6) {
            data.append (ENCODE_16_BIT (kRecordAAAA));
            data.append (ENCODE_16_BIT (kIN_BitFlush));
            data.append (ENCODE_32_BIT (m_ttl));
            data.append (ENCODE_16_BIT (sizeof (ip.c)));

            /* Add IPv6 bytes */
            for (unsigned long i = 0; i < sizeof (ip.c); ++i)
                data.append (ip.c [i]);

            /* Add FQDN offset */
            data.append (ENCODE_16_BIT (kFQDN_Length));
        }

        /* TODO: Generate NSEC code block */
        int nsec_length = 0;

        /* Add NSEC data */
        data.append (ENCODE_16_BIT (kNsecType));
        data.append (ENCODE_16_BIT (kIN_BitFlush));
        data.append (ENCODE_32_BIT (m_ttl));
        data.append (ENCODE_16_BIT (nsec_length));

        /* Send the response */
        sendPacket (data);
    }
}

/*
 * Initializes the IPv4 and IPv6 mDNS UDP sockets.
 */
void qMDNS::initSockets() {
    if (m_IPv4Socket)
        delete m_IPv4Socket;
    if (m_IPv6Socket)
        delete m_IPv6Socket;

    m_IPv4Socket = new QUdpSocket (this);
    m_IPv6Socket = new QUdpSocket (this);

    /* Read and interpret data received from mDNS group */
    connect (m_IPv4Socket, &QUdpSocket::readyRead, this, &qMDNS::onReadyRead);
    connect (m_IPv6Socket, &QUdpSocket::readyRead, this, &qMDNS::onReadyRead);

    /* Bind the sockets to the mDNS multicast group */
    if (BIND (m_IPv4Socket, QHostAddress::AnyIPv4, MDNS_PORT))
        m_IPv4Socket->joinMulticastGroup (IPV4_ADDRESS);
    if (BIND (m_IPv6Socket, QHostAddress::AnyIPv6, MDNS_PORT))
        m_IPv6Socket->joinMulticastGroup (IPV6_ADDRESS);
}

#ifdef Q_OS_LINUX
/*
 * Opens an RTNETLINK socket for listening to interface events.
 */
QUdpSocket* qMDNS::openNetlinkSocket (QObject*) {
    int sock = socket (AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);

    if (sock == -1) {
        return nullptr;
    }

    struct sockaddr_nl sa;
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK;
    sa.nl_pid = getpid();

    if (bind (sock, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        close (sock);
        return nullptr;
    }

    QUdpSocket* qsock = new QUdpSocket (this);

    if (!qsock->setSocketDescriptor(sock)) {
        close (sock);
        return nullptr;
    }

    return qsock;
}
#endif

/**
 * Extracts the host name from the \a data received from the mDNS network.
 * The host name begins at byte #12 (when the header and flags end) and ends
 * with a mandatory NUL character after the domain.
 *
 * The host name is constructed in the following way (without spaces):
 * \c NAME_LENGTH + \c NAME + \c DOMAIN_LENGTH + \c DOMAIN + \c NUL
 *
 * For example, appletv.local would be formatted as:
 * \c 0x07 + \c appletv + \c 0x05 + \c local + \c 0x00
 *
 * Or, if you prefer hex data:
 * \c { 07 61 70 70 6c 65 74 76 05 6c 6f 63 61 6c 00 }
 * \c {  7 a  p  p  l  e  t  v   5 l  o  c  a  l   0 }
 *
 * In order to obtain the full host name (and its mDNS domain), we construct
 * the string backwards. When the code notices that the current character is
 * the same as the domain length, we know that the domain name has been
 * extracted, and thus we can replace the domain length with a dot (.) and
 * begin extracting the host name.
 */
QString qMDNS::getHostNameFromResponse (const QByteArray& data) {
    QList<char> list;
    QString address = "";

    /* Begin reading host name at byte 13 (byte 12 is the host name length) */
    int n = 13;

    /* Read the host name until we stumble with the FQDN/TLD separator */
    while (data.at (n) != kFQDN_Separator) {
        list.append (data.at (n));
        ++n;
    }

    /* Construct the string backwards (to replace domain length with a dot) */
    for (int i = 0; i < list.count(); ++i) {
        char character = list.at (list.count() - i - 1);

        if (character == (char) address.length())
            address.prepend (".");
        else
            address.prepend (character);
    }

    return address;
}

/**
 * Extracts the IPv4 from the \a data received from the mDNS network.
 * The IPv4 data begins when the host name data ends.
 *
 * For the packet to contain IPv4 information, the DNS Record Type code must
 * be "A" (IPv4) and the DNS Class code should correspond to "IN" (Internet).
 *
 * Here is the layout of the IPv4 section of the packet:
 *
 * - DNS Record Type
 * - DNS Class Code
 * - TTL
 * - IP length
 * - IP address bytes
 *
 * This is an example IPv4 section:
 * \c {00 01 80 01 00 00 78 00 00 04 99 6d 07 5a}
 *
 * Data in example section:
 * - \c {00 01} Type Codes
 * - \c {80 01} Class Codes
 * - \c {00 00 78 00} IP TTL
 * - \c {00 04} Number of address bytes (length in layman's terms)
 * - \c {99 6d 07 5a} IPv4 Address bytes (153, 109, 7, 90)
 */
QString qMDNS::getIPv4FromResponse (const QByteArray& data,
                                    const QString& host) {
    QString ip = "";

    /* n stands for the byte index in which the host name data ends */
    int n = MIN_LENGTH + host.length();

    /* Packet is too small */
    if (data.length() < n + IP4_LENGTH)
        return ip;

    /* Get the IP type and class codes */
    quint16 typeCode  = DECODE_16_BIT (data.at (n + 1), data.at (n + 2));
    quint16 classCode = DECODE_16_BIT (data.at (n + 3), data.at (n + 4));

    /* Check if type and class codes are good */
    if (typeCode != kRecordA || classCode != kIN_BitFlush)
        return ip;

    /* Skip TTL indicator and obtain the number of address bytes */
    quint8 length = data.at (n + IPI_LENGTH);

    /* Append each IPv4 address byte (and decimal dots) to the IP string */
    for (int i = 1; i < length + 1; ++i) {
        ip += QString::number ((quint8) data.at (n + IPI_LENGTH + i));
        ip += (i < length) ? "." : "";
    }

    return ip;
}

/**
 * Extracts the IPv6 from the \a data received from the mDNS network.
 * The IPv6 data begins when the host name data ends.
 *
 * For the packet to contain IPv6 information, the DNS Record Type code must
 * be "AAAA" (IPv6) and the DNS Class code should correspond to "IN" (Internet).
 *
 * Here is the layout of the IPv4 section of the packet:
 *
 * - DNS Record Type
 * - DNS Class Code
 * - TTL
 * - IP length
 * - IP address bytes
 *
 * This is an example IPv6 section:
 * \c { 00 1c 80 01 00 00 78 00 00 10 fe 80 00 00 00 00 00 00 02 23 32 ff fe b1 21 52 }
 *
 * Data in example section:
 * - \c {00 1c} Type Codes
 * - \c {80 01} Class Codes
 * - \c {00 00 78 00} IP TTL
 * - \c {00 10} Number of address bytes (length in layman's terms)
 * - \c {fe 80 00 00 ... 52} IPv6 Address bytes (there are 16 of them)
 */
QStringList qMDNS::getIPv6FromResponse (const QByteArray& data,
                                        const QString& host) {
    QStringList list;

    /* Skip the FQDN and IPv4 section */
    int n = MIN_LENGTH + IP4_LENGTH + host.length();

    /* Get the IPv6 list */
    bool isIPv6 = true;
    while (isIPv6) {
        /* Skip FQDN bytes */
        n += 2;

        /* Packet is invalid */
        if (data.length() < n + IP6_LENGTH)
            break;

        /* Get the IP type and class codes */
        quint16 typeCode  = DECODE_16_BIT (data.at (n + 1), data.at (n + 2));
        quint16 classCode = DECODE_16_BIT (data.at (n + 3), data.at (n + 4));
        isIPv6 = (typeCode == kRecordAAAA && classCode == kIN_BitFlush);

        /* IP type and class codes are OK, extract IP */
        if (isIPv6) {
            /* Skip TTL indicator and obtain the number of address bytes */
            quint8 length = data.at (n + IPI_LENGTH);

            /* Append each IPv6 address byte (encoded as hex) to the IP string */
            QString ip = "";
            for (int i = 1; i < length + 1; ++i) {
                /* Get the hexadecimal representation of the byte */
                QString byte;
                byte.setNum ((quint8) data.at (n + i + IPI_LENGTH), 16);

                /* Add the obtained string */
                ip += byte;

                /* Append colons after even indexes (except in the last byte) */
                if ((i & 1) == 0 && (i < length))
                    ip += ":";
            }

            /* Increase the counter to 'jump' to the next section */
            n += 26;

            /* Append the obtained IP to the list */
            if (!list.contains (ip))
                list.append (ip);
        }
    }

    return list;
}

/**
 * Obtains the IPv4 and IPv6 addresses from the received data.
 * \note This function will only generate a list with the valid IP addresses.
 */
QList<QHostAddress> qMDNS::getAddressesFromResponse (const QByteArray& data,
        const QString& host) {
    QList<QHostAddress> list;

    /* Add IPv4 address */
    QHostAddress IPv4Address = QHostAddress (getIPv4FromResponse (data, host));
    if (!IPv4Address.isNull())
        list.append (IPv4Address);

    /* Add IPv6 addresses */
    foreach (QString ip, getIPv6FromResponse (data, host)) {
        QHostAddress address = QHostAddress (ip);
        if (!address.isNull())
            list.append (address);
    }

    return list;
}
