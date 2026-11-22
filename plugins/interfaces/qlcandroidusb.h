/*
  Q Light Controller Plus
  qlcandroidusb.h

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

#ifndef QLCANDROIDUSB_H
#define QLCANDROIDUSB_H

#include <QString>
#include <QVector>
#include <QList>

class QObject;

/**
 * Raw USB access on Android, shared by the I/O plugins.
 * Enumeration, permission and open go through the Java USB Host API
 * (platforms/android/src/org/qlcplus/usb/UsbHelper.java), then
 * transfers are performed with usbdevfs ioctls on the connection fd.
 *
 * Device IDs are passed as a flat list of VID/PID pairs.
 */
namespace AndroidUSB
{
    struct DeviceInfo
    {
        QString path;
        quint16 vendorID;
        quint16 productID;
        int interfaceNumber;
        QString manufacturer;
        QString product;
        QString serial;
    };

    struct Handle
    {
        int fd = -1;
        int epIn = 0;
        int epOut = 0;
        int inMaxPacket = 0;
        int outMaxPacket = 0;
        int interfaceNumber = 0;
        int interfaceCount = 0;
        int bcdDevice = 0;
        /** CDC communication interface paired with a CDC data interface, or -1 */
        int commInterface = -1;
    };

    /** Invoke @listener's slotDeviceAdded(uint,uint) / slotDeviceRemoved(uint,uint)
     *  on attach/detach of, or permission granted to, devices matching @ids */
    bool addListener(QObject *listener, const QVector<int> &ids);

    /** List the interfaces of class @interfaceClass (-1 = any) of the devices
     *  matching @ids. Permission is requested for devices that don't have it yet,
     *  and they are listed once granted (see addListener) */
    QList<DeviceInfo> enumerate(int interfaceClass, const QVector<int> &ids);

    /** Open and claim the interface at @path, filling @handle */
    bool open(const QString &path, Handle &handle);

    /** Release the interface at @path */
    void close(const QString &path);

    /** Bulk/interrupt transfer on @endpoint. @timeoutMs = 0 waits forever.
     *  Return the transferred bytes or -1 (errno set) */
    int bulkTransfer(const Handle &handle, int endpoint, void *data, int length, int timeoutMs);

    /** Control transfer. Return the transferred bytes or -1 (errno set) */
    int controlTransfer(const Handle &handle, quint8 requestType, quint8 request,
                        quint16 value, quint16 index, void *data, quint16 length, int timeoutMs);
}

#endif
