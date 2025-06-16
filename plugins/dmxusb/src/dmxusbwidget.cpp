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
#include <QSettings>
#include <QDebug>
#include <cmath>

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

#if defined(WIN32) || defined(Q_OS_WIN)
#include <Windows.h>
#define DMXUSB_WINDOWSTIMERRESOLUTION "dmxusb/windowstimerresolution"
#endif

#if defined(WIN32) || defined(Q_OS_WIN)
uint DMXUSBWidget::s_windowsTimerResolution = 1; // Default to 1 millisecond.
#endif

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

    QList<int> ports;
    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output);
    setPortsMapping(ports);

#if defined(WIN32) || defined(Q_OS_WIN)
    QSettings settings;
    QVariant var = settings.value(DMXUSB_WINDOWSTIMERRESOLUTION);
    if (var.isValid())
        s_windowsTimerResolution = var.toUInt();
    
    setWindowsTimerResolution(s_windowsTimerResolution);
#endif
}

DMXUSBWidget::~DMXUSBWidget()
{
    delete m_interface;

#if defined(WIN32) || defined(Q_OS_WIN)
    clearWindowsTimerResolution(s_windowsTimerResolution);
#endif
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

                    // 2 DMX outputs, 1 DMX input, 1 MIDI input, 1 MIDI output
                    QList<int> ports;
                    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output | DMXUSBWidget::Input);
                    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output);
                    ports << (DMXUSBWidget::MIDI | DMXUSBWidget::Input);
                    ports << (DMXUSBWidget::MIDI | DMXUSBWidget::Output);
                    promkii->setPortsMapping(ports);

                    output_id += 3;
                    input_id += 2;
                    widgetList << promkii;
                }
                break;
                case DMXUSBWidget::UltraPro:
                {
                    EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(iface, output_id, input_id++);
                    QList<int> ports;
                    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output | DMXUSBWidget::Input);
                    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output);
                    ultra->setPortsMapping(ports);

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

            // 2 DMX outputs, 1 DMX input, 1 MIDI input, 1 MIDI output
            QList<int> ports;
            ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output | DMXUSBWidget::Input);
            ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output);
            ports << (DMXUSBWidget::MIDI | DMXUSBWidget::Input);
            ports << (DMXUSBWidget::MIDI | DMXUSBWidget::Output);
            promkii->setPortsMapping(ports);

            output_id += 3;
            input_id += 2;
            widgetList << promkii;
        }
        else if (iface->vendorID() == DMXInterface::NXPVID && iface->productID() == DMXInterface::DMXKINGMAXPID)
        {
            int ESTAID = 0, DEVID = 0;
            QByteArray portsConfig;
            QString manName, devName;
            bool isDmxKing = EnttecDMXUSBPro::detectDMXKingDevice(iface, manName, devName, ESTAID, DEVID, portsConfig);

            if (isDmxKing)
            {
                qDebug() << "Number of ports detected:" << portsConfig.length();
                QList<int> ports;

                for (int p = 0; p < portsConfig.length(); p++)
                {
                    if (portsConfig[p] & EnttecDMXUSBPro::Input)
                        ports << (DMXUSBWidget::DMX | DMXUSBWidget::Input);
                    if (portsConfig[p] & EnttecDMXUSBPro::Output)
                        ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output);
                }

                EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(iface, output_id, input_id);
                ultra->setPortsMapping(ports);
                ultra->setDMXKingMode();
                ultra->setRealName(devName);
                output_id += ultra->portFlagsCount(DMXUSBWidget::Output);
                input_id += ultra->portFlagsCount(DMXUSBWidget::Input);
                widgetList << ultra;
            }
        }
        else if (productName.contains("DMX USB PRO") || productName.contains("ULTRADMX"))
        {
            int ESTAID = 0, DEVID = 0;
            QByteArray dummy;
            QString manName, devName;
            bool isDmxKing = EnttecDMXUSBPro::detectDMXKingDevice(iface, manName, devName, ESTAID, DEVID, dummy);

            if (isDmxKing)
            {
                if (DEVID == ULTRADMX_PRO_DEV_ID)
                {
                    EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(iface, output_id, input_id++);
                    QList<int> ports;
                    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output | DMXUSBWidget::Input);
                    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output);
                    ultra->setPortsMapping(ports);
                    ultra->setDMXKingMode();
                    ultra->setRealName(devName);
                    output_id += 2;
                    widgetList << ultra;
                }
                else
                {
                    EnttecDMXUSBPro *pro = new EnttecDMXUSBPro(iface, output_id++);
                    QList<int> ports;
                    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output);
                    pro->setPortsMapping(ports);
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
            QList<int> ports;
            ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output);
            pro->setPortsMapping(ports);
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

