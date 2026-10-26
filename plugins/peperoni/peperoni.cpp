/*
  Q Light Controller
  peperoni.cpp

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

#include <QMessageBox>
#include <QString>
#include <QSet>
#include <QDebug>
#include <libusb.h>

#include "peperonidevice.h"
#include "peperoni.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Peperoni::~Peperoni()
{
    /* Devices hold libusb references, so release them before the context */
    qDeleteAll(uniqueDevices());
    m_devices.clear();

    if (m_ctx != NULL)
        libusb_exit(m_ctx);
}

void Peperoni::init()
{
    m_ctx = NULL;

    if (libusb_init(&m_ctx) != 0)
    {
        qWarning() << "Unable to initialize libusb context!";
        m_ctx = NULL;
        return;
    }

    rescanDevices();
}

QString Peperoni::name() const
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

    /* Lines are contiguous, so walk them in order (QHash order is random) */
    for (quint32 i = 0; i < quint32(m_devices.count()); i++)
        list << m_devices.value(i)->name(i);

    return list;
}

QString Peperoni::pluginInfo() const
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
        connect(m_devices[input], SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                Qt::UniqueConnection);
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
    return outputs();
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

bool Peperoni::canConfigure() const
{
    return true;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void Peperoni::rescanDevices()
{
    if (m_ctx == NULL)
        return;

    libusb_device** devices = NULL;
    ssize_t count = libusb_get_device_list(m_ctx, &devices);
    if (count < 0)
    {
        qWarning() << "[Peperoni] Unable to get USB device list:" << libusb_strerror(libusb_error(count));
        return;
    }

    /* Find which known devices are still there and which ones are new */
    QSet<PeperoniDevice*> stillPresent;
    QList<PeperoniDevice*> newDevices;

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

        if (PeperoniDevice::isPeperoniDevice(&desc) == false)
            continue;

        PeperoniDevice *known = device(dev);
        if (known != NULL)
            stillPresent.insert(known);
        else
            newDevices.append(new PeperoniDevice(this, dev, &desc, 0));
    }

    /* Each PeperoniDevice holds its own reference to its libusb device */
    libusb_free_device_list(devices, 1);

    /* Keep surviving devices in their previous order, regardless of the
       order libusb reports them in, and append the new ones at the end */
    QList<PeperoniDevice*> ordered;
    QList<PeperoniDevice*> deadDevices;
    foreach (PeperoniDevice *dev, uniqueDevices())
    {
        if (stillPresent.contains(dev))
            ordered.append(dev);
        else
            deadDevices.append(dev);
    }
    ordered.append(newDevices);

    /* destructor calls closeAll() */
    qDeleteAll(deadDevices);

    /* Assign contiguous lines. A USBDMX21 owns two consecutive lines */
    m_devices.clear();
    quint32 line = 0;
    foreach (PeperoniDevice *dev, ordered)
    {
        dev->setBaseLine(line);
        for (int l = 0; l < dev->linesCount(); l++)
            m_devices[line++] = dev;
    }

    if (deadDevices.isEmpty() == false || newDevices.isEmpty() == false)
        emit configurationChanged();
}

PeperoniDevice *Peperoni::device(libusb_device *usbdev) const
{
    foreach (PeperoniDevice* dev, m_devices)
    {
        if (dev->device() == usbdev)
            return dev;
    }
    return NULL;
}

QList<PeperoniDevice *> Peperoni::uniqueDevices() const
{
    QList<PeperoniDevice *> list;

    for (quint32 i = 0; i < quint32(m_devices.count()); i++)
    {
        PeperoniDevice *dev = m_devices.value(i);
        if (list.isEmpty() || list.last() != dev)
            list.append(dev);
    }

    return list;
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
