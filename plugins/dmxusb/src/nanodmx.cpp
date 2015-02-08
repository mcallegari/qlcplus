/*
  Q Light Controller Plus
  nanodmx.cpp

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

#include "nanodmx.h"

#include <QDebug>
#include <QDir>

NanoDMX::NanoDMX(const QString& serial, const QString& name,
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

NanoDMX::~NanoDMX()
{
    if (m_file.isOpen() == true)
        m_file.close();
}

DMXUSBWidget::Type NanoDMX::type() const
{
    return DMXUSBWidget::DMX4ALL;
}

bool NanoDMX::checkReply()
{
    QByteArray reply = m_file.readAll();
    //qDebug() << Q_FUNC_INFO << "Reply: " << QString::number(reply[0], 16);
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

bool NanoDMX::sendChannelValue(int channel, uchar value)
{
    QByteArray chanMsg;
    QString msg;
    chanMsg.append(msg.sprintf("C%03dL%03d", channel, value));
    return ftdi()->write(chanMsg);
}

QString NanoDMX::getDeviceName()
{
    if (m_device == NULL)
        return QString();

    QDir sysfsDevDir("/sys/bus/usb/devices");
    QStringList devDirs = sysfsDevDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 1- scan all the devices in the device bus
    foreach (QString dir, devDirs)
    {
#ifdef LIBFTDI1
        if (dir.startsWith(QString::number(libusb_get_port_number(m_device))) &&
#else
        if (dir.startsWith(QString::number(m_device->bus->location)) &&
#endif
            dir.contains(".") &&
            dir.contains(":") == false)
        {
            // 2- Match the product name
            //qDebug() << "SYSFS Directory:" << dir;
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
                                    qDebug() << "This NanoDMX adapter will use" << QString("/dev/" + ttyName);
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
bool NanoDMX::open(quint32 line, bool input)
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
        qWarning() << "NanoDMX output cannot be opened:"
                   << m_file.errorString();
        return false;
    }

    QByteArray initSequence;

    /* Check connection */
    initSequence.append("C?");
    if (m_file.write(initSequence) == true)
    {
        if (checkReply() == false)
            return false;
    }
    else
        qWarning() << Q_FUNC_INFO << name() << "Initialization failed";

    /* set the DMX OUT channels number */
    initSequence.clear();
    initSequence.append("N511");
    if (m_file.write(initSequence) == true)
    {
        if (checkReply() == false)
            return false;
    }

    return true;
}

bool NanoDMX::close(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

    if (m_file.isOpen() == true)
        m_file.close();

    return true;
}

QString NanoDMX::uniqueName(ushort line, bool input) const
{
    Q_UNUSED(line)
    Q_UNUSED(input)
    return QString("%1").arg(name());
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString NanoDMX::additionalInfo() const
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

bool NanoDMX::writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)

    if (m_file.isOpen() == false)
        return false;

    /* Since the DMX4ALL array transfer protocol can handle bulk transfer of
     * a maximum of 256 channels, I need to split a 512 universe into 2 */

    QByteArray arrayTransfer(data);
    arrayTransfer.prepend(char(0xFF));
    arrayTransfer.prepend(char(0x00));        // Start channel low byte
    arrayTransfer.prepend(char(0x00));        // Start channel high byte
    if (data.size() < 256)
    {
        arrayTransfer.prepend(data.size());   // Number of channels
    }
    else
    {
        arrayTransfer.prepend(char(0xFF));   // First 256 channels
        arrayTransfer.insert(259, char(0xFF));
        arrayTransfer.insert(260, char(0x00));
        arrayTransfer.insert(261, char(0x01));
    }

    if (m_file.write(arrayTransfer) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
        return false;
    }
    else
    {
        return true;
    }
}


