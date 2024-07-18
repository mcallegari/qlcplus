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
#include "dmxusbopenrx.h"
#if defined(Q_WS_X11) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
  #include "nanodmx.h"
  #include "euroliteusbdmxpro.h"
#endif
#include "stageprofi.h"
#include "vinceusbdmx512.h"

DMXUSBWidget::DMXUSBWidget(DMXInterface *iface, quint32 outputLine, int frequency)
    : m_interface(iface)
    , m_outputBaseLine(outputLine)
    , m_inputBaseLine(0)
{
    Q_ASSERT(iface != NULL);

    QMap <QString, QVariant> freqMap(DMXInterface::frequencyMap());
    if (freqMap.contains(m_interface->serial()))
        setOutputFrequency(freqMap[m_interface->serial()].toInt());
    else
        setOutputFrequency(frequency);

    setOutputsNumber(1);
    setInputsNumber(0);
}

DMXUSBWidget::~DMXUSBWidget()
{
    delete m_interface;
}

DMXInterface *DMXUSBWidget::iface() const
{
    return m_interface;
}

QString DMXUSBWidget::interfaceTypeString() const
{
    if (m_interface == NULL)
        return QString();

    return m_interface->typeString();
}

bool DMXUSBWidget::detectDMXKingDevice(DMXInterface *iface,
                                       QString &manufName, QString &deviceName,
                                       int &ESTA_ID, int &DEV_ID)
{
    if (iface->readLabel(DMXKING_USB_DEVICE_MANUFACTURER, ESTA_ID, manufName) == false)
        return false;

    qDebug() << "--------> Device Manufacturer: " << manufName;
    if (iface->readLabel(DMXKING_USB_DEVICE_NAME, DEV_ID, deviceName) == false)
        return false;

    qDebug() << "--------> Device Name: " << deviceName;
    qDebug() << "--------> ESTA Code: " << QString::number(ESTA_ID, 16) << ", Device ID: " << QString::number(DEV_ID, 16);

    if (ESTA_ID == DMXKING_ESTA_ID)
        return true;

    return false;
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

    QMap <QString, QVariant> types(DMXInterface::typeMap());

    foreach (DMXInterface *iface, interfacesList)
    {
        QString productName = iface->name().toUpper();

        // check if protocol must be forced on an interface
        if (types.contains(iface->serial()) == true)
        {
            DMXUSBWidget::Type type = (DMXUSBWidget::Type) types[iface->serial()].toInt();
            switch (type)
            {
                case DMXUSBWidget::OpenTX:
                    widgetList << new EnttecDMXUSBOpen(iface, output_id++);
                break;
                case DMXUSBWidget::OpenRX:
                    widgetList << new DMXUSBOpenRx(iface, input_id++);
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
#if defined(Q_WS_X11) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
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
        else if (productName.contains("PRO MK2") == true)
        {
            EnttecDMXUSBPro *promkii = new EnttecDMXUSBPro(iface, output_id, input_id);
            promkii->setOutputsNumber(2);
            promkii->setMidiPortsNumber(1, 1);
            output_id += 3;
            input_id += 2;
            widgetList << promkii;
        }
        else if (iface->vendorID() == DMXInterface::NXPVID && iface->productID() == DMXInterface::DMXKINGMAXPID)
        {
            int ESTAID = 0, DEVID = 0, outNumber;
            QString manName, devName;
            bool isDmxKing = detectDMXKingDevice(iface, manName, devName, ESTAID, DEVID);

            // read also ports count
            if (isDmxKing && iface->readLabel(DMXKING_DMX_PORT_COUNT, outNumber, manName))
            {
                qDebug() << "Number of outputs detected:" << outNumber;

                EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(iface, output_id, input_id++);
                ultra->setOutputsNumber(outNumber);
                ultra->setDMXKingMode();
                ultra->setRealName(devName);
                output_id += outNumber;
                widgetList << ultra;
            }
        }
        else if (productName.contains("DMX USB PRO") || productName.contains("ULTRADMX"))
        {
            int ESTAID = 0, DEVID = 0;
            QString manName, devName;
            bool isDmxKing = detectDMXKingDevice(iface, manName, devName, ESTAID, DEVID);

            if (isDmxKing)
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
        else if (productName.contains("DMXIS"))
        {
            EnttecDMXUSBPro *pro = new EnttecDMXUSBPro(iface, output_id++);
            pro->setInputsNumber(0);
            widgetList << pro;
        }
        else if (productName.contains("USB-DMX512 CONVERTER") == true)
        {
            widgetList << new VinceUSBDMX512(iface, output_id++);
        }
        else if (iface->vendorID() == DMXInterface::FTDIVID &&
                 iface->productID() == DMXInterface::DMX4ALLPID)
        {
            widgetList << new Stageprofi(iface, output_id++);
        }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
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
    if (input)
    {
        quint32 devLine = line - m_inputBaseLine;
        if (devLine >= (quint32)m_inputLines.count())
        {
            qWarning() << "Trying to open an out of bounds input line !" << devLine << m_inputLines.count();
            return false;
        }
        m_inputLines[devLine].m_isOpen = true;
    }
    else
    {
        quint32 devLine = line - m_outputBaseLine;
        if (devLine >= (quint32)m_outputLines.count())
        {
            qWarning() << "Trying to open an out of bounds output line !" << devLine << m_outputLines.count();
            return false;
        }
        m_outputLines[devLine].m_isOpen = true;
    }

    qDebug() << Q_FUNC_INFO << "Line:" << line << ", open inputs:" << openInputLines() << ", open outputs:" << openOutputLines();

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

    if (m_interface->setLineProperties() == false)
        return close(line);

    if (m_interface->setFlowControl() == false)
        return close(line);

    if (m_interface->setBaudRate() == false)
        return close(line);

    if (m_interface->purgeBuffers() == false)
        return close(line);

    qDebug() << Q_FUNC_INFO << "Interface correctly opened and configured";

    return true;
}

bool DMXUSBWidget::close(quint32 line, bool input)
{
    if (input)
    {
        quint32 devLine = line - m_inputBaseLine;
        if (devLine >= (quint32)m_inputLines.count())
        {
            qWarning() << "Trying to close an out of bounds input line !" << devLine << m_inputLines.count();
            return false;
        }
        m_inputLines[devLine].m_isOpen = false;
    }
    else
    {
        quint32 devLine = line - m_outputBaseLine;
        if (devLine >= (quint32)m_outputLines.count())
        {
            qWarning() << "Trying to close an out of bounds output line !" << devLine << m_outputLines.count();
            return false;
        }
        m_outputLines[devLine].m_isOpen = false;
    }

    qDebug() << Q_FUNC_INFO << "Line:" << line << ", open inputs:" << openInputLines() << ", open outputs:" << openOutputLines();

    if (openInputLines() == 0 && openOutputLines() == 0)
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
    m_outputLines.clear();
    m_outputLines.resize(num);
    for (ushort i = 0; i < num; i++)
    {
        m_outputLines[i].m_isOpen = false;
        m_outputLines[i].m_lineType = DMX;
    }

    qDebug() << "[setOutputsNumber] base line:" << m_outputBaseLine << "m_outputLines:" << m_outputLines.count();
}

int DMXUSBWidget::outputsNumber()
{
    return m_outputLines.count();
}

int DMXUSBWidget::openOutputLines()
{
    int count = 0;
    for (int i = 0; i < m_outputLines.count(); i++)
        if (m_outputLines[i].m_isOpen)
            count++;

    return count;
}

QStringList DMXUSBWidget::outputNames()
{
    QStringList names;
    for (ushort i = 0; i < m_outputLines.count(); i++)
        names << uniqueName(i, false);
    return names;
}

int DMXUSBWidget::outputFrequency()
{
    return m_frequency;
}

void DMXUSBWidget::setOutputFrequency(int frequency)
{
    m_frequency = frequency;
    // One "official" DMX frame can take (1s/44Hz) = 23ms
    m_frameTimeUs = int((floor((1000.0 / double(m_frequency)) + 0.5)) * 1000.0);
}

/********************************************************************
 * Inputs
 ********************************************************************/

void DMXUSBWidget::setInputsNumber(int num)
{
    m_inputLines.clear();
    m_inputLines.resize(num);
    for (ushort i = 0; i < num; i++)
    {
        m_inputLines[i].m_isOpen = false;
        m_inputLines[i].m_lineType = DMX;
    }
}

int DMXUSBWidget::inputsNumber()
{
    return m_inputLines.count();
}

QStringList DMXUSBWidget::inputNames()
{
    QStringList names;
    for (ushort i = 0; i < m_inputLines.count(); i++)
        names << uniqueName(i, true);
    return names;
}

int DMXUSBWidget::openInputLines()
{
    int count = 0;
    for (int i = 0; i < m_inputLines.count(); i++)
        if (m_inputLines[i].m_isOpen)
            count++;

    return count;
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

/********************************************************************
 * RDM
 ********************************************************************/

bool DMXUSBWidget::supportRDM()
{
    return false;
}

bool DMXUSBWidget::sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params)
{
    Q_UNUSED(universe)
    Q_UNUSED(line)
    Q_UNUSED(command)
    Q_UNUSED(params)

    return false;
}

/****************************************************************************
 * Write universe
 ****************************************************************************/

bool DMXUSBWidget::writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    Q_UNUSED(data)
    Q_UNUSED(dataChanged)

    return false;
}
