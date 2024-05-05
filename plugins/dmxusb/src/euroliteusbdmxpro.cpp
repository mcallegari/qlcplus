/*
  Q Light Controller Plus
  euroliteusbdmxpro.cpp

  Copyright (C) Karri Kaksonen based on nanodmx.cpp by Massimo Callegari

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

EuroliteUSBDMXPro::EuroliteUSBDMXPro(DMXInterface *iface, quint32 outputLine)
    : DMXUSBWidget(iface, outputLine, DEFAULT_OUTPUT_FREQUENCY)
    , m_running(false)
{
}

EuroliteUSBDMXPro::~EuroliteUSBDMXPro()
{
    stop();

#ifdef QTSERIAL
    if (isOpen())
        DMXUSBWidget::close();
#else
    if (m_file.isOpen() == true)
        m_file.close();
#endif
}

DMXUSBWidget::Type EuroliteUSBDMXPro::type() const
{
    return DMXUSBWidget::Eurolite;
}

#ifndef QTSERIAL
QString EuroliteUSBDMXPro::getDeviceName()
{
    QDir sysfsDevDir("/sys/bus/usb/devices");
    QStringList devDirs = sysfsDevDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 1- scan all the devices in the device bus
    foreach (QString dir, devDirs)
    {

        if (dir.startsWith(QString::number(iface()->busLocation())) &&
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
                                foreach (QString ttyName, ttyList)
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
#endif

/****************************************************************************
 * Open & Close
 ****************************************************************************/
bool EuroliteUSBDMXPro::open(quint32 line, bool input)
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
        qWarning() << "EuroliteUSBDMXPro output cannot be opened:"
                   << m_file.errorString();
        return false;
    }
#endif

    // start the output thread
    start();

    return true;
}

bool EuroliteUSBDMXPro::close(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

    stop();

#ifdef QTSERIAL
    if (isOpen())
        return DMXUSBWidget::close();
#else
    if (m_file.isOpen() == true)
        m_file.close();
#endif

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
                                         .arg("Eurolite DMX-USB Pro")
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

bool EuroliteUSBDMXPro::writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged)
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

    if (m_outputLines[0].m_universeData.size() == 0)
    {
        m_outputLines[0].m_universeData.append(data);
        m_outputLines[0].m_universeData.append(DMX_CHANNELS - data.size(), 0);
    }

    if (dataChanged)
        m_outputLines[0].m_universeData.replace(0, data.size(), data);

    return true;
}

void EuroliteUSBDMXPro::stop()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

void EuroliteUSBDMXPro::run()
{
    qDebug() << "OUTPUT thread started";
    QElapsedTimer timer;
    QByteArray request;

    m_running = true;
    while (m_running == true)
    {
        timer.restart();

        int dataLen = m_outputLines[0].m_universeData.length();
        if (dataLen == 0)
            goto framesleep;

        request.clear();
        request.append(EUROLITE_USB_DMX_PRO_START_OF_MSG); // Start byte
        request.append(EUROLITE_USB_DMX_PRO_SEND_DMX_RQ); // Send request
        request.append((dataLen + 1) & 0xff); // Data length LSB
        request.append(((dataLen + 1) >> 8) & 0xff); // Data length MSB
        request.append(char(EUROLITE_USB_DMX_PRO_DMX_ZERO)); // DMX start code (Which constitutes the + 1 below)
        request.append(m_outputLines[0].m_universeData);
        request.append(EUROLITE_USB_DMX_PRO_END_OF_MSG); // Stop byte

#ifdef QTSERIAL
        if (iface()->write(request) == false)
#else
        if (m_file.write(request) == false)
#endif
        {
            qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
#ifdef QTSERIAL
            iface()->purgeBuffers();
#endif
        }
framesleep:
        int timetoSleep = m_frameTimeUs - (timer.nsecsElapsed() / 1000);
        if (timetoSleep < 0)
            qWarning() << "DMX output is running late !";
        else
            usleep(timetoSleep);
    }

    qDebug() << "OUTPUT thread terminated";
}


