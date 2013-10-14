/*
  Q Light Controller
  peperoni.cpp

  Copyright (c)	Heikki Junnila

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

#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <usb.h>

#include "peperonidevice.h"
#include "peperoni.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Peperoni::~Peperoni()
{
}

void Peperoni::init()
{
    usb_init();
    rescanDevices();
}

QString Peperoni::name()
{
    return QString("Peperoni");
}

int Peperoni::capabilities() const
{
    return QLCIOPlugin::Output;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

void Peperoni::openOutput(quint32 output)
{
    if (output < quint32(m_devices.size()))
        m_devices.at(output)->open();
}

void Peperoni::closeOutput(quint32 output)
{
    if (output < quint32(m_devices.size()))
        m_devices.at(output)->close();
}

QStringList Peperoni::outputs()
{
    QStringList list;
    int i = 1;

    QListIterator <PeperoniDevice*> it(m_devices);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->name());
    return list;
}

QString Peperoni::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<H3>%1</H3>").arg(name());
    str += QString("<P>");
    str += tr("This plugin provides DMX output support for Peperoni DMX devices.");
    str += QString("</P>");

    return str;
}

QString Peperoni::outputInfo(quint32 output)
{
    QString str;

    if (output == QLCIOPlugin::invalidLine() && output < quint32(m_devices.size()))
    {
        str += m_devices.at(output)->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void Peperoni::writeUniverse(quint32 output, const QByteArray& universe)
{
    if (output < quint32(m_devices.size()))
        m_devices.at(output)->outputDMX(universe);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void Peperoni::configure()
{
    int r = QMessageBox::question(NULL, name(),
                                  tr("Do you wish to re-scan your hardware?"),
                                  QMessageBox::Yes, QMessageBox::No);
    if (r == QMessageBox::Yes)
        rescanDevices();
}

bool Peperoni::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void Peperoni::rescanDevices()
{
    struct usb_device* dev;
    struct usb_bus* bus;

    /* Treat all devices as dead first, until we find them again. Those
       that aren't found, get destroyed at the end of this function. */
    QList <PeperoniDevice*> destroyList(m_devices);

    usb_find_busses();
    usb_find_devices();

    /* Iterate thru all buses */
    for (bus = usb_get_busses(); bus != NULL; bus = bus->next)
    {
        /* Iterate thru all devices in each bus */
        for (dev = bus->devices; dev != NULL; dev = dev->next)
        {
            PeperoniDevice* pepdev = device(dev);
            if (pepdev != NULL)
            {
                /* We already have this device and it's still
                   there. Remove from the destroy list and
                   continue iterating. */
                destroyList.removeAll(pepdev);
                continue;
            }
            else if (PeperoniDevice::isPeperoniDevice(dev) == true)
            {
                /* This is a new device. Create and append. */
                pepdev = new PeperoniDevice(this, dev);
                m_devices.append(pepdev);
            }
        }
    }

    /* Destroy those devices that were no longer found. */
    while (destroyList.isEmpty() == false)
    {
        PeperoniDevice* pepdev = destroyList.takeFirst();
        m_devices.removeAll(pepdev);
        delete pepdev;
    }
}

PeperoniDevice* Peperoni::device(struct usb_device* usbdev)
{
    QListIterator <PeperoniDevice*> it(m_devices);
    while (it.hasNext() == true)
    {
        PeperoniDevice* dev = it.next();
        if (dev->device() == usbdev)
            return dev;
    }

    return NULL;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(peperoni, Peperoni)
#endif
