/*
  Q Light Controller
  peperoni.cpp

  Copyright (c)	Heikki Junnila

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

#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <libusb.h>

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
    m_ctx = NULL;

    if (libusb_init(&m_ctx) != 0)
        qWarning() << "Unable to initialize libusb context!";

    rescanDevices();
}

QString Peperoni::name()
{
    return QString("Peperoni");
}

int Peperoni::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

bool Peperoni::openOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(universe)
    if (m_devices.contains(output) == false)
        return false;

    if (m_devices[output] != NULL)
        return m_devices[output]->open(output, PeperoniDevice::OutputMode);
    return false;
}

void Peperoni::closeOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(universe)
    if (m_devices.contains(output) && m_devices[output] != NULL)
            m_devices[output]->close(output, PeperoniDevice::OutputMode);
}

QStringList Peperoni::outputs()
{
    QStringList list;
    int i = 0;

    QList <PeperoniDevice*> devList = m_devices.values();
    foreach (PeperoniDevice* dev, devList)
        list << dev->name(i++);

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
    str += tr("This plugin provides DMX input and output support for Peperoni DMX devices.");
    str += QString("</P>");

    return str;
}

QString Peperoni::outputInfo(quint32 output)
{
    QString str;

    if (m_devices.contains(output) == false)
        return str;

    if (m_devices[output] != NULL)
    {
        str += m_devices[output]->outputInfoText(output);
    }
    else
        qDebug() << "Peperoni invalid output!" << output << m_devices.size();

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void Peperoni::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(dataChanged)

    if (m_devices.contains(output) == false)
        return;

    if (m_devices[output] != NULL)
        m_devices[output]->outputDMX(output, data);
    else
        qDebug() << "Peperoni invalid output!" << output << m_devices.size();
}

/*************************************************************************
 * Inputs
 *************************************************************************/

bool Peperoni::openInput(quint32 input, quint32 universe)
{
    Q_UNUSED(universe)
    if (m_devices.contains(input) == false)
        return false;

    if (m_devices[input] != NULL)
    {
        connect(m_devices[input], SIGNAL(valueChanged(quint32, quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32, quint32,quint32,uchar)));
        return m_devices[input]->open(input, PeperoniDevice::InputMode);
    }
    return false;
}

void Peperoni::closeInput(quint32 input, quint32 universe)
{
    Q_UNUSED(universe)
    if (m_devices.contains(input) && m_devices[input] != NULL)
    {
        m_devices[input]->close(input, PeperoniDevice::InputMode);
        disconnect(m_devices[input], SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                   this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
    }
}

QStringList Peperoni::inputs()
{
    QStringList list;
    int i = 0;

    QList <PeperoniDevice*> devList = m_devices.values();
    foreach (PeperoniDevice* dev, devList)
        list << dev->name(i++);

    return list;
}

QString Peperoni::inputInfo(quint32 input)
{
    QString str;

    if (m_devices.contains(input) == false)
        return str;

    if (m_devices[input] != NULL)
    {
        str += m_devices[input]->baseInfoText(input);
        str += m_devices[input]->inputInfoText(input);
    }
    else
        qDebug() << "Peperoni invalid input!" << input << m_devices.size();

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
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
    /* Treat all devices as dead first, until we find them again. Those
       that aren't found, get destroyed at the end of this function. */
    QHash <quint32, PeperoniDevice*> destroyList(m_devices);
    quint32 line = 0;
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

        if (device(dev) == true)
        {
            /* We already have this device and it's still
               there. Remove from the destroy list and
               continue iterating. */
            destroyList.remove(line);
            line++;
            continue;
        }
        else if (PeperoniDevice::isPeperoniDevice(&desc) == true)
        {
            /* This is a new device. Create and append. */
            PeperoniDevice* pepdev = new PeperoniDevice(this, dev, &desc, line);
            m_devices[line] = pepdev;
            if (PeperoniDevice::outputsNumber(&desc) == 2)
            {
                line++;
                /* reserve another line with the same device reference */
                m_devices[line] = pepdev;
            }
            line++;
        }
    }

    //qDebug() << "[Peperoni] Need to destroy" << destroyList.count() << "devices";
    QHashIterator<quint32, PeperoniDevice*> it(destroyList);
    while (it.hasNext())
    {
        it.next();
        PeperoniDevice *dev = m_devices.take(it.key());
        dev->closeAll();
        delete dev;
    }

    //qDebug() << "[Peperoni] devices found:" << m_devices.count();
    if (m_devices.count() != devCount)
        emit configurationChanged();
}

bool Peperoni::device(struct libusb_device* usbdev)
{
    foreach (PeperoniDevice* dev, m_devices.values())
    {
        if (dev->device() == usbdev)
            return true;
    }
    return false;
}


/*****************************************************************************
 * Hotplug
 *****************************************************************************/

void Peperoni::slotDeviceAdded(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << QString::number(vid, 16) << QString::number(pid, 16);

    if (!PeperoniDevice::isPeperoniDevice(vid, pid))
    {
        qDebug() << Q_FUNC_INFO << "not a Peperoni device";
        return;
    }

    rescanDevices();
}

void Peperoni::slotDeviceRemoved(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << QString::number(vid, 16) << QString::number(pid, 16);

    if (!PeperoniDevice::isPeperoniDevice(vid, pid))
    {
        qDebug() << Q_FUNC_INFO << "not a Peperoni device";
        return;
    }

    rescanDevices();
}
