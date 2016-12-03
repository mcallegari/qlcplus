/*
  Q Light Controller Plus
  osccontroller.h

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

#ifndef OSCCONTROLLER_H
#define OSCCONTROLLER_H

#if defined(ANDROID)
#include <QNetworkInterface>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QHostAddress>
#include <QUdpSocket>
#else
#include <QtNetwork>
#endif
#include <QMutex>
#include <QTimer>
#include <QHash>
#include <QMap>

#include "oscpacketizer.h"

typedef struct
{
    QSharedPointer<QUdpSocket> inputSocket;

    quint16 inputPort;

    QHostAddress feedbackAddress;
    quint16 feedbackPort;

    QHostAddress outputAddress;
    quint16 outputPort;

    // cache of the OSC paths with multiple values, used to correctly
    // handle the flow of input and feedback values
    QHash<QString, QByteArray> multipartCache;
    int type;
} UniverseInfo;

class OSCController : public QObject
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    enum Type { Unknown = 0x0, Input = 0x01, Output = 0x02 };

    OSCController(QString ipaddr,
                   Type type, quint32 line, QObject *parent = 0);

    ~OSCController();

    /** Return the controller IP address */
    QHostAddress getNetworkIP() const;

    /** Add a universe to the map of this controller */
    void addUniverse(quint32 universe, Type type);

    /** Remove a universe from the map of this controller */
    void removeUniverse(quint32 universe, Type type);

    /** Set a specific port to be bound to receive data from the
     *  given universe.
     *  Return true if this restores default input port */
    bool setInputPort(quint32 universe, quint16 port);

    /** Set a specific feedback IP address for the given QLC+ universe.
     *  Return true if this restores default feedback IP address */
    bool setFeedbackIPAddress(quint32 universe, QString address);

    /** Set a specific port where to send feedback packets
     *  for the given universe.
     *  Return true if this restores default feedback port */
    bool setFeedbackPort(quint32 universe, quint16 port);

    /** Set a specific output IP address for the given QLC+ universe.
     *  Return true if this restores default output IP address */
    bool setOutputIPAddress(quint32 universe, QString address);

    /** Set a specific port where to send outgoing packets
     *  for the given universe.
     *  Return true if this restores default output port */
    bool setOutputPort(quint32 universe, quint16 port);

    /** Return the list of the universes handled by
     *  this controller */
    QList<quint32> universesList() const;

    /** Return the specific information for the given universe */
    UniverseInfo* getUniverseInfo(quint32 universe);

    /** Return the global type of this controller */
    Type type() const;

    /** Return the plugin line associated to this controller */
    quint32 line() const;

    /** Get the number of packets sent by this controller */
    quint64 getPacketSentNumber() const;

    /** Get the number of packets received by this controller */
    quint64 getPacketReceivedNumber() const;

    /** Send DMX data to a specific universe */
    void sendDmx(const quint32 universe, const QByteArray& dmxData);

    /** Send a feedback using the specified path and value */
    void sendFeedback(const quint32 universe, quint32 channel, uchar value, const QString &key);

private:
    QSharedPointer<QUdpSocket> getInputSocket(quint16 port);

protected:
    /** Calculate a 16bit unsigned hash as a unique representation
     *  of a OSC path. If new, the hash is added to the hash map (m_hashMap) */
    quint16 getHash(QString path);

private:
    void handlePacket(QUdpSocket* socket, QByteArray const& datagram, QHostAddress const& senderAddress);

private slots:
    /** Async event raised when new packets have been received */
    void processPendingPackets();

signals:
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value, QString key);

private:
    /** The controller IP address as QHostAddress */
    QHostAddress m_ipAddr;

    quint64 m_packetSent;
    quint64 m_packetReceived;

    /** QLC+ line to be used when emitting a signal */
    quint32 m_line;

    /** The UDP socket used to output OSC packets */
    QSharedPointer<QUdpSocket> m_outputSocket;

    /** Helper class used to create or parse OSC packets */
    QScopedPointer<OSCPacketizer> m_packetizer;

    /** Keeps the current dmx values to send only the ones that changed */
    /** It holds values for all the handled universes */
    QMap<quint32, QByteArray *> m_dmxValuesMap;

    /** Map of the QLC+ universes transmitted/received by this
     *  controller, with the related, specific parameters */
    QMap<quint32, UniverseInfo> m_universeMap;

    /** Mutex to handle the change of output IP address or in general
     *  variables that could be used to transmit/receive data */
    QMutex m_dataMutex;

    /** This is fundamental for the OSC controller. Every time a OSC signal is received,
      * the controller will calculate a 16 bit checksum of the OSC path and add it to
      * this hash table if new, otherwise the controller will use the hash table
      * to quickly retrieve a unique channel number
      */
    QHash<QString, quint16> m_hashMap;
};

#endif
