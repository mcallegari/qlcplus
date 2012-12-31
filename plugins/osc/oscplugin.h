/*
  Q Light Controller
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

#define OSC_INPUTS   4

class OSCPlugin;

typedef struct
{
    int input;
    OSCPlugin *plugin;
} OSC_cbk_info;

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
    void openOutput(quint32 output) { Q_UNUSED(output); }

    /** @reimp */
    void closeOutput(quint32 output) { Q_UNUSED(output); }

    /** @reimp */
    QStringList outputs()  { return QStringList(); }

    /** @reimp */
    QString outputInfo(quint32 output) { Q_UNUSED(output); return QString(); }

    /** @reimp */
    void writeUniverse(quint32 output, const QByteArray& universe)
        { Q_UNUSED(output); Q_UNUSED(universe); }

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
    void sendFeedBack(quint32 input, quint32 channel, uchar value)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); }

    /** send an event to the upper layers */
    void sendValueChanged(quint32 input, QString path, uchar value);

/*
private:
    int messageCallback(const char *path, const char *types, lo_arg **argv,
                         int argc, void *data, void *user_data);

    void errorCallback(int num, const char *msg, const char *path);
*/

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

private:
    quint16 getHash(QString path);

private:
    /** The port the OSC server is listening */
    QString m_ports[OSC_INPUTS];

    /** The actual OSC server thread */
    lo_server_thread m_serv_threads[OSC_INPUTS];

    OSC_cbk_info m_callbackInfo[OSC_INPUTS];

    /** This is fundamental for OSC plugin. Every time a OSC signal is received,
      * QLC+ will calculate a 16 bit checksum of the OSC path and add it to
      * this hash table if new, otherwise QLC+ will use the hash table
      * to quickly retrieve a unique channel number
      */
    QHash<QString, quint16> m_hash;
};

#endif
