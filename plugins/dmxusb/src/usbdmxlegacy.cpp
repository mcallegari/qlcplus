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
#include "qlcmacros.h"

#define DEFAULT_USBDMX_FREQUENCY 40

UsbdmxLegacy::UsbdmxLegacy(DMXInterface *iface, quint32 outputLine, QObject *parent)
    : DMXUSBWidget(iface, outputLine, DEFAULT_USBDMX_FREQUENCY)
{
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
    return DMXUSBWidget::UsbdmxLegacy;
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

    if (isOpen() == false)
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

    // Update cache and send only diffs as SET VALUE commands, as per spec.
    const int n = qMin(DMX_CHANNELS, data.size());
    for (int i = 0; i < n; ++i)
    {
        const uchar newVal = static_cast<uchar>(data.at(i));
        uchar &oldVal = reinterpret_cast<uchar&>(m_portsInfo[0].m_universeData[i]);
        if (newVal != oldVal)
        {
            if (cmdSetChannelValue(i, newVal) == false)
                return false;
            oldVal = newVal;
        }
    }
    return true;
}

/*************************************************************************
 * Info
 *************************************************************************/

QString UsbdmxLegacy::additionalInfo() const
{
    QString info;
    info += QString("<P>");
    info += QString("<B>%1:</B> %2 (%3)").arg(QObject::tr("Protocol"))
                                         .arg("usbdmx.com (legacy)")
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
    msg.append(char(0x44)); // TX ON
    return iface()->write(msg);
}

bool UsbdmxLegacy::cmdTxOff()
{
    QByteArray msg;
    msg.append(char(0x46)); // TX OFF
    return iface()->write(msg);
}

bool UsbdmxLegacy::cmdSetLastChannel(int lastIdx)
{
    if (lastIdx < 0) lastIdx = 0;
    if (lastIdx > 511) lastIdx = 511;

    QByteArray msg;
    const bool hib = hiBit(lastIdx);
    msg.append(char(hib ? 0x4F : 0x4E));
    msg.append(char(lo(lastIdx)));
    return iface()->write(msg);
}

bool UsbdmxLegacy::cmdSetChannelValue(int idx, uchar val)
{
    if (idx < 0 || idx > 511) return false;
    QByteArray msg;
    const bool hib = hiBit(idx);
    msg.append(char(hib ? 0x49 : 0x48));   // SET VALUE with high address selector
    msg.append(char(lo(idx)));             // <A7..A0>
    msg.append(char(val));                 // <VAL>
    return iface()->write(msg);
}