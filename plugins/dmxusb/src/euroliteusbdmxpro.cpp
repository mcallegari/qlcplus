/*
  Q Light Controller Plus
  euroliteusbdmxpro.cpp

  Copyright (C) Massimo Callegari

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

#include "euroliteusbdmxpro.h"

#include <QDebug>
#include <QDir>

EuroliteUSBDMXPro::EuroliteUSBDMXPro(const QString& serial, const QString& name,
                       const QString &vendor, void *usb_ref, quint32 id)
    : DMXUSBWidget(serial, name, vendor, 0)
{
    Q_UNUSED(id)
#ifdef LIBFTDI1
    m_device = (libusb_device *)usb_ref;
#else
    m_device = (struct usb_device *)usb_ref;
#endif
}

EuroliteUSBDMXPro::~EuroliteUSBDMXPro()
{
    if (m_file.isOpen() == true)
        m_file.close();
}

DMXUSBWidget::Type EuroliteUSBDMXPro::type() const
{
    return DMXUSBWidget::DMX4ALL;
}

bool EuroliteUSBDMXPro::checkReply()
{
    QByteArray reply = m_file.readAll();
    qDebug() << Q_FUNC_INFO << "Reply: " << QString::number(reply[0], 16);
    for (int i = 0; i < reply.count(); i++)
    {
        if (reply[i] == 'G')
        {
            qDebug() << Q_FUNC_INFO << name() << "Good connection.";
            return true;
        }
    }

    qWarning() << Q_FUNC_INFO << name() << "Response failed (got: " << reply << ")";
    return false;
}

bool EuroliteUSBDMXPro::sendChannelValue(int channel, uchar value)
{
    QByteArray chanMsg;
    QString msg;
    chanMsg.append(msg.sprintf("C%03dL%03d", channel, value));
    return ftdi()->write(chanMsg);
}

QString EuroliteUSBDMXPro::getDeviceName()
{
    if (m_device == NULL)
        return QString();

    QDir sysfsDevDir("/sys/bus/usb/devices");
    QStringList devDirs = sysfsDevDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 1- scan all the devices in the device bus
    foreach (QString dir, devDirs)
    {
        qDebug() << QString::number(m_device->bus->location);

#ifdef LIBFTDI1
        if (dir.startsWith(QString::number(libusb_get_port_number(m_device))) &&
#else
        if (dir.startsWith(QString::number(m_device->bus->location)) &&
#endif
            dir.contains(":") == false)
        {
            // 2- Match the product name
            qDebug() << "SYSFS Directory:" << dir;
            QFile pName(QString("/sys/bus/usb/devices/%1/product").arg(dir));
            if (pName.open(QIODevice::ReadOnly))
            {
                QString prodString = pName.readAll();
                pName.close();
                //qDebug() << "Got prod string:" << prodString.simplified() << "name:" << name;
                if (name() == prodString.simplified())
                {
                    QDir devPorts("/sys/bus/usb/devices/" + dir);
                    QStringList devDirs = devPorts.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

                    // 3- scan all the device ports
                    foreach (QString portDir, devDirs)
                    {
                        if (portDir.startsWith(dir))
                        {
                            QDir ttyDir(QString("/sys/bus/usb/devices/%1/%2/tty").arg(dir).arg(portDir));
                            qDebug() << "ttyDir:" << ttyDir.absolutePath();

                            // 4- extract the tty port number
                            if (ttyDir.exists())
                            {
                                QStringList ttyList = ttyDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                                foreach(QString ttyName, ttyList)
                                {
                                    qDebug() << "This EuroliteUSBDMXPro adapter will use" << QString("/dev/" + ttyName);
                                    return QString("/dev/" + ttyName);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return QString();
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/
bool EuroliteUSBDMXPro::open(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

    QString ttyName = getDeviceName();
    if (ttyName.isEmpty())
        m_file.setFileName("/dev/ttyACM0");
    else
        m_file.setFileName(ttyName);

    m_file.unsetError();
    if (m_file.open(QIODevice::ReadWrite | QIODevice::Unbuffered) == false)
    {
        qWarning() << "EuroliteUSBDMXPro output cannot be opened:"
                   << m_file.errorString();
        return false;
    }

    return true;
}

bool EuroliteUSBDMXPro::close(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

    if (m_file.isOpen() == true)
        m_file.close();

    return true;
}

QString EuroliteUSBDMXPro::uniqueName(ushort line, bool input) const
{
    Q_UNUSED(line)
    Q_UNUSED(input)
    return QString("%1").arg(name());
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EuroliteUSBDMXPro::additionalInfo() const
{
    QString info;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2 (%3)").arg(QObject::tr("Protocol"))
                                         .arg("DMX4ALL DMX-USB")
                                         .arg(QObject::tr("Output"));
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Manufacturer"))
                                         .arg(vendor());
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Serial number"))
                                                 .arg(serial());
    info += QString("</P>");

    return info;
}

/****************************************************************************
 * Write universe data
 ****************************************************************************/

bool EuroliteUSBDMXPro::writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)

    if (m_file.isOpen() == false)
        return false;

    QByteArray request(data);
    request.prepend(char(EUROLITE_USB_DMX_PRO_DMX_ZERO)); // DMX start code (Which constitutes the + 1 below)
    request.prepend(((data.size() + 1) >> 8) & 0xff); // Data length MSB
    request.prepend((data.size() + 1) & 0xff); // Data length LSB
    request.prepend(EUROLITE_USB_DMX_PRO_SEND_DMX_RQ); // Command - first port
    request.prepend(EUROLITE_USB_DMX_PRO_START_OF_MSG); // Start byte
    request.append(EUROLITE_USB_DMX_PRO_END_OF_MSG); // Stop byte

    if (m_file.write(request) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
        return false;
    }
    else
    {
        return true;
    }
}


