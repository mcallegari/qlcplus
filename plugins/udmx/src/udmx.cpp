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

#if defined(WIN32) || defined(Q_OS_WIN)
#   include <Windows.h>
#   include "libusb_dyn.h"
#else
#   include <usb.h>
#endif

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
    usb_init();
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
    int i = 1;

    QListIterator <UDMXDevice*> it(m_devices);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->name());
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

void UDMX::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe)

    if (output < quint32(m_devices.size()))
        m_devices.at(output)->outputDMX(data);
}

void UDMX::rescanDevices()
{
    struct usb_device* dev;
    struct usb_bus* bus;

    /* Treat all devices as dead first, until we find them again. Those
       that aren't found, get destroyed at the end of this function. */
    QList <UDMXDevice*> destroyList(m_devices);

    usb_find_busses();
    usb_find_devices();

    /* Iterate thru all buses */
    for (bus = usb_get_busses(); bus != NULL; bus = bus->next)
    {
        /* Iterate thru all devices in each bus */
        for (dev = bus->devices; dev != NULL; dev = dev->next)
        {
            UDMXDevice* udev = device(dev);
            if (udev != NULL)
            {
                /* We already have this device and it's still
                   there. Remove from the destroy list and
                   continue iterating. */
                destroyList.removeAll(udev);
                continue;
            }
            else if (UDMXDevice::isUDMXDevice(dev) == true)
            {
                /* This is a new device. Create and append. */
                udev = new UDMXDevice(dev, this);
                m_devices.append(udev);
            }
        }
    }

    /* Destroy those devices that were no longer found. */
    while (destroyList.isEmpty() == false)
    {
        UDMXDevice* udev = destroyList.takeFirst();
        m_devices.removeAll(udev);
        delete udev;
    }
}

UDMXDevice* UDMX::device(struct usb_device* usbdev)
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

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(udmx, UDMX)
#endif
