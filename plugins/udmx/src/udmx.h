/*
  Q Light Controller
  udmx.h

  Copyright (c)	Lutz Hillebrand
                Heikki Junnila

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

#ifndef UDMX_H
#define UDMX_H

#include <QStringList>
#include <QList>

#include "qlcioplugin.h"

class UDMXDevice;

class UDMX : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~UDMX();

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

private:
    /** Attempt to find all uDMX devices */
    void rescanDevices();

    /** Get a UDMXDevice entry by its usbdev struct */
    UDMXDevice* device(struct usb_device* usbdev);

private:
    /** List of available devices */
    QList <UDMXDevice*> m_devices;

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    void openInput(quint32 input) { Q_UNUSED(input); }

    /** @reimp */
    void closeInput(quint32 input) { Q_UNUSED(input); }

    /** @reimp */
    QStringList inputs() { return QStringList(); }

    /** @reimp */
    QString inputInfo(quint32 input) { Q_UNUSED(input); return QString(); }

    /** @reimp */
    void sendFeedBack(quint32 input, quint32 channel, uchar value, const QString& key)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); Q_UNUSED(key); }

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();
};

#endif
