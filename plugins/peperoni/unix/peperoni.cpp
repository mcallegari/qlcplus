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
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

bool Peperoni::openOutput(quint32 output)
{
    if (output < quint32(m_devices.size()) &&
        m_devices[output] != NULL)
            return m_devices[output]->open(output, PeperoniDevice::OutputMode);
    return false;
}

void Peperoni::closeOutput(quint32 output)
{
    if (output < quint32(m_devices.size()) &&
        m_devices[output] != NULL)
            m_devices[output]->close(output, PeperoniDevice::OutputMode);
}

QStringList Peperoni::outputs()
{
    QStringList list;
    int i = 1;

    QList <PeperoniDevice*> devList = m_devices.values();
    foreach(PeperoniDevice* dev, devList)
    {
        list << QString("%1: %2").arg(i).arg(dev->name(i - 1));
        i++;
    }
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

    if (output != QLCIOPlugin::invalidLine() &&
        output < quint32(m_devices.size()) &&
        m_devices[output] != NULL)
    {
        str += m_devices[output]->outputInfoText(output);
    }
    else
        qDebug() << "Peperoni invalid output !" << output << m_devices.size();

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void Peperoni::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe)

    if (output < quint32(m_devices.size()) &&
        m_devices[output] != NULL)
            m_devices[output]->outputDMX(output, data);
    else
        qDebug() << "Peperoni invalid output !" << output << m_devices.size();
}

/*************************************************************************
 * Inputs
 *************************************************************************/

bool Peperoni::openInput(quint32 input)
{
    if (input < quint32(m_devices.size()) &&
        m_devices[input] != NULL)
    {
        connect(m_devices[input], SIGNAL(valueChanged(quint32, quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32, quint32,quint32,uchar)));
        return m_devices[input]->open(input, PeperoniDevice::InputMode);
    }
    return false;
}

void Peperoni::closeInput(quint32 input)
{
    if (input < quint32(m_devices.size()) &&
        m_devices[input] != NULL)
    {
        m_devices[input]->close(input, PeperoniDevice::InputMode);
        disconnect(m_devices[input], SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                   this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
    }
}

QStringList Peperoni::inputs()
{
    QStringList list;
    int i = 1;

    QList <PeperoniDevice*> devList = m_devices.values();
    foreach(PeperoniDevice* dev, devList)
    {
        list << QString("%1: %2").arg(i).arg(dev->name(i - 1));
        i++;
    }
    return list;
}

QString Peperoni::inputInfo(quint32 input)
{
    QString str;

    if (input != QLCIOPlugin::invalidLine() &&
        input < quint32(m_devices.size()) &&
        m_devices[input] != NULL)
    {
        str += m_devices[input]->baseInfoText(input);
        str += m_devices[input]->inputInfoText(input);
    }
    else
        qDebug() << "Peperoni invalid input !" << input << m_devices.size();

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
    struct usb_device* dev;
    struct usb_bus* bus;

    /* Treat all devices as dead first, until we find them again. Those
       that aren't found, get destroyed at the end of this function. */
    QHash <quint32, PeperoniDevice*> destroyList(m_devices);
    quint32 line = 0;

    usb_find_busses();
    usb_find_devices();

    /* Iterate through all buses */
    for (bus = usb_get_busses(); bus != NULL; bus = bus->next)
    {
        /* Iterate thru all devices in each bus */
        for (dev = bus->devices; dev != NULL; dev = dev->next)
        {
            if(device(dev) == true)
            {
                /* We already have this device and it's still
                   there. Remove from the destroy list and
                   continue iterating. */
                destroyList.remove(line);
                line++;
                continue;
            }
            else if (PeperoniDevice::isPeperoniDevice(dev) == true)
            {
                /* This is a new device. Create and append. */
                PeperoniDevice* pepdev = new PeperoniDevice(this, dev, line);
                m_devices[line] = pepdev;
                if (PeperoniDevice::outputsNumber(dev) == 2)
                {
                    line++;
                    /* reserve another line with the same device reference */
                    m_devices[line] = pepdev;
                }
                line++;
            }
        }
    }

    qDebug() << "Peperoni devices found:" << m_devices.count();
}

bool Peperoni::device(struct usb_device* usbdev)
{
    foreach(PeperoniDevice* dev, m_devices.values())
    {
        if (dev->device() == usbdev)
            return true;
    }
    return false;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(peperoni, Peperoni)
#endif
