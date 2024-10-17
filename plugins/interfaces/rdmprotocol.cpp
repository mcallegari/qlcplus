/*
  Q Light Controller Plus
  rdmprotocol.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing software
  distributed under the License is distributed on an "AS IS" BASIS
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QDebug>

#include "rdmprotocol.h"

RDMProtocol::RDMProtocol()
    : m_estaID(QLCPLUS_ESTA_ID)
    , m_deviceID(QLCPLUS_DEVICE_ID)
    , m_transactionNum(0x01)
{
}

void RDMProtocol::setEstaID(quint16 id)
{
    m_estaID = id;
}

void RDMProtocol::setDeviceId(quint32 id)
{
    m_deviceID = id;
}

bool RDMProtocol::packetizeCommand(ushort command, QVariantList params, bool startCode, QByteArray &buffer)
{
    buffer.clear();

    if (startCode)
        buffer.append(char(RDM_START_CODE));

    buffer.append(char(RDM_SC_SUB_MESSAGE));

    // temporary length. Fixed at the end
    buffer.append(char(0x00));

    if (params.length() == 0)
        return false;

    // destination UID
    buffer.append(QByteArray::fromHex(params.at(0).toString().toUtf8()));

    // source UID
    buffer.append(UIDToByteArray(m_estaID, m_deviceID));

    // transaction number
    buffer.append(char(m_transactionNum));

    // Port ID
    buffer.append(char(0x01));

    // message count
    buffer.append(char(0x00));

    // SUB device
    buffer.append(char(0x00));
    buffer.append(char(0x00));

    // Command
    buffer.append(char(command));

    switch (command)
    {
        case DISCOVERY_COMMAND:
        {
            if (params.length() < 2)
                return false;

            quint16 pid = params.at(1).toUInt();
            buffer.append(shortToByteArray(pid));

            // check if this is a mute/unmute command
            if (pid == PID_DISC_MUTE || pid == PID_DISC_UN_MUTE)
            {
                buffer.append(char(0x00)); // no payload
            }
            else
            {
                buffer.append(char(0x0C)); // PDL
                qulonglong start = params.at(2).toULongLong();
                qulonglong end = params.at(3).toULongLong();

                buffer.append(UIDToByteArray(start >> 32, start & 0x00000000FFFFFFFF)); // Lower bound UID
                buffer.append(UIDToByteArray(end >> 32, end & 0x00000000FFFFFFFF)); // Upper bound UID
            }
        }
        break;
        case GET_COMMAND:
        {
            if (params.length() < 2)
                return false;

            quint16 pid = params.at(1).toUInt();
            buffer.append(shortToByteArray(pid));

            if (params.length() > 3)
            {
                uchar size = params.at(2).toUInt();
                buffer.append(size); // add PDL
                switch (size)
                {
                case 1:
                    buffer.append(uchar(params.at(3).toUInt()));
                    break;
                case 2:
                    buffer.append(shortToByteArray(params.at(3).toUInt()));
                    break;
                case 4:
                    buffer.append(longToByteArray(params.at(3).toUInt()));
                    break;
                default:
                    break;
                }
            }
            else
            {
                buffer.append(char(0));
            }
        }
        break;
        case SET_COMMAND:
        {
            if (params.length() < 2)
                return false;

            quint16 pid = params.at(1).toUInt();
            buffer.append(shortToByteArray(pid));
            int pdlPosition = buffer.length();

            // Temporarily add a zero PDL.
            // Will be fixed after parameters append
            buffer.append(char(0));

            for (int i = 0; i < params.length(); i += 2)
            {
                int size = params.at(i).toInt();

                // special case for byte arrays
                if (size == 99)
                {
                    QByteArray ba = params.at(i + 1).toByteArray();
                    buffer.append(ba);
                    break;
                }

                switch (size)
                {
                    case 1:
                        buffer.append(uchar(params.at(i + 1).toUInt()));
                    break;
                    case 2:
                        buffer.append(shortToByteArray(params.at(i + 1).toUInt()));
                    break;
                    case 4:
                        buffer.append(longToByteArray(params.at(i + 1).toUInt()));
                    break;
                    default:
                    break;
                }
            }

            int pdl = buffer.length() - pdlPosition - 1;
            buffer[pdlPosition] = pdl;
        }
        break;
        default:
        break;
    }

    // set the correct length
    buffer[startCode ? 2 : 1] = buffer.length() + (startCode ? 0 : 1);

    // append checksum
    buffer.append(shortToByteArray(calculateChecksum(startCode, buffer, buffer.length())));

    m_transactionNum++;

    return true;
}

bool RDMProtocol::parseDiscoveryReply(const QByteArray &buffer, QVariantMap &values)
{
    if (buffer.length() < 24)
        return false;

    int i = 0;

    // check preamble
    if (buffer.at(i) != char(0xFE) ||
        buffer.at(i + 1) != char(0xFE) ||
        buffer.at(i + 2) != char(0xFE) ||
        buffer.at(i + 3) != char(0xFE) ||
        buffer.at(i + 4) != char(0xFE) ||
        buffer.at(i + 5) != char(0xFE) ||
        buffer.at(i + 6) != char(0xFE))
            return false;

    i += 7;
    // check separator
    if (buffer.at(i++) != char(0xAA))
        return false;

    quint8 mMSB = quint8(buffer.at(i)) & quint8(buffer.at(i + 1));
    quint8 mLSB = quint8(buffer.at(i + 2)) & quint8(buffer.at(i + 3));
    i += 4;

    quint8 dMSB3 = quint8(buffer.at(i)) & quint8(buffer.at(i + 1));
    quint8 dMSB2 = quint8(buffer.at(i + 2)) & quint8(buffer.at(i + 3));
    quint8 dMSB1 = quint8(buffer.at(i + 4)) & quint8(buffer.at(i + 5));
    quint8 dLSB  = quint8(buffer.at(i + 6)) & quint8(buffer.at(i + 7));
    i += 8;

    quint16 ESTAId;
    quint32 deviceId;
    QByteArray ba;
    ba.append(mMSB);
    ba.append(mLSB);
    ba.append(dMSB3);
    ba.append(dMSB2);
    ba.append(dMSB1);
    ba.append(dLSB);

    QString UID = byteArrayToUID(ba, ESTAId, deviceId);

    // calculate checksum
    quint8 cMSB = quint8(buffer.at(i)) & quint8(buffer.at(i + 1));
    quint8 cLSB = quint8(buffer.at(i + 2)) & quint8(buffer.at(i + 3));
    quint16 readCS = (cMSB << 8) | cLSB;
    quint16 calcCS = calculateChecksum(true, buffer.mid(8), 12);

    if (readCS != calcCS)
    {
        qDebug().nospace().noquote() << "ERROR: Read checksum 0x" << QString::number(readCS, 16)
                                     << ", calculated 0x"<< QString::number(calcCS, 16);
        return false;
    }

    qDebug() << "[RDM] Detected UID:" << UID;
    values.insert("DISCOVERY_COUNT", 1);
    values.insert("UID-0", UID);

    return true;
}

bool RDMProtocol::parsePacket(const QByteArray &buffer, QVariantMap &values)
{
    int i = 0;
    bool startCode = false;

#ifdef DEBUG_RDM
    qDebug() << "[RDM] Packet payload:" << buffer.toHex(',');
#endif

    if (buffer.length() == 0)
        return false;

    // check first bytes
    if (buffer.at(i) == char(RDM_START_CODE))
    {
        startCode = true;
        i++;
    }

    if (buffer.at(i++) != char(RDM_SC_SUB_MESSAGE))
        return false;

    // Data length
    quint8 length = quint8(buffer.at(i++));

    // Destination UID and source UID
    quint16 ESTAId;
    quint32 deviceId;
    QString destUID = byteArrayToUID(buffer.mid(i, 6), ESTAId, deviceId);
    i += 6;
    QString sourceUID = byteArrayToUID(buffer.mid(i, 6), ESTAId, deviceId);
    i += 6;

    // check if we are reading our own request
    if (ESTAId == m_estaID && deviceId == m_deviceID)
        return false;

    values.insert("UID_INFO", sourceUID);

    // transaction number
    quint8 transactionNum = quint8(buffer.at(i++));

    // Response type
    quint8 responseType = quint8(buffer.at(i++));
    values.insert("Response", responseToString(responseType));

    if (responseType != RESPONSE_TYPE_ACK)
        qWarning() << "[RDM] bad response type" << responseType;

    // message count
    quint8 messageCount = quint8(buffer.at(i++));

    // sub device
    quint16 subDevice = byteArrayToShort(buffer, i);
    i+=2;

    // command class
    quint8 commandClass = quint8(buffer.at(i++));

    // Parameter ID
    quint16 PID = byteArrayToShort(buffer, i);
    values.insert("PID", PID);
    i += 2;

    // Parameter data length
    quint8 PDL = quint8(buffer.at(i++));

#ifdef DEBUG_RDM
    qDebug().nospace().noquote() <<
        "[RDM] Data length: " << QString::number(length) <<
        ", source UID: " << sourceUID << ", destination UID: " << destUID <<
        ", transaction number: " << QString::number(transactionNum) <<
        ", Response type: " << QString::number(responseType) <<
        ", Message count: " << QString::number(messageCount) <<
        ", Sub-device: " << QString::number(subDevice) <<
        ", Command class: 0x" << QString::number(commandClass, 16) <<
        ", Parameter ID: 0x" << QString::number(PID, 16) <<
        ", Parameter data length: " << QString::number(PDL);
#else
    Q_UNUSED(length)
    Q_UNUSED(transactionNum)
    Q_UNUSED(messageCount)
    Q_UNUSED(subDevice)
    Q_UNUSED(commandClass)
#endif

    switch (PID)
    {
        case PID_SUPPORTED_PARAMETERS:
        {
            QVector<quint16> pidList;
#ifdef DEBUG_RDM
            QDebug out = qDebug();
            out.nospace().noquote() << "Supported PIDs list: ";
#endif
            for (int n = 0; n < PDL; n += 2)
            {
                quint16 pid = byteArrayToShort(buffer, i + n);
                pidList.append(pid);
#ifdef DEBUG_RDM
                out << "0x" << QString::number(pid, 16) << ", ";
#endif
            }
            values.insert("PID_LIST", QVariant::fromValue(pidList));
        }
        break;
        case PID_DEVICE_INFO:
        {
            values.insert("RDM version", byteArrayToShort(buffer, i));
            values.insert("Device model ID", byteArrayToShort(buffer, i + 2));
            values.insert("TYPE", categoryToString(byteArrayToShort(buffer, i + 4)));
            values.insert("Software version", byteArrayToLong(buffer, i + 6));
            values.insert("DMX_CHANNELS", byteArrayToShort(buffer, i + 10));
            values.insert("Current personality", quint8(buffer.at(i + 12)));
            values.insert("Number of personalities", quint8(buffer.at(i + 13)));
            values.insert("DMX_START_ADDRESS", byteArrayToShort(buffer, i + 14));
            values.insert("Sub-device count", byteArrayToShort(buffer, i + 16));
            values.insert("Number of sensors", quint8(buffer.at(i + 20)));
        }
        break;
        case PID_DEVICE_MODEL_DESCRIPTION:
        {
            values.insert("MODEL_NAME", QString(buffer.mid(i, PDL)));
        }
        break;
        case PID_MANUFACTURER_LABEL:
        {
            values.insert("MANUFACTURER", QString(buffer.mid(i, PDL)));
        }
        break;
        case PID_PARAMETER_DESCRIPTION:
        {
            if (PDL < 20)
                break;

            values.insert("PID_INFO", byteArrayToShort(buffer, i));
            values.insert("PDL Size", quint8(buffer.at(i + 2)));
            values.insert("Data type", quint8(buffer.at(i + 3)));
            values.insert("Command class", quint8(buffer.at(i + 4)));
            values.insert("Type", quint8(buffer.at(i + 5)));
            values.insert("Unit", quint8(buffer.at(i + 6)));
            values.insert("Prefix", quint8(buffer.at(i + 7)));
            values.insert("Min Valid Value", byteArrayToLong(buffer, i + 8));
            values.insert("Max Valid Value", byteArrayToLong(buffer, i + 12));
            values.insert("Default Value", byteArrayToLong(buffer, i + 16));
            values.insert("PID_DESC", QString(buffer.mid(i + 20, PDL - 20)));
        }
        break;
        case PID_DMX_PERSONALITY:
        {
            values.insert("PERS_CURRENT", quint8(buffer.at(i)));
            values.insert("PERS_COUNT", quint8(buffer.at(i + 1)));
        }
        break;
        case PID_DMX_PERSONALITY_DESCRIPTION:
        {
            values.insert("PERS_INDEX", quint8(buffer.at(i)));
            values.insert("PERS_CHANNELS", byteArrayToShort(buffer, i + 1));
            values.insert("PERS_DESC", QString(buffer.mid(i + 3, PDL - 3)));
        }
        break;
        case PID_DMX_START_ADDRESS:
        {
            values.insert("DMX_START_ADDRESS", byteArrayToShort(buffer, i));
        }
        break;
        case PID_SLOT_INFO:
        {
            QVector<quint16> slotList;

            for (int n = 0; n < PDL; n += 5)
            {
                quint16 slotId = byteArrayToShort(buffer, i + n);
                slotList.append(slotId);
                //qDebug().nospace().noquote() << "SLOT ID: " << QString::number(slotId);
            }
            values.insert("SLOT_LIST", QVariant::fromValue(slotList));
        }
        break;
        case PID_SLOT_DESCRIPTION:
        {
            values.insert("SLOT_ID", byteArrayToShort(buffer, i));
            values.insert("SLOT_DESC", QString(buffer.mid(i + 2, PDL - 2)));
        }
        break;
        default:
        break;
    }

    if (PDL)
    {
        QByteArray data = buffer.mid(i, PDL);
        QByteArray hexData = data.toHex(',');
        values.insert("RAW_DATA", hexData);
        QString dString;
        for (int c = 0; c < data.length(); c++)
        {
            if (data.at(c) < 0x20)
                dString.append("#");
            else
                dString.append(data.at(c));
        }
        values.insert("RAW_DATA_STRING", dString);
    }

    i += PDL;

    quint16 csFromData = byteArrayToShort(buffer, i);
    quint16 csElapsed = calculateChecksum(startCode, buffer, i);

    if (csFromData != csElapsed)
    {
        qDebug() << "Checksum ERROR. Got:" << QString::number(csFromData, 16) << ", calculated:" << QString::number(csElapsed, 16);
        return false;
    }

    return true;
}

QByteArray RDMProtocol::UIDToByteArray(quint16 manufacturer, quint32 deviceId)
{
    QByteArray ba;

    ba.append(char(manufacturer >> 8));
    ba.append(char(manufacturer & 0x00FF));

    ba.append(char((deviceId >> 24) & 0x00FF));
    ba.append(char((deviceId >> 16) & 0x00FF));
    ba.append(char((deviceId >> 8) & 0x00FF));
    ba.append(char(deviceId & 0x00FF));

    return ba;
}

QString RDMProtocol::byteArrayToUID(QByteArray buffer, quint16 &ESTAId, quint32 &deviceId)
{
    int i = 0;
    ESTAId = quint8(buffer.at(i)) << 8 | quint8(buffer.at(i + 1));
    i += 2;

    deviceId  = quint8(buffer.at(i++)) << 24;
    deviceId |= quint8(buffer.at(i++)) << 16;
    deviceId |= quint8(buffer.at(i++)) << 8;
    deviceId |= quint8(buffer.at(i++));

    return QString("%1%2")
            .arg(ESTAId, 4, 16, QLatin1Char('0'))
            .arg(deviceId, 8, 16, QLatin1Char('0')).toUpper();
}

QString RDMProtocol::broadcastAddress()
{
    return QString("%1%2")
            .arg(BROADCAST_ESTA_ID, 4, 16)
            .arg(BROADCAST_DEVICE_ID, 6, 16);
}

QByteArray RDMProtocol::shortToByteArray(quint16 data)
{
    QByteArray ba;

    ba.append(char(data >> 8));
    ba.append(char(data & 0x00FF));

    return ba;
}

QByteArray RDMProtocol::longToByteArray(quint32 data)
{
    QByteArray ba;

    ba.append(char((data >> 24) & 0x00FF));
    ba.append(char((data >> 16) & 0x00FF));
    ba.append(char((data >> 8) & 0x00FF));
    ba.append(char(data & 0x00FF));


    return ba;
}

quint16 RDMProtocol::byteArrayToShort(const QByteArray &buffer, int index)
{
    if (buffer.length() < index + 2)
        return 0;

    quint16 value = quint8(buffer.at(index)) << 8 |
                    quint8(buffer.at(index + 1));
    return value;
}

quint32 RDMProtocol::byteArrayToLong(const QByteArray &buffer, int index)
{
    if (buffer.length() < index + 4)
        return 0;

    quint32 value = quint8(buffer.at(index)) << 24 |
                    quint8(buffer.at(index + 1)) << 16 |
                    quint8(buffer.at(index + 2)) << 8  |
                    quint8(buffer.at(index + 3));
    return value;
}

quint16 RDMProtocol::calculateChecksum(bool startCode, const QByteArray &ba, int len)
{
    ushort cs = startCode ? 0 : RDM_START_CODE;

    for (int i = 0; i < len; i++)
        cs += uchar(ba.at(i));

    return cs;
}

QString RDMProtocol::responseToString(quint8 response)
{
    switch (response)
    {
        case RESPONSE_TYPE_ACK: return "ACK";
        case RESPONSE_TYPE_ACK_TIMER: return "TIMEOUT";
        case RESPONSE_TYPE_NACK_REASON: return "NACK";
        case RESPONSE_TYPE_ACK_OVERFLOW: return "OVERFLOW";
        default: return "UNKNOWN";
    }
}

QString RDMProtocol::categoryToString(quint16 category)
{
    switch (category)
    {
        case 0x0000: return "Not Declared";
        case 0x0100: return "Fixture";
        case 0x0101: return "Fixture Fixed";
        case 0x0102: return "Fixture Moving Yoke";
        case 0x0103: return "Fixture Moving Mirror";
        case 0x01FF: return "Fixture Other";
        case 0x0200: return "Fixture Accessory";
        case 0x0201: return "Fixture Accessory Color";
        case 0x0202: return "Fixture Accessory Yoke";
        case 0x0203: return "Fixture Accessory Mirror";
        case 0x0204: return "Fixture Accessory Effect";
        case 0x0205: return "Fixture Accessory Beam";
        case 0x02FF: return "Fixture Accessory Other";
        case 0x0300: return "Projector";
        case 0x0301: return "Projector Fixed";
        case 0x0302: return "Projector Moving Yoke";
        case 0x0303: return "Projector Moving Mirror";
        case 0x03FF: return "Projector Other";
        case 0x0400: return "Atmospheric";
        case 0x0401: return "Atmospheric Effect";
        case 0x0402: return "Atmospheric Pyro";
        case 0x04FF: return "Atmospheric Other";
        case 0x0500: return "Dimmer";
        case 0x0501: return "Dimmer AC Incandescent";
        case 0x0502: return "Dimmer AC Fluorescent";
        case 0x0503: return "Dimmer AC Cold Cathode";
        case 0x0504: return "Dimmer AC non-dim";
        case 0x0505: return "Dimmer AC ELV";
        case 0x0506: return "Dimmer AC Other";
        case 0x0507: return "Dimmer DC Level";
        case 0x0508: return "Dimmer DC PWM";
        case 0x0509: return "Dimmer CS LED";
        case 0x05FF: return "Dimmer Other";
        case 0x0600: return "Power";
        case 0x0601: return "Power Control";
        case 0x0602: return "Power Source";
        case 0x06FF: return "Power Other";
        case 0x0700: return "Scenic";
        case 0x0701: return "Scenic Drive";
        case 0x07FF: return "Scenic Other";
        case 0x0800: return "Data";
        case 0x0801: return "Data Distribution";
        case 0x0802: return "Data Conversion";
        case 0x08FF: return "Data Other";
        case 0x0900: return "AV";
        case 0x0901: return "AV Audio";
        case 0x0902: return "AV Video";
        case 0x09FF: return "AV Other";
        case 0x0A00: return "Monitor";
        case 0x0A01: return "Monitor AC Line Power";
        case 0x0A02: return "Monitor DC Power";
        case 0x0A03: return "Monitor Environmental";
        case 0x0AFF: return "Monitor Other";
        case 0x7000: return "Control";
        case 0x7001: return "Control Controller";
        case 0x7002: return "Control Backup Device";
        case 0x70FF: return "Control Other";
        case 0x7100: return "Test";
        case 0x7101: return "Test Equipment";
        case 0x71FF: return "Test Equipment Other";
        case 0x7FFF: return "Other";
        default: return "Unknown";
    }
}

QString RDMProtocol::pidToString(quint16 pid)
{
    switch (pid)
    {
        case PID_DISC_UNIQUE_BRANCH: return "PID_DISC_UNIQUE_BRANCH";
        case PID_DISC_MUTE: return "PID_DISC_MUTE";
        case PID_DISC_UN_MUTE: return "PID_DISC_UN_MUTE";
        case PID_PROXIED_DEVICES: return "PID_PROXIED_DEVICES";
        case PID_PROXIED_DEVICE_COUNT: return "PID_PROXIED_DEVICE_COUNT";
        case PID_COMMS_STATUS: return "PID_COMMS_STATUS";
        case PID_QUEUED_MESSAGE: return "PID_QUEUED_MESSAGE";
        case PID_STATUS_MESSAGES: return "PID_STATUS_MESSAGES";
        case PID_STATUS_ID_DESCRIPTION: return "PID_STATUS_ID_DESCRIPTION";
        case PID_CLEAR_STATUS_ID: return "PID_CLEAR_STATUS_ID";
        case PID_SUB_DEVICE_STATUS_REPORT_THRESHOLD: return "PID_SUB_DEVICE_STATUS_REPORT_THRESHOLD";
        case PID_SUPPORTED_PARAMETERS: return "PID_SUPPORTED_PARAMETERS";
        case PID_PARAMETER_DESCRIPTION: return "PID_PARAMETER_DESCRIPTION";
        case PID_DEVICE_INFO: return "PID_DEVICE_INFO";
        case PID_PRODUCT_DETAIL_ID_LIST: return "PID_PRODUCT_DETAIL_ID_LIST";
        case PID_DEVICE_MODEL_DESCRIPTION: return "PID_DEVICE_MODEL_DESCRIPTION";
        case PID_MANUFACTURER_LABEL: return "PID_MANUFACTURER_LABEL";
        case PID_DEVICE_LABEL: return "PID_DEVICE_LABEL";
        case PID_FACTORY_DEFAULTS: return "PID_FACTORY_DEFAULTS";
        case PID_LANGUAGE_CAPABILITIES: return "PID_LANGUAGE_CAPABILITIES";
        case PID_LANGUAGE: return "PID_LANGUAGE";
        case PID_SOFTWARE_VERSION_LABEL: return "PID_SOFTWARE_VERSION_LABEL";
        case PID_BOOT_SOFTWARE_VERSION_ID: return "PID_BOOT_SOFTWARE_VERSION_ID";
        case PID_BOOT_SOFTWARE_VERSION_LABEL: return "PID_BOOT_SOFTWARE_VERSION_LABEL";
        case PID_DMX_PERSONALITY: return "PID_DMX_PERSONALITY";
        case PID_DMX_PERSONALITY_DESCRIPTION: return "PID_DMX_PERSONALITY_DESCRIPTION";
        case PID_DMX_START_ADDRESS: return "PID_DMX_START_ADDRESS";
        case PID_SLOT_INFO: return "PID_SLOT_INFO";
        case PID_SLOT_DESCRIPTION: return "PID_SLOT_DESCRIPTION";
        case PID_DEFAULT_SLOT_VALUE: return "PID_DEFAULT_SLOT_VALUE";
        case PID_SENSOR_DEFINITION: return "PID_SENSOR_DEFINITION";
        case PID_SENSOR_VALUE: return "PID_SENSOR_VALUE";
        case PID_RECORD_SENSORS: return "PID_RECORD_SENSORS";
        case PID_DEVICE_HOURS: return "PID_DEVICE_HOURS";
        case PID_LAMP_HOURS: return "PID_LAMP_HOURS";
        case PID_LAMP_STRIKES: return "PID_LAMP_STRIKES";
        case PID_LAMP_STATE: return "PID_LAMP_STATE";
        case PID_LAMP_ON_MODE: return "PID_LAMP_ON_MODE";
        case PID_DEVICE_POWER_CYCLES: return "PID_DEVICE_POWER_CYCLES";
        case PID_DISPLAY_INVERT: return "PID_DISPLAY_INVERT";
        case PID_DISPLAY_LEVEL: return "PID_DISPLAY_LEVEL";
        case PID_PAN_INVERT: return "PID_PAN_INVERT";
        case PID_TILT_INVERT: return "PID_TILT_INVERT";
        case PID_PAN_TILT_SWAP: return "PID_PAN_TILT_SWAP";
        case PID_REAL_TIME_CLOCK: return "PID_REAL_TIME_CLOCK";
        case PID_IDENTIFY_DEVICE: return "PID_IDENTIFY_DEVICE";
        case PID_RESET_DEVICE: return "PID_RESET_DEVICE";
        case PID_POWER_STATE: return "PID_POWER_STATE";
        case PID_PERFORM_SELFTEST: return "PID_PERFORM_SELFTEST";
        case PID_SELF_TEST_DESCRIPTION: return "PID_SELF_TEST_DESCRIPTION";
        case PID_CAPTURE_PRESET: return "PID_CAPTURE_PRESET";
        case PID_PRESET_PLAYBACK: return "PID_PRESET_PLAYBACK";
        case PID_DMX_BLOCK_ADDRESS: return "PID_DMX_BLOCK_ADDRESS";
        case PID_DMX_FAIL_MODE: return "PID_DMX_FAIL_MODE";
        case PID_DMX_STARTUP_MODE: return "PID_DMX_STARTUP_MODE";
        case PID_DIMMER_INFO: return "PID_DIMMER_INFO";
        case PID_MINIMUM_LEVEL: return "PID_MINIMUM_LEVEL";
        case PID_MAXIMUM_LEVEL: return "PID_MAXIMUM_LEVEL";
        case PID_CURVE: return "PID_CURVE";
        case PID_CURVE_DESCRIPTION: return "PID_CURVE_DESCRIPTION";
        case PID_OUTPUT_RESPONSE_TIME: return "PID_OUTPUT_RESPONSE_TIME";
        case PID_OUTPUT_RESPONSE_TIME_DESCRIPTION: return "PID_OUTPUT_RESPONSE_TIME_DESCRIPTION";
        case PID_MODULATION_FREQUENCY: return "PID_MODULATION_FREQUENCY";
        case PID_MODULATION_FREQUENCY_DESCRIPTION: return "PID_MODULATION_FREQUENCY_DESCRIPTION";
        case PID_BURN_IN: return "PID_BURN_IN";
        case PID_LOCK_PIN: return "PID_LOCK_PIN";
        case PID_LOCK_STATE: return "PID_LOCK_STATE";
        case PID_LOCK_STATE_DESCRIPTION: return "PID_LOCK_STATE_DESCRIPTION";
        case PID_IDENTIFY_MODE: return "PID_IDENTIFY_MODE";
        case PID_PRESET_INFO: return "PID_PRESET_INFO";
        case PID_PRESET_STATUS: return "PID_PRESET_STATUS";
        case PID_PRESET_MERGEMODE: return "PID_PRESET_MERGEMODE";
        case PID_POWER_ON_SELF_TEST: return "PID_POWER_ON_SELF_TEST";
        default: return "";
    }
}
