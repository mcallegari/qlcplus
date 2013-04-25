/*
  Q Light Controller Plus
  oscplugin.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef OSCPLUGIN_H
#define OSCPLUGIN_H

#include <QString>
#include <QHash>
#include <QFile>

#include <lo/lo.h>
#include "qlcioplugin.h"

#define OSC_UNIVERSES   4

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

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    void openOutput(quint32 output);

    /** @reimp */
    void closeOutput(quint32 output);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 output, const QByteArray& universe);

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    void openInput(quint32 input);

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
    OSC_Node m_nodes[OSC_UNIVERSES];
};

#endif
