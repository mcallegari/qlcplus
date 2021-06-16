/*
  Q Light Controller
  unixioenumerator.cpp

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

#include <libusb.h>
#include <QDebug>

#include "unixpeperonidevice.h"
#include "unixioenumerator.h"
#include "peperonidefs.h"

UnixIOEnumerator::UnixIOEnumerator(QObject* parent)
    : IOEnumerator(parent)
    , m_ctx(NULL)
{
    qDebug() << Q_FUNC_INFO;

    int r = libusb_init(&m_ctx);
    if (r != 0)
        qWarning() << "Unable to initialize libusb context!";

    // libusb_set_debug(m_ctx, 3);
}

UnixIOEnumerator::~UnixIOEnumerator()
{
    qDebug() << Q_FUNC_INFO;

    if (m_ctx != NULL)
        libusb_exit(m_ctx);
}

void UnixIOEnumerator::extractData(struct libusb_device* device,
                                   struct libusb_device_descriptor* desc,
                                   QVariant* uid, QString* name)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(device != NULL);
    Q_ASSERT(desc != NULL);
    Q_ASSERT(uid != NULL);
    Q_ASSERT(name != NULL);

    libusb_device_handle* handle = NULL;
    int r = libusb_open(device, &handle);
    if (r == 0)
    {
        char buf[128];
        int r = 0;

        // UID
        r = libusb_get_string_descriptor_ascii(handle, desc->iSerialNumber,
                                               (uchar*) &buf, sizeof(buf));
        if (r > 0)
            *uid = QByteArray(buf, r);
        else
            qWarning() << "Unable to get device serial:" << r;

        // Name
        r = libusb_get_string_descriptor_ascii(handle, desc->iProduct,
                                               (uchar*) &buf, sizeof(buf));
        if (r > 0)
            *name = QString(QByteArray(buf, r));
        else
            qWarning() << "Unable to get product name:" << r;

        // Close device
        libusb_close(handle);
    }
    else
    {
        qWarning() << "Unable to open device:" << r;
    }
}

bool UnixIOEnumerator::isPeperoniDevice(const struct libusb_device_descriptor* descriptor)
{
    Q_ASSERT(descriptor != NULL);

    /* If it's not manufactured by them, we're not interested in it */
    if (descriptor->idVendor != PEPERONI_VID)
        return false;

    if (descriptor->idProduct == PEPERONI_PID_RODIN1     ||
        descriptor->idProduct == PEPERONI_PID_RODIN1_MK3 ||
        descriptor->idProduct == PEPERONI_PID_RODIN2     ||
        descriptor->idProduct == PEPERONI_PID_RODINT     ||
        descriptor->idProduct == PEPERONI_PID_XSWITCH    ||
        descriptor->idProduct == PEPERONI_PID_USBDMX21)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void UnixIOEnumerator::rescan()
{
    qDebug() << Q_FUNC_INFO;

    QList <OutputDevice*> destroyOutputs(m_outputDevices);
    bool changed = false;

    libusb_device** devices = NULL;
    ssize_t count = libusb_get_device_list(m_ctx, &devices);
    for (ssize_t i = 0; i < count; i++)
    {
        libusb_device* dev = devices[i];
        Q_ASSERT(dev != NULL);

        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0)
        {
            qWarning() << "Unable to get device descriptor:" << r;
            continue;
        }

        // We're only interested in Peperoni devices
        if (isPeperoniDevice(&desc) == false)
            continue;

        QVariant uid;
        QString name;
        extractData(dev, &desc, &uid, &name);

        OutputDevice* od = outputDevice(uid);
        if (od == NULL)
        {
            // New device
            od = new UnixPeperoniDevice(uid, name, dev, &desc, this);
            m_outputDevices << od;
            changed = true;
        }
        else
        {
            // Device is known and still present
            destroyOutputs.removeAll(od);
        }
    }

    // Release the list's references to the devices
    libusb_free_device_list(devices, 1);

    // Destroy all devices that are no longer present
    while (destroyOutputs.isEmpty() == false)
    {
        delete destroyOutputs.takeFirst();
        changed = true;
    }

    // Emit signal if something was changed
    if (changed == true)
        emit configurationChanged();
}

QList <OutputDevice*> UnixIOEnumerator::outputDevices() const
{
    return m_outputDevices;
}

QList <InputDevice*> UnixIOEnumerator::inputDevices() const
{
    return m_inputDevices;
}

OutputDevice* UnixIOEnumerator::outputDevice(const QVariant& uid) const
{
    QListIterator <OutputDevice*> it(m_outputDevices);
    while (it.hasNext() == true)
    {
        OutputDevice* dev(it.next());
        if (dev->uid() == uid)
            return dev;
    }

    return NULL;
}

InputDevice* UnixIOEnumerator::inputDevice(const QVariant& uid) const
{
    Q_UNUSED(uid);
    return NULL;
}
