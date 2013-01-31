/*
  Q Light Controller
  artnetplugin.h

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

#ifndef ARTNETPLUGIN_H
#define ARTNETPLUGIN_H

#include <QNetworkAddressEntry>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QString>
#include <QHash>
#include <QFile>

//#include <artnet/artnet.h>

#include "qlcioplugin.h"
#include "artnetcontroller.h"

class ArtNetPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~ArtNetPlugin();

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
    void sendFeedBack(quint32 input, quint32 channel, uchar value)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); }

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

    QList<QNetworkAddressEntry> interfaces();

    /** Get a list of the available outputs mapped from the detect network interfaces */
    QList<QString> mappedOutputs();

    /** Get a list of the available outputs ports */
    QList<int> mappedPorts();

    void remapOutputs(QList<QString> IPs, QList<int> ports);

    /** Returns the mapped controllers created on openOutput */
    QList<ArtNetController *> mappedControllers();

private:
    /** List holding the detected system network interfaces */
    QList<QNetworkAddressEntry> m_netInterfaces;

    /** Map of the IPs associated to each plugin output */
    /** Basically these are those selected in the config panel */
    QList<QString> m_outputIPlist;

    /** Map of the ports associated to each plugin output */
    /** (not to be confused with network ports !!) */
    QList<int> m_outputPortList;

    /** Map of the ArtNet controllers associated to each plugin output */
    QList<ArtNetController*> m_controllersList;
};

#endif
