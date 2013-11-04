/*
  Q Light Controller
  peperonidevice.h

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


struct usb_dev_handle;
struct usb_device;
class QString;
class QByteArray;

class PeperoniDevice : public QObject
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    PeperoniDevice(QObject* parent, struct usb_device* device);
    virtual ~PeperoniDevice();

    /** Find out, whether the given USB device is a Peperoni device */
    static bool isPeperoniDevice(const struct usb_device* device);

    /********************************************************************
     * Device information
     ********************************************************************/
public:
    QString name() const;
    QString infoText() const;

protected:
    void extractName();

protected:
    QString m_name;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    void open();
    void close();

    const struct usb_device* device() const;
    const usb_dev_handle* handle() const;

protected:
    struct usb_device* m_device;
    usb_dev_handle* m_handle;
    int m_firmwareVersion;
    int m_blockingControlWrite;
    QByteArray m_bulkBuffer;

    /********************************************************************
     * Write
     ********************************************************************/
public:
    void outputDMX(const QByteArray& universe);
};

#endif
