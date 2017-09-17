/*
  Q Light Controller Plus
  networkpacketizer.cpp

  Copyright (c) Massimo Callegari

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

#include "networkpacketizer.h"

#define HEADER_LENGTH       5

NetworkPacketizer::NetworkPacketizer()
{
}

void NetworkPacketizer::initializePacket(QByteArray &packet, int opCode)
{
    packet.clear();
    packet.append((char)0xE6);            // protocol ID MSB
    packet.append((char)0x86);            // protocol ID LSB
    packet.append((char)(opCode >> 8));   // opCode MSB
    packet.append((char)opCode & 0x00FF); // opCode LSB
    packet.append((char)0x00);            // sections number
}

void NetworkPacketizer::addSection(QByteArray &packet, QVariant value)
{
    switch (value.type())
    {
        case QMetaType::Bool:
            packet.append(BoolType);   // section type
            packet.append(value.toBool() ? (char)0x01 : (char)0x00);
        break;
        case QMetaType::Int:
        {
            int intVal = value.toInt();
            packet.append(IntType);                 // section type
            packet.append((char)(intVal >> 24));    // section data MSB3
            packet.append((char)(intVal >> 16));    // section data MSB2
            packet.append((char)(intVal >> 8));     // section data MSB1
            packet.append((char)(intVal & 0x00FF)); // section data LSB
        }
        break;
        case QMetaType::QByteArray:
        {
            QByteArray ba = value.toByteArray();
            packet.append(ByteArrayType);              // section type
            packet.append((char)ba.length() >> 8);     // section length MSB
            packet.append((char)ba.length() & 0x00FF); // section length LSB
            packet.append(ba);
        }
        break;
        case QMetaType::QString:
        {
            QString strVal = value.toString();
            packet.append(StringType);                     // section type
            packet.append((char)strVal.length() >> 8);     // section length MSB
            packet.append((char)strVal.length() & 0x00FF); // section length LSB
            packet.append(strVal.toUtf8());
        }
        break;
        default:
        break;
    }

    // increment the number of sections
    packet[HEADER_LENGTH - 1] = packet.at(HEADER_LENGTH - 1) + 1;
}

int NetworkPacketizer::decodePacket(QByteArray &packet, int &opCode, QVariantList &sections)
{
    int bytes_read = 0;
    quint8 sections_number = 0;

    /* A packet header must be at least 4 bytes long */
    if (packet.length() < HEADER_LENGTH)
        return packet.length();

    /* Check the protocol ID presence */
    if (packet.at(0) != (char)0xE6 || packet.at(1) != (char)0x86)
        return 1;

    bytes_read += 2;

    opCode = ((quint8)packet.at(bytes_read) << 8) + (quint8)packet.at(bytes_read + 1);

    bytes_read += 2;

    sections_number = packet.at(bytes_read++);

    for (int i = 0; i < sections_number; i++)
    {
        quint8 sType = (quint8)packet.at(bytes_read++);

        switch(sType)
        {
            case BoolType:
                sections.append(QVariant((bool)packet.at(bytes_read++)));
            break;
            case IntType:
            {
                int intVal = ((quint8)packet.at(bytes_read) << 24) + ((quint8)packet.at(bytes_read + 1) << 16) +
                             ((quint8)packet.at(bytes_read + 2) << 8) + (quint8)packet.at(bytes_read + 3);
                bytes_read += 4;
                sections.append(QVariant(intVal));
            }
            break;
            case StringType:
            {
                QString strVal;
                int sLength = ((quint8)packet.at(bytes_read) << 8) + (quint8)packet.at(bytes_read + 1);
                bytes_read += 2;

                strVal.append(packet.mid(bytes_read, sLength));
                sections.append(QVariant(strVal));
                bytes_read += sLength;
            }
            break;
            case ByteArrayType:
            {
                int sLength = ((quint8)packet.at(bytes_read) << 8) + (quint8)packet.at(bytes_read + 1);
                bytes_read += 2;

                sections.append(QVariant(packet.mid(bytes_read, sLength)));
                bytes_read += sLength;
            }
            break;
        }
    }

    return bytes_read;
}