#if defined(WIN32) || defined(Q_OS_WIN)
bool DMXUSBWidget::setWindowsTimerResolution(uint resolution)
{
    TIMECAPS ptc;
    MMRESULT result;
    
    /* Find out the minimum and maximum possible timer resolution, in milliseconds */
    result = timeGetDevCaps(&ptc, sizeof(TIMECAPS));
    if (result != TIMERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "timeGetDevCaps() returned with error" << result;
        return false;
    }
    
    qDebug() << Q_FUNC_INFO << "timeGetDevCaps(): wPeriodMin =" << ptc.wPeriodMin << "wPeriodMax =" << ptc.wPeriodMax;
    
    /* Is given resolution within allowed range? */
    if (resolution < ptc.wPeriodMin || resolution > ptc.wPeriodMax)
    {
        qWarning() << Q_FUNC_INFO << "Period of" << resolution << "ms out of range";
        return false;
    }
    
    /* Request system timer resolution of the given number of milliseconds */
    result = timeBeginPeriod(resolution);
    if (result != TIMERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "timeBeginPeriod() returned with error" << result;
        return false;
    }
    
    qDebug() << Q_FUNC_INFO << "timeBeginPeriod() of" << resolution << "ms";
    
    return true;
}

bool DMXUSBWidget::clearWindowsTimerResolution(uint resolution)
{
    MMRESULT result;
    
    result = timeEndPeriod(resolution);
    if (result != TIMERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "timeEndPeriod() returned with error" << result;
        return false;
    }
    
    qDebug() << Q_FUNC_INFO << "timeEndPeriod() of" << resolution << "ms";
    
    return true;
}
#endif

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool DMXUSBWidget::open(quint32 line, bool input)
{
    int type = input ? DMXUSBWidget::Input : DMXUSBWidget::Output;
    quint32 portIdx = lineToPortIndex(line, type);

    qDebug() << "LINE TO OPEN" << line << "port number:" << portIdx;

    if (portIdx == UINT_MAX)
    {
        qWarning() << "Trying to open an out of bounds line!" << portIdx << m_portsInfo.count();
        return false;
    }

    m_portsInfo[portIdx].m_openDirection = type;

    qDebug() << Q_FUNC_INFO << "Line:" << line << ", open ports:" << openPortsCount();

    if (isOpen() == true)
        return true;

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
    int type = input ? DMXUSBWidget::Input : DMXUSBWidget::Output;
    quint32 portIdx = lineToPortIndex(line, type);

    if (portIdx == UINT_MAX)
    {
        qWarning() << "Trying to open an out of bounds line!" << portIdx << m_portsInfo.count();
        return false;
    }

    m_portsInfo[portIdx].m_openDirection = DMXUSBWidget::None;

    qDebug() << Q_FUNC_INFO << "Line:" << line << ", open ports:" << openPortsCount();

    if (openPortsCount() == 0)
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

void DMXUSBWidget::setPortsMapping(QList<int> ports)
{
    m_portsInfo.clear();

    for (int i = 0; i < ports.count(); i++)
    {
        DMXUSBLineInfo port;
        port.m_portFlags = ports.at(i);
        port.m_openDirection = DMXUSBWidget::None;
        m_portsInfo.append(port);
    }
}

int DMXUSBWidget::portFlagsCount(LineFlags flags)
{
    int count = 0;
    for (int i = 0; i < m_portsInfo.count(); i++)
    {
        if (m_portsInfo.at(i).m_portFlags & flags)
            count++;
    }
    return count;
}

int DMXUSBWidget::openPortsCount()
{
    int count = 0;
    for (int i = 0; i < m_portsInfo.count(); i++)
    {
        if (m_portsInfo.at(i).m_openDirection != DMXUSBWidget::None)
            count++;
    }
    return count;
}

quint32 DMXUSBWidget::lineToPortIndex(quint32 line, int type)
{
    quint32 portIdx = 0;
    quint32 baseLine = type == DMXUSBWidget::Output ? m_outputBaseLine : m_inputBaseLine;

    for (int i = 0; i < m_portsInfo.count(); i++)
    {
        if (m_portsInfo[i].m_portFlags & type)
        {
            if (portIdx == line - baseLine)
                return i;
            portIdx++;
        }
    }

    return UINT_MAX;
}

/********************************************************************
 * Outputs
 ********************************************************************/

int DMXUSBWidget::outputsNumber()
{
    return portFlagsCount(DMXUSBWidget::Output);
}

QStringList DMXUSBWidget::outputNames()
{
    QStringList names;
    for (ushort i = 0; i < m_portsInfo.count(); i++)
    {
        if (m_portsInfo.at(i).m_portFlags & DMXUSBWidget::Output)
            names << uniqueName(i, false);
    }
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

int DMXUSBWidget::inputsNumber()
{
    return portFlagsCount(DMXUSBWidget::Input);
}

QStringList DMXUSBWidget::inputNames()
{
    QStringList names;
    for (ushort i = 0; i < m_portsInfo.count(); i++)
    {
        if (m_portsInfo.at(i).m_portFlags & DMXUSBWidget::Input)
            names << uniqueName(i, true);
    }
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
