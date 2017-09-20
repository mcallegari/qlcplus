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

#include <QDebug>

#include "networkpacketizer.h"
#include "simplecrypt.h"

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
    packet.append((char)0x00);            // sections length MSB
    packet.append((char)0x00);            // sections length LSB
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
            QByteArray strVal = value.toString().toUtf8();
            packet.append(StringType);                     // section type
            packet.append((char)strVal.length() >> 8);     // section length MSB
            packet.append((char)strVal.length() & 0x00FF); // section length LSB
            packet.append(strVal);
        }
        break;
        default:
        break;
    }

    quint16 newLength = packet.length() - HEADER_LENGTH;
    // increment the number of sections
    packet[4] = packet.at(4) + 1;
    // update the total sections length
    packet[5] = newLength >> 8;
    packet[6] = newLength & 0xFF;
}

QByteArray NetworkPacketizer::encryptPacket(QByteArray &packet, SimpleCrypt *crypter)
{
    QByteArray encPacket = packet.mid(0, HEADER_LENGTH); // copy the fixed size header
    encPacket.append(crypter->encryptToByteArray(packet.mid(HEADER_LENGTH))); // encrypt the rest

    quint16 newLength = encPacket.length() - HEADER_LENGTH;
    encPacket[5] = newLength >> 8;
    encPacket[6] = newLength & 0xFF;

    return encPacket;
}

int NetworkPacketizer::decodePacket(QByteArray &packet, int &opCode, QVariantList &sections, SimpleCrypt *decrypter)
{
    int bytes_read = 0;
    quint8 sections_number = 0;
    quint16 sections_length = 0;
    QByteArray ba;

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

    sections_length = ((quint8)packet.at(bytes_read) << 8) + (quint8)packet.at(bytes_read + 1);
    bytes_read += 2;

    if (decrypter)
    {
        QByteArray payload = packet.mid(bytes_read, sections_length);
        qDebug() << "section length:" << sections_length << "payload len:" << payload.length();
        ba = decrypter->decryptToByteArray(payload);
        bytes_read = 0;
        if (ba.length() == 0)
        {
            qDebug() << "decryption error:" << decrypter->lastError();
            return bytes_read + sections_length;
        }
    }
    else
        ba = packet;

    for (int i = 0; i < sections_number; i++)
    {
        quint8 sType = (quint8)ba.at(bytes_read++);

        qDebug() << "Section" << i << "type" << sType;

        switch(sType)
        {
            case BoolType:
                sections.append(QVariant((bool)ba.at(bytes_read++)));
            break;
            case IntType:
            {
                int intVal = ((quint8)ba.at(bytes_read) << 24) + ((quint8)ba.at(bytes_read + 1) << 16) +
                             ((quint8)ba.at(bytes_read + 2) << 8) + (quint8)ba.at(bytes_read + 3);
                bytes_read += 4;
                sections.append(QVariant(intVal));
            }
            break;
            case StringType:
            {
                QString strVal;
                int sLength = ((quint8)ba.at(bytes_read) << 8) + (quint8)ba.at(bytes_read + 1);
                bytes_read += 2;

                strVal.append(ba.mid(bytes_read, sLength));
                sections.append(QVariant(strVal));
                bytes_read += sLength;
            }
            break;
            case ByteArrayType:
            {
                int sLength = ((quint8)ba.at(bytes_read) << 8) + (quint8)ba.at(bytes_read + 1);
                bytes_read += 2;

                sections.append(QVariant(ba.mid(bytes_read, sLength)));
                bytes_read += sLength;
            }
            break;
            default:
                qDebug() << "Unknown section type" << sType;
            break;
        }
    }

    return HEADER_LENGTH + sections_length;
}
