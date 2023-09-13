/*
  Q Light Controller
  udmx.cpp

  Copyright (c)	Lutz Hillebrand
                Heikki Junnila

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

#include <QMessageBox>
#include <QString>
#include <QDebug>

#include "udmxdevice.h"
#include "udmx.h"

UDMX::~UDMX()
{
}

void UDMX::init()
{
    m_ctx = NULL;

    if (libusb_init(&m_ctx) != 0)
        qWarning() << "Unable to initialize libusb context!";

    rescanDevices();
}

QString UDMX::name()
{
    return QString("uDMX");
}

int UDMX::capabilities() const
{
    return QLCIOPlugin::Output;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

bool UDMX::openOutput(quint32 output, quint32 universe)
{
    if (output < quint32(m_devices.size()))
    {
        addToMap(universe, output, Output);
        return m_devices.at(output)->open();
    }
    return false;
}

void UDMX::closeOutput(quint32 output, quint32 universe)
{
    if (output < quint32(m_devices.size()))
    {
        removeFromMap(output, universe, Output);
        m_devices.at(output)->close();
    }
}

QStringList UDMX::outputs()
{
    QStringList list;

    QListIterator <UDMXDevice*> it(m_devices);
    while (it.hasNext() == true)
        list << it.next()->name();

    return list;
}

QString UDMX::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output support for Anyma uDMX devices.");
    str += QString("</P>");

    return str;
}

QString UDMX::outputInfo(quint32 output)
{
    QString str;

    if (output != QLCIOPlugin::invalidLine() && output < quint32(m_devices.size()))
    {
        str += m_devices.at(output)->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void UDMX::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)

    if (output < quint32(m_devices.size()) && dataChanged)
        m_devices.at(output)->outputDMX(data);
}

void UDMX::rescanDevices()
{
    /* Treat all devices as dead first, until we find them again. Those
       that aren't found, get destroyed at the end of this function. */
    QList <UDMXDevice*> destroyList(m_devices);
    int devCount = m_devices.count();

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
        UDMXDevice* udev = device(dev);
        if (udev != NULL)
        {
            /* We already have this device and it's still
               there. Remove from the destroy list and
               continue iterating. */
            destroyList.removeAll(udev);
            continue;
        }
        else if (UDMXDevice::isUDMXDevice(&desc) == true)
        {
            /* This is a new device. Create and append. */
            udev = new UDMXDevice(dev, &desc, this);
            m_devices.append(udev);
        }
    }

    /* Destroy those devices that were no longer found. */
    while (destroyList.isEmpty() == false)
    {
        UDMXDevice* udev = destroyList.takeFirst();
        m_devices.removeAll(udev);
        delete udev;
    }

    if (m_devices.count() != devCount)
        emit configurationChanged();
}

UDMXDevice* UDMX::device(struct libusb_device* usbdev)
{
    QListIterator <UDMXDevice*> it(m_devices);
    while (it.hasNext() == true)
    {
        UDMXDevice* udev = it.next();
        if (udev->device() == usbdev)
            return udev;
    }

    return NULL;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void UDMX::configure()
{
    int r = QMessageBox::question(NULL, name(),
                                  tr("Do you wish to re-scan your hardware?"),
                                  QMessageBox::Yes, QMessageBox::No);
    if (r == QMessageBox::Yes)
        rescanDevices();
}

bool UDMX::canConfigure()
{
    return true;
}
