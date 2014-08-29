/*
  Q Light Controller Plus
  oscplugin.h

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

#ifndef OSCPLUGIN_H
#define OSCPLUGIN_H

#include <QString>
#include <QHash>
#include <QFile>

#include <lo/lo.h>
#include "qlcioplugin.h"

class OSCPlugin;

typedef struct
{
    int input;
    OSCPlugin *plugin;
} OSC_cbk_info;

typedef struct
{
    QString m_port;                 /** The port the OSC server is listening to */
    lo_address m_outAddr;           /** The address the OSC server will send to (libLO form) */
    QString m_outAddrStr;           /** The address the OSC server will send to (Qt form) */
    lo_server_thread m_serv_thread; /** The actual OSC server thread */
    OSC_cbk_info m_callbackInfo;    /** Callback called by the OSC server when receiving data */

    /** This is fundamental for OSC plugin. Every time a OSC signal is received,
      * QLC+ will calculate a 16 bit checksum of the OSC path and add it to
      * this hash table if new, otherwise QLC+ will use the hash table
      * to quickly retrieve a unique channel number
      */
    QHash<QString, quint16> m_hash;

    /** Keeps the current dmx values to send only the ones that changed */
    /** It holds values for a whole 4 universes address (512 * 4) */
    QByteArray m_dmxValues;

    /** XY pads have 2 bytes in a single message. This variable is used to keep the */
    /** first byte, so when the second arrives the message can be composed correctly */
    uchar m_multiDataFirst;

} OSC_Node;

class OSCPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)
#endif

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~OSCPlugin();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

    /** @reimp */
    void setParameter(QString name, QVariant &value)
    { Q_UNUSED(name); Q_UNUSED(value); }

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    bool openOutput(quint32 output);

    /** @reimp */
    void closeOutput(quint32 output);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input);

    /** @reimp */
    void closeInput(quint32 input);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString inputInfo(quint32 input);

    /** @reimp */
    void sendFeedBack(quint32 input, quint32 channel, uchar value, const QString& key);

    /** send an event to the upper layers */
    void sendValueChanged(quint32 input, QString path, uchar value);

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    QString getPort(int num);

    void setPort(int num, QString port);

    QString getOutputAddress(int num);

    void setOutputAddress(int num, QString addr);

private:
    quint16 getHash(quint32 line, QString path);

private:
    OSC_Node m_nodes[QLCIOPLUGINS_UNIVERSES];
};

#endif
