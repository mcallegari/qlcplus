/*
  Q Light Controller
  udmx.cpp

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

#ifdef WIN32
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

void UDMX::openOutput(quint32 output)
{
    if (output < quint32(m_devices.size()))
        m_devices.at(output)->open();
}

void UDMX::closeOutput(quint32 output)
{
    if (output < quint32(m_devices.size()))
        m_devices.at(output)->close();
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

void UDMX::writeUniverse(quint32 output, const QByteArray& universe)
{
    if (output < quint32(m_devices.size()))
        m_devices.at(output)->outputDMX(universe);
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

Q_EXPORT_PLUGIN2(udmx, UDMX)
