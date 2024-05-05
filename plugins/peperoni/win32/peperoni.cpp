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

#include <QtCore>
#include <QtWidgets/QMessageBox>
#include <QStringList>
#include <windows.h>
#include <QDebug>
#include <QString>

#include "usbdmx-dynamic.h"
#include "peperonidevice.h"
#include "peperoni.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Peperoni::~Peperoni()
{
    while (m_devices.isEmpty() == false)
        delete m_devices.takeFirst();

    // @TODO: Free m_usbdmx???
}

void Peperoni::init()
{
    /* Load usbdmx.dll */
    m_usbdmx = usbdmx_init();
    if (m_usbdmx == NULL)
    {
        qWarning() << "Loading USBDMX.DLL failed.";
    }
    else if (USBDMX_DLL_VERSION_CHECK(m_usbdmx) == FALSE)
    {
        /* verify USBDMX dll version */
        qWarning() << "USBDMX.DLL version does not match. Abort.";
        qWarning() << "Found" << m_usbdmx->version() << "but expected"
                   << USBDMX_DLL_VERSION;
    }
    else
    {
        qDebug() << "Using USBDMX.DLL version" << m_usbdmx->version();
        rescanDevices();
    }
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

bool Peperoni::openOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(universe)
    if (m_usbdmx == NULL)
        return false;

    if (output < quint32(m_devices.size()))
        return m_devices.at(output)->open();

    return false;
}

void Peperoni::closeOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(universe)
    if (m_usbdmx == NULL)
        return;

    if (output < quint32(m_devices.size()))
        m_devices.at(output)->close();
}

QStringList Peperoni::outputs()
{
    QStringList list;

    QListIterator <PeperoniDevice*> it(m_devices);
    while (it.hasNext() == true)
        list << it.next()->name();

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

    if (m_usbdmx == NULL)
    {
        str += QString("<H3>%1</H3>").arg(name());
        str += QString("<P>");
        str += tr("The shared library usbdmx.dll could not be found or is too old to be used with QLC.");
        str += QString("</P>");
    }
    else if (output != QLCIOPlugin::invalidLine() && output < quint32(m_devices.size()))
    {
        str += m_devices.at(output)->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void Peperoni::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(dataChanged)

    if (output < quint32(m_devices.size()))
        m_devices.at(output)->outputDMX(data);
}

void Peperoni::rescanDevices()
{
    USHORT id = 0;

    if (m_usbdmx == NULL)
        return;

    for (id = 0; id < 32; id++)
    {
        HANDLE handle = NULL;
        if (m_usbdmx->open(id, &handle) == TRUE)
        {
            /* We don't need the handle now. */
            m_usbdmx->close(handle);

            if (id >= m_devices.size())
            {
                PeperoniDevice* device;

                /* Device was opened successfully and it's
                   a new device. Append it to our list. */
                device = new PeperoniDevice(this, m_usbdmx, id);
                m_devices.append(device);
            }
            else
            {
                /* We already have a device with this id. Try
                   the next one. */
            }
        }
        else
        {
            /* This device ID doesn't exist and neither does any
               consecutive id, so we can stop looking. */
            break;
        }
    }

    /* Remove those devices that aren't present. I.e. if our search
       stopped into an ID that is equal to or less than the current number
       of devices, one or more devices are no longer present. */
    while (id < m_devices.size())
        delete m_devices.takeLast();

    /* Because all devices have just plain and dull IDs, we can't know,
       whether the user removed one XSwitch and plugged in a Rodin1,
       that ends up getting the same ID. Therefore, force all known devices
       to reload their info again. */
    QListIterator <PeperoniDevice*> it(m_devices);
    while (it.hasNext() == true)
        it.next()->rehash();
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
