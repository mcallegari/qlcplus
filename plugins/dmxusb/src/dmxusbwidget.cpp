/*
  Q Light Controller Plus
  dmxusbwidget.cpp

  Copyright (C) Heikki Junnila
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

#include <QStringList>
#include <QDebug>

#include "dmxusbwidget.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbopen.h"
#if defined(Q_WS_X11) || defined(Q_OS_LINUX) || defined(Q_OS_OSX)
  #include "nanodmx.h"
  #include "euroliteusbdmxpro.h"
#endif
#include "stageprofi.h"
#include "vinceusbdmx512.h"

DMXUSBWidget::DMXUSBWidget(DMXInterface *interface, quint32 outputLine)
{
    Q_ASSERT(interface != NULL);

    m_outputBaseLine = outputLine;
    m_inputBaseLine = 0;

    m_inputOpenMask = 0;
    m_outputOpenMask = 0;

    setOutputsNumber(1);
    setInputsNumber(0);

    m_interface = interface;
}

DMXUSBWidget::~DMXUSBWidget()
{
    delete m_interface;
}

DMXInterface *DMXUSBWidget::interface() const
{
    return m_interface;
}

QString DMXUSBWidget::interfaceTypeString() const
{
    if (m_interface == NULL)
        return QString();

    return m_interface->typeString();
}

QList<DMXUSBWidget *> DMXUSBWidget::widgets()
{
    QList<DMXUSBWidget *> widgetList;
    QList<DMXInterface *> interfacesList;
    quint32 input_id = 0;
    quint32 output_id = 0;

#if defined(FTD2XX)
    interfacesList.append(FTD2XXInterface::interfaces(interfacesList));
#endif
#if defined(QTSERIAL)
    interfacesList.append(QtSerialInterface::interfaces(interfacesList));
#endif
#if defined(LIBFTDI) || defined(LIBFTDI1)
    interfacesList.append(LibFTDIInterface::interfaces(interfacesList));
#endif

    QMap <QString,QVariant> types(DMXInterface::typeMap());

    foreach (DMXInterface *iface, interfacesList)
    {
        if (types.contains(iface->serial()) == true)
        {
            // Force a widget with a specific serial to either type
            DMXUSBWidget::Type type = (DMXUSBWidget::Type) types[iface->serial()].toInt();
            switch (type)
            {
                case DMXUSBWidget::OpenTX:
                    widgetList << new EnttecDMXUSBOpen(iface, output_id++);
                break;
                case DMXUSBWidget::ProMk2:
                {
                    EnttecDMXUSBPro *promkii = new EnttecDMXUSBPro(iface, output_id, input_id);
                    promkii->setOutputsNumber(2);
                    promkii->setMidiPortsNumber(1, 1);
                    output_id += 3;
                    input_id += 2;
                    widgetList << promkii;
                }
                break;
                case DMXUSBWidget::UltraPro:
                {
                    EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(iface, output_id, input_id++);
                    ultra->setOutputsNumber(2);
                    ultra->setDMXKingMode();
                    output_id += 2;
                    widgetList << ultra;
                }
                break;
                case DMXUSBWidget::DMX4ALL:
                    widgetList << new Stageprofi(iface, output_id++);
                break;
                case DMXUSBWidget::VinceTX:
                    widgetList << new VinceUSBDMX512(iface, output_id++);
                break;
#if defined(Q_WS_X11) || defined(Q_OS_LINUX) || defined(Q_OS_OSX)
                case DMXUSBWidget::Eurolite:
                    widgetList << new EuroliteUSBDMXPro(iface, output_id++);
                break;
#endif
                default:
                case DMXUSBWidget::ProRXTX:
                    widgetList << new EnttecDMXUSBPro(iface, output_id++, input_id++);
                break;
            }
        }
        else if (iface->name().toUpper().contains("PRO MK2") == true)
        {
            EnttecDMXUSBPro *promkii = new EnttecDMXUSBPro(iface, output_id, input_id);
            promkii->setOutputsNumber(2);
            promkii->setMidiPortsNumber(1, 1);
            output_id += 3;
            input_id += 2;
            widgetList << promkii;
        }
        else if (iface->name().toUpper().contains("DMX USB PRO"))
        {
            /** Check if the device responds to label 77 and 78, so it might be a DMXking adapter */
            int ESTAID = 0;
            int DEVID = 0;
            QString manName = iface->readLabel(DMXKING_USB_DEVICE_MANUFACTURER, &ESTAID);
            qDebug() << "--------> Device Manufacturer: " << manName;
            QString devName = iface->readLabel(DMXKING_USB_DEVICE_NAME, &DEVID);
            qDebug() << "--------> Device Name: " << devName;
            qDebug() << "--------> ESTA Code: " << QString::number(ESTAID, 16) << ", Device ID: " << QString::number(DEVID, 16);
            if (ESTAID == DMXKING_ESTA_ID)
            {
                if (DEVID == ULTRADMX_PRO_DEV_ID)
                {
                    EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(iface, output_id, input_id++);
                    ultra->setOutputsNumber(2);
                    ultra->setDMXKingMode();
                    ultra->setRealName(devName);
                    output_id += 2;
                    widgetList << ultra;
                }
                else
                {
                    EnttecDMXUSBPro *pro = new EnttecDMXUSBPro(iface, output_id++);
                    pro->setInputsNumber(0);
                    pro->setRealName(devName);
                    widgetList << pro;
                }
            }
            else
            {
                /* This is probably a Enttec DMX USB Pro widget */
                EnttecDMXUSBPro *pro = new EnttecDMXUSBPro(iface, output_id++, input_id++);
                pro->setRealName(devName);
                widgetList << pro;
            }
        }
        else if (iface->name().toUpper().contains("USB-DMX512 CONVERTER") == true)
        {
            widgetList << new VinceUSBDMX512(iface, output_id++);
        }
        else if (iface->vendorID() == DMXInterface::FTDIVID &&
                 iface->productID() == DMXInterface::DMX4ALLPID)
        {
            widgetList << new Stageprofi(iface, output_id++);
        }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX) || defined(Q_OS_OSX)
        else if (iface->vendorID() == DMXInterface::ATMELVID &&
                 iface->productID() == DMXInterface::NANODMXPID)
        {
            widgetList << new NanoDMX(iface, output_id++);
        }
        else if (iface->vendorID() == DMXInterface::MICROCHIPVID &&
                 iface->productID() == DMXInterface::EUROLITEPID)
        {
            widgetList << new EuroliteUSBDMXPro(iface, output_id++);
        }
