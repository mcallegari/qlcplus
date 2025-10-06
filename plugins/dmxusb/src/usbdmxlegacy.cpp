/*
  Q Light Controller Plus
  usbdmxlegacy.cpp

  Adds support for the "USB/DMX Interface" (usbdmx.com) custom protocol.

  Protocol reference (V1.4):
   - TX ON       : 0x44
   - TX OFF      : 0x46
   - SET VALUE   : 0x48 | 0x49 (high address bit), <A7-A0>, <VAL>
   - SET LAST TX : 0x4E | 0x4F (high address bit), <A7-A0>  (response 0xCE)
   - Start code defaults to 0; we don't change it here.

  Addressing in the spec is 0-based (0x000 -> channel 1). In QLC+ data
  buffer the first slot is channel 1 at index 0, so we can pass the index
  as-is.
*/

#include "usbdmxlegacy.h"

#define USBDMXLEGACY_DEFAULT_FREQUENCY                 40
#define USBDMXLEGACY_CMD_TX_ON                         0x44
#define USBDMXLEGACY_CMD_TX_OFF                        0x46
#define USBDMXLEGACY_CMD_SET_VALUE                     0x48
#define USBDMXLEGACY_CMD_SET_VALUE_HIGH_ADDRESS_BIT    0x49
#define USBDMXLEGACY_CMD_SET_LAST_TX                   0x4E
#define USBDMXLEGACY_CMD_SET_LAST_TX_HIGH_ADDRESS_BIT  0x4F


UsbdmxLegacy::UsbdmxLegacy(DMXInterface *interface, quint32 outputLine)
    : DMXUSBWidget(interface, outputLine, USBDMXLEGACY_DEFAULT_FREQUENCY)
{
    // configure one output port
    QList<int> ports;
    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output);
    setPortsMapping(ports);
}

UsbdmxLegacy::~UsbdmxLegacy()
{
    close();
}

DMXUSBWidget::Type UsbdmxLegacy::type() const
{
    return DMXUSBWidget::USBDMXLegacy;
}

/*************************************************************************
 * Open & Close
 *************************************************************************/

bool UsbdmxLegacy::open(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

    if (DMXUSBWidget::open() == false)
        return false;

    // Ensure TX is off, set last channel, then enable TX.
    cmdTxOff();
    cmdSetLastChannel(DMX_CHANNELS - 1);
    return cmdTxOn();
}

bool UsbdmxLegacy::close(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

    cmdTxOff();
    return DMXUSBWidget::close();
}

/*************************************************************************
 * Outputs
 *************************************************************************/

bool UsbdmxLegacy::writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)

    if (!isOpen())
        return false;

    // Initialize local cache once.
    if (m_portsInfo[0].m_universeData.size() == 0)
    {
        m_portsInfo[0].m_universeData = QByteArray(DMX_CHANNELS, 0);

        // Make sure the device does not waste time sending beyond our bounds
        cmdSetLastChannel(DMX_CHANNELS - 1);
        cmdTxOn();
    }

    if (!dataChanged)
        return true;

    bool result = true;
    // Update cache and send only diffs as SET VALUE commands, as per spec.
    for (int i = 0; i < qMin(DMX_CHANNELS, data.size()); i++)
    {
        const uchar newVal = static_cast<uchar>(data.at(i));
        uchar &oldVal = reinterpret_cast<uchar&>(m_portsInfo[0].m_universeData[i]);

        if (newVal == oldVal)
            continue;

        if (!cmdSetChannelValue(i, newVal))
        {
            result = false;
            continue;
        }

        oldVal = newVal;
    }

    return result;
}

/*************************************************************************
 * Info
 *************************************************************************/

QString UsbdmxLegacy::additionalInfo() const
{
    QString info;
    info += QString("<P>");
    info += QString("<B>%1:</B> usbdmx.com (legacy) (%3)").arg(QObject::tr("Protocol"))
                                                          .arg(QObject::tr("Output"));
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Serial number"))
                                    .arg(serial());
    info += QString("</P>");
    return info;
}

/*************************************************************************
 * Command helpers
 *************************************************************************/

bool UsbdmxLegacy::cmdTxOn()
{
    QByteArray msg;
    msg.append(char(USBDMXLEGACY_CMD_TX_ON));
    return iface()->write(msg);
}

bool UsbdmxLegacy::cmdTxOff()
{
    QByteArray msg;
    msg.append(char(USBDMXLEGACY_CMD_TX_OFF));
    return iface()->write(msg);
}

bool UsbdmxLegacy::cmdSetLastChannel(int lastIndex)
{
    if (lastIndex < 0)
        lastIndex = 0;

    if (lastIndex > 511)
        lastIndex = 511;

    QByteArray msg;
    msg.append(char(hiBit(lastIndex) ? USBDMXLEGACY_CMD_SET_LAST_TX_HIGH_ADDRESS_BIT : USBDMXLEGACY_CMD_SET_LAST_TX));
    msg.append(char(lo(lastIndex)));
    return iface()->write(msg);
}

bool UsbdmxLegacy::cmdSetChannelValue(int index, uchar value)
{
    if (index < 0 || index > 511)
        return false;

    QByteArray msg;
    msg.append(char(hiBit(index) ? USBDMXLEGACY_CMD_SET_VALUE_HIGH_ADDRESS_BIT : USBDMXLEGACY_CMD_SET_VALUE));
    msg.append(char(lo(index)));
    msg.append(char(value));
    return iface()->write(msg);
}
