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

NanoDMX::NanoDMX(DMXInterface *interface, quint32 outputLine)
    : DMXUSBWidget(interface, outputLine)
{
}

NanoDMX::~NanoDMX()
{
#ifdef QTSERIAL
    if (isOpen())
        DMXUSBWidget::close();
#else
    if (m_file.isOpen() == true)
        m_file.close();
#endif
}

DMXUSBWidget::Type NanoDMX::type() const
{
    return DMXUSBWidget::DMX4ALL;
}

bool NanoDMX::checkReply()
{
#ifdef QTSERIAL
    bool ok = false;
    uchar res;

    res = interface()->readByte(&ok);
    if (ok == false || res != 0x47)
        return false;

    return true;
#else
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
#endif
}

bool NanoDMX::sendChannelValue(int channel, uchar value)
{
    QByteArray chanMsg;
    QString msg;
    chanMsg.append(msg.sprintf("C%03dL%03d", channel, value));
    return interface()->write(chanMsg);
}

#ifndef QTSERIAL
QString NanoDMX::getDeviceName()
{
    QDir sysfsDevDir("/sys/bus/usb/devices");
    QStringList devDirs = sysfsDevDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 1- scan all the devices in the device bus
    foreach (QString dir, devDirs)
    {
        if (dir.startsWith(QString::number(interface()->busLocation())) &&
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
#endif

/****************************************************************************
 * Open & Close
 ****************************************************************************/
bool NanoDMX::open(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

#ifdef QTSERIAL
    if (DMXUSBWidget::open() == false)
        return false;
#else
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
#endif

    m_universe.fill(0, 512);

    QByteArray initSequence;

    /* Check connection */
    initSequence.append("C?");
#ifdef QTSERIAL
    if (interface()->write(initSequence) == true)
#else
    if (m_file.write(initSequence) == true)
#endif
    {
        if (checkReply() == false)
            qWarning() << Q_FUNC_INFO << name() << "Initialization failed";
    }
    else
        qWarning() << Q_FUNC_INFO << name() << "Initialization failed";

    /* set the DMX OUT channels number */
    initSequence.clear();
    initSequence.append("N511");
#ifdef QTSERIAL
    if (interface()->write(initSequence) == true)
#else
    if (m_file.write(initSequence) == true)
#endif
    {
        if (checkReply() == false)
            qWarning() << Q_FUNC_INFO << name() << "Channels initialization failed";
    }

    return true;
}

bool NanoDMX::close(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

#ifdef QTSERIAL
    if (isOpen())
        return DMXUSBWidget::close();
#else
    if (m_file.isOpen() == true)
        m_file.close();
#endif

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

#ifdef QTSERIAL
    if (isOpen() == false)
        return false;
#else
    if (m_file.isOpen() == false)
        return false;
#endif

    //qDebug() << "Writing universe...";

    for (int i = 0; i < data.size(); i++)
    {
        if (data[i] != m_universe[i])
        {
            //qDebug() << "Writing value at index" << i;
            QByteArray fastTrans;
            if (i < 256)
            {
                fastTrans.append((char)0xE2);
                fastTrans.append((char)i);
            }
            else
            {
                fastTrans.append((char)0xE3);
                fastTrans.append((char)(i - 256));
            }
            fastTrans.append(data[i]);
#ifdef QTSERIAL
            if (interface()->write(fastTrans) == false)
#else
            if (m_file.write(fastTrans) <= 0)
#endif
            {
                qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
#ifdef QTSERIAL
                interface()->purgeBuffers();
#endif
                return false;
            }
            else
            {
                m_universe[i] = data[i];
#ifdef QTSERIAL
                if (checkReply() == false)
                    interface()->purgeBuffers();
#endif
            }
        }
    }

    return true;
}


