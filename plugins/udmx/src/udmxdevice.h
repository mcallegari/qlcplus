/*
  Q Light Controller
  udmxdevice.h

  Copyright (c) Heikki Junnila
		Lutz Hillebrand

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

#ifndef UDMXDEVICE_H
#define UDMXDEVICE_H

#include <QThread>

struct libusb_device;
struct libusb_device_handle;
struct libusb_device_descriptor;

class UDMXDevice : public QThread
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    UDMXDevice(libusb_device *device, libusb_device_descriptor *desc, QObject* parent = 0);
    virtual ~UDMXDevice();

    /** Find out, whether the given USB device is a uDMX device */
    static bool isUDMXDevice(const libusb_device_descriptor *desc);

    /********************************************************************
     * Device information
     ********************************************************************/
public:
    QString name() const;
    QString infoText() const;

private:
    void extractName();

private:
    QString m_name;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    bool open();
    void close();

    const libusb_device *device() const;

private:
    struct libusb_device* m_device;
    struct libusb_device_descriptor *m_descriptor;
    struct libusb_device_handle* m_handle;

    /********************************************************************
     * Thread
     ********************************************************************/
public:
    void outputDMX(const QByteArray& universe);

private:
    enum TimerGranularity { Unknown, Good, Bad };

    /** Stop the writer thread */
    void stop();

    /** DMX writer thread worker method */
    void run();

private:
    bool m_running;
    QByteArray m_universe;
    double m_frequency;
    TimerGranularity m_granularity;
};

#endif