#endif
        else
        {
            /* This is probably an Open DMX USB widget */
            widgetList << new EnttecDMXUSBOpen(iface, output_id++);
        }
    }

    return widgetList;
}

bool DMXUSBWidget::forceInterfaceDriver(DMXInterface::Type type)
{
    DMXInterface *forcedIface = NULL;

    qDebug() << "[DMXUSBWidget] forcing widget" << m_interface->name() << "to type:" << type;

#if defined(FTD2XX)
    if (type == DMXInterface::FTD2xx)
        forcedIface = new FTD2XXInterface(m_interface->serial(), m_interface->name(), m_interface->vendor(),
                                          m_interface->vendorID(), m_interface->productID(), m_interface->id());
#endif
#if defined(QTSERIAL)
    if (type == DMXInterface::QtSerial)
        forcedIface = new QtSerialInterface(m_interface->serial(), m_interface->name(), m_interface->vendor(),
                                          m_interface->vendorID(), m_interface->productID(), m_interface->id());
#endif
#if defined(LIBFTDI) || defined(LIBFTDI1)
    if (type == DMXInterface::libFTDI)
        forcedIface = new LibFTDIInterface(m_interface->serial(), m_interface->name(), m_interface->vendor(),
                                          m_interface->vendorID(), m_interface->productID(), m_interface->id());
#endif

    if (forcedIface != NULL)
    {
        delete m_interface;
        m_interface = forcedIface;
        return true;
    }

    return false;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool DMXUSBWidget::open(quint32 line, bool input)
{
    if (input == true && m_inputsMap.contains(line))
    {
        quint32 devLine = m_inputsMap[line];
        m_inputOpenMask |= (1 << devLine);
    }
    else if (input == false && m_outputsMap.contains(line))
    {
        quint32 devLine = m_outputsMap[line];
        m_outputOpenMask |= (1 << devLine);
    }
    else
    {
        qWarning() << "[DMXUSBWidget] Line" << line << "doesn't belong to any mapped inputs nor to outputs !";
        return false;
    }

    qDebug() << Q_FUNC_INFO << "Line:" << line << ", Input mask:" << m_inputOpenMask << ", Output Mask:" << m_outputOpenMask;

    if (isOpen() == true)
        return true; //close();

    if (this->type() == DMXUSBWidget::DMX4ALL)
    {
        if (m_interface->openByPID(DMXInterface::DMX4ALLPID) == false)
            return close();
    }
    else
    {
        if (m_interface->open() == false)
            return close(line);
    }

    if (m_interface->reset() == false)
        return close(line);

    if (m_interface->setBaudRate() == false)
        return close(line);

    if (m_interface->setLineProperties() == false)
        return close(line);

    if (m_interface->setFlowControl() == false)
        return close(line);

    if (m_interface->purgeBuffers() == false)
        return close(line);

    qDebug() << Q_FUNC_INFO << "Interface correctly opened and configured";

    return true;
}

bool DMXUSBWidget::close(quint32 line, bool input)
{
    if (input == true && m_inputsMap.contains(line))
    {
        quint32 devLine = m_inputsMap[line];
        m_inputOpenMask &= ~(1 << devLine);
    }
    else if (input == false && m_outputsMap.contains(line))
    {
        quint32 devLine = m_outputsMap[line];
        m_outputOpenMask &= ~(1 << devLine);
    }
    else
    {
        qWarning() << "Line" << line << "doesn't belong to any mapped inputs nor to outputs !";
        return false;
    }

    qDebug() << Q_FUNC_INFO << "Line:" << line << ", Input mask:" << m_inputOpenMask << ", Output Mask:" << m_outputOpenMask;

    if (m_inputOpenMask == 0 && m_outputOpenMask == 0)
    {
        qDebug() << Q_FUNC_INFO << "All inputs/outputs have been closed. Close FTDI too.";
        if (m_interface->isOpen())
            return m_interface->close();
        else
            return true;
    }

    return true;
}

bool DMXUSBWidget::isOpen()
{
    return m_interface->isOpen();
}

/********************************************************************
 * Outputs
 ********************************************************************/

void DMXUSBWidget::setOutputsNumber(int num)
{
    m_outputsNumber = num;
    m_outputsMap.clear();
    for (ushort i = 0; i < num; i++)
        m_outputsMap[m_outputBaseLine + i] = i;
    qDebug() << "[setOutputsNumber] base line:" << m_outputBaseLine << "outputMap:" << m_outputsMap;
}

int DMXUSBWidget::outputsNumber()
{
    return m_outputsNumber;
}

QStringList DMXUSBWidget::outputNames()
{
    QStringList names;
    for (ushort i = 0; i < m_outputsNumber; i++)
        names << uniqueName(i, false);
    return names;
}

/********************************************************************
 * Inputs
 ********************************************************************/

void DMXUSBWidget::setInputsNumber(int num)
{
    m_inputsNumber = num;
    m_inputsMap.clear();
    for (ushort i = 0; i < num; i++)
        m_inputsMap[m_inputBaseLine + i] = i;
}

int DMXUSBWidget::inputsNumber()
{
    return m_inputsNumber;
}

QStringList DMXUSBWidget::inputNames()
{
    QStringList names;
    for (ushort i = 0; i < m_inputsNumber; i++)
        names << uniqueName(i, true);
    return names;
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString DMXUSBWidget::name() const
{
    return m_interface->name();
}

QString DMXUSBWidget::serial() const
{
    return m_interface->serial();
}

QString DMXUSBWidget::uniqueName(ushort line, bool input) const
{
    Q_UNUSED(line)
    Q_UNUSED(input)
    return QString("%1 (S/N: %2)").arg(name()).arg(serial());
}

void DMXUSBWidget::setRealName(QString devName)
{
    m_realName = devName;
}

QString DMXUSBWidget::realName() const
{
    return m_realName;
}

QString DMXUSBWidget::vendor() const
{
    return m_interface->vendor();
}

/****************************************************************************
 * Write universe
 ****************************************************************************/

bool DMXUSBWidget::writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    Q_UNUSED(data)

    return false;
}
