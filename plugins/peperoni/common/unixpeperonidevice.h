/*
  Q Light Controller
  unixpeperonidevice.h

  Copyright (c) Heikki Junnila

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

#ifndef PEPERONIDEVICE_H
#define PEPERONIDEVICE_H

#include <QObject>
#include "outputdevice.h"

struct libusb_device_descriptor;
struct libusb_device_handle;
struct libusb_device;

class UnixPeperoniDevice : public OutputDevice
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    UnixPeperoniDevice(const QVariant& uid, const QString& name,
                       struct libusb_device* device,
                       const struct libusb_device_descriptor* desc,
                       QObject* parent = 0);
    virtual ~UnixPeperoniDevice();

    /** @reimp */
    void open();

    /** @reimp */
    void close();

    /** @reimp */
    bool isOpen() const;

    /** @reimp */
    void writeChannel(ushort channel, uchar value);

    /** @reimp */
    void writeUniverse(const QByteArray& universe);

private:
    struct libusb_device* m_device;
    struct libusb_device_handle* m_handle;
    int m_firmwareVersion;
    int m_blockingControlWrite;
    int m_configuration;
    QByteArray m_bulkBuffer;
};

#endif
