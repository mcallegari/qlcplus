/*
  Q Light Controller
  unixpeperonidevice.h

  Copyright (c) Heikki Junnila

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
