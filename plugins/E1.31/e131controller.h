/*
  Q Light Controller Plus
  e131controller.h

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

#ifndef E131CONTROLLER_H
#define E131CONTROLLER_H

#if defined(ANDROID)
#include <QScopedPointer>
#include <QSharedPointer>
#endif
#include <QNetworkInterface>
#include <QHostAddress>
#include <QUdpSocket>
#include <QMutex>
#include <QTimer>

#include "e131packetizer.h"

#define E131_DEFAULT_PORT     5568

typedef struct _uinfo
{
    bool inputMulticast;
    QHostAddress inputMcastAddress;
    quint16 inputUcastPort;
    quint16 inputUniverse;
    QSharedPointer<QUdpSocket> inputSocket;

    bool outputMulticast;
    QHostAddress outputMcastAddress;
    QHostAddress outputUcastAddress;
    quint16 outputUcastPort;
    quint16 outputUniverse;
    int outputTransmissionMode;
    int outputPriority;

    int type;
} UniverseInfo;

class E131Controller : public QObject
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    enum Type { Unknown = 0x0, Input = 0x01, Output = 0x02 };

    enum TransmissionMode { Full, Partial };

    explicit E131Controller(QNetworkInterface const& iface,
                            QNetworkAddressEntry const& address,
                            quint32 line, QObject *parent = 0);

    ~E131Controller();

    /** Send DMX data to a specific port/universe */
    void sendDmx(const quint32 universe, const QByteArray& data);

    /** Return the controller IP address */
    QString getNetworkIP();

    /** Add a universe to the map of this controller */
    void addUniverse(quint32 universe, Type type);

    /** Remove a universe from the map of this controller */
    void removeUniverse(quint32 universe, Type type);

    /** Set input as multicast for the givin QLC+ universe */
    void setInputMulticast(quint32 universe, bool multicast);

    /** Set input as multicast for the givin QLC+ universe */
    void setInputMCastAddress(quint32 universe, QString address, bool legacy);

    /** Set a specific port for the given QLC+ universe */
    void setInputUCastPort(quint32 universe, quint16 port);

    /** Set a specific E1.31 input universe for the given QLC+ universe */
    void setInputUniverse(quint32 universe, quint32 e131Uni);

    /** Set output as multicast for the given QLC+ universe */
    void setOutputMulticast(quint32 universe, bool multicast);

    /** Set a specific multicast IP address for the given QLC+ universe */
    void setOutputMCastAddress(quint32 universe, QString address, bool legacy);

    /** Set a specific unicast IP address for the given QLC+ universe */
    void setOutputUCastAddress(quint32 universe, QString address);

    /** Set a specific port for the given QLC+ universe */
    void setOutputUCastPort(quint32 universe, quint16 port);

    /** Set a specific E1.31 output universe for the given QLC+ universe */
    void setOutputUniverse(quint32 universe, quint32 e131Uni);

    /** Set a specific E1.31 output priority for the given QLC+ universe */
    void setOutputPriority(quint32 universe, quint32 e131Priority);

    /** Set the transmission mode of the ArtNet DMX packets over the network.
     *  It can be 'Full', which transmits always 512 channels, or
     *  'Partial', which transmits only the channels actually used in a
     *  universe */
    void setOutputTransmissionMode(quint32 universe, TransmissionMode mode);

    /** Converts a TransmissionMode value into a human readable string */
    static QString transmissionModeToString(TransmissionMode mode);

    /** Converts a human readable string into a TransmissionMode value */
    static TransmissionMode stringToTransmissionMode(const QString& mode);

    /** Return the list of the universes handled by
     *  this controller */
    QList<quint32> universesList();

    /** Return the specific information for the given universe */
    UniverseInfo *getUniverseInfo(quint32 universe);

    /** Return the global type of this controller */
    Type type();

    /** Return the plugin line associated to this controller */
    quint32 line();

    /** Get the number of packets sent by this controller */
    quint64 getPacketSentNumber();

    /** Get the number of packets received by this controller */
    quint64 getPacketReceivedNumber();

private:
    QSharedPointer<QUdpSocket> getInputSocket(bool multicast, QHostAddress const& address, quint16 port);

private:
    /** The network interface associated to this controller */
    QNetworkInterface m_interface;
    /** The controller IP address as QHostAddress */
    QHostAddress m_ipAddr;

    quint64 m_packetSent;
    quint64 m_packetReceived;

    /** QLC+ line to be used when emitting a signal */
    quint32 m_line;

    /** The UDP socket used to send E131 packets */
    QSharedPointer<QUdpSocket> m_UdpSocket;

    /** Helper class used to create or parse E131 packets */
    QScopedPointer<E131Packetizer> m_packetizer;

    /** Keeps the current dmx values to send only the ones that changed */
    /** It holds values for all the handled universes */
    QMap<quint32, QByteArray*> m_dmxValuesMap;

    /** Map of the QLC+ universes transmitted/received by this
     *  controller, with the related, specific parameters */
    QMap<quint32, UniverseInfo> m_universeMap;

    /** Mutex to handle the change of output IP address or in general
     *  variables that could be used to transmit/receive data */
    QMutex m_dataMutex;

private slots:
    /** Async event raised when new packets have been received */
    void processPendingPackets();

signals:
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value);
};

#endif
