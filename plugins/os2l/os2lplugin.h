/*
  Q Light Controller Plus
  os2lplugin.h

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

#ifndef OS2LPLUGIN_H
#define OS2LPLUGIN_H

#include "qlcioplugin.h"

#define OS2L_HOST_ADDRESS "hostAddress"
#define OS2L_HOST_PORT    "hostPort"

#define OS2L_DEFAULT_PORT 9996

class QTcpServer;

class OS2LPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~OS2LPlugin();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input, quint32 universe);

    /** @reimp */
    void closeInput(quint32 input, quint32 universe);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString inputInfo(quint32 input);

    quint32 universe() const;

protected:
    bool enableTCPServer(bool enable);
    quint16 getHash(QString channel);

protected slots:
    /** Event raised when an incoming connection is requested on
     *  the TCP socket server side */
    void slotProcessNewTCPConnection();
    void slotHostDisconnected();

    /** Async event raised when unicast packets are received */
    void slotProcessTCPPackets();

protected:
    /** Universe selected for this plugin */
    quint32 m_inputUniverse;

    /** Port to listen for incoming packets */
    quint16 m_hostPort;

    /** Reference to the TCP listener for incoming connections */
    QTcpServer *m_tcpServer;

    /** Every time a OS2L message is received, the plugin will calculate a 16 bit checksum
      * of the OS2L command string and add it to
      * this hash table if new, otherwise the plugin will use the hash table
      * to quickly retrieve a unique channel number
      */
    QHash<QString, quint16> m_hashMap;

    QByteArray m_packetLeftOver;

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    /** @reimp */
    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value);
};

#endif
