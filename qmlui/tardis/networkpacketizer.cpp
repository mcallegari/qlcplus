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
#include <QVector3D>
#include <QRectF>
#include <QColor>
#include <QFont>

#include "networkpacketizer.h"
#include "simplecrypt.h"
#include "scenevalue.h"
#include "tardis.h"

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
    if (value.isNull())
        return;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    switch (QMetaType::Type(value.type()))
#else
    switch (QMetaType::Type(value.metaType().id()))
#endif
    {
        case QMetaType::Bool:
            packet.append(BoolType);   // section type
            packet.append(value.toBool() ? (char)0x01 : (char)0x00);
        break;
        case QMetaType::Int:
        case QMetaType::UInt:
        {
            int intVal = value.toInt();
            packet.append(IntType);                 // section type
            packet.append((char)(intVal >> 24));    // section data MSB3
            packet.append((char)(intVal >> 16));    // section data MSB2
            packet.append((char)(intVal >> 8));     // section data MSB1
            packet.append((char)(intVal & 0x00FF)); // section data LSB
        }
        break;
        case QMetaType::Double:
        {
            float val = value.toFloat();
            packet.append(FloatType);
            packet.append(reinterpret_cast<const char*>(&val), sizeof(val));
        }
        break;
        case QMetaType::QByteArray:
        {
            QByteArray ba = value.toByteArray();
            packet.append(ByteArrayType);                // section type
            packet.append((char)(ba.length() >> 8));     // section length MSB
            packet.append((char)(ba.length() & 0x00FF)); // section length LSB
            packet.append(ba);
        }
        break;
        case QMetaType::QString:
        {
            QByteArray strVal = value.toString().toUtf8();
            packet.append(StringType);                       // section type
            packet.append((char)(strVal.length() >> 8));     // section length MSB
            packet.append((char)(strVal.length() & 0x00FF)); // section length LSB
            packet.append(strVal);
        }
        break;
        case QMetaType::QVector3D:
        {
            QVector3D vect = value.value<QVector3D>();
            float x = vect.x();
            float y = vect.y();
            float z = vect.z();
            packet.append(Vector3DType);
            packet.append(reinterpret_cast<const char*>(&x), sizeof(x));
            packet.append(reinterpret_cast<const char*>(&y), sizeof(y));
            packet.append(reinterpret_cast<const char*>(&z), sizeof(z));
        }
        break;
        case QMetaType::QRectF:
        {
            QRectF rect = value.value<QRectF>();
            float x = rect.x();
            float y = rect.y();
            float w = rect.width();
            float h = rect.height();
            packet.append(RectFType);
            packet.append(reinterpret_cast<const char*>(&x), sizeof(x));
            packet.append(reinterpret_cast<const char*>(&y), sizeof(y));
            packet.append(reinterpret_cast<const char*>(&w), sizeof(w));
            packet.append(reinterpret_cast<const char*>(&h), sizeof(h));
        }
        break;
        case QMetaType::QColor:
        {
            QRgb rgb = value.value<QColor>().rgb();
            packet.append(ColorType);               // section type
            packet.append((char)(rgb >> 24));    // section data MSB3
            packet.append((char)(rgb >> 16));    // section data MSB2
            packet.append((char)(rgb >> 8));     // section data MSB1
            packet.append((char)(rgb & 0x00FF)); // section data LSB
        }
        break;
        case QMetaType::QFont:
        {
            QFont font = value.value<QFont>();
            QByteArray strVal = font.toString().toUtf8();
            packet.append(FontType);                         // section type
            packet.append((char)(strVal.length() >> 8));     // section length MSB
            packet.append((char)(strVal.length() & 0x00FF)); // section length LSB
            packet.append(strVal);
        }
        break;
        default:
        {
            if (value.canConvert<SceneValue>())
            {
                SceneValue scv = value.value<SceneValue>();
                packet.append(SceneValueType);
                packet.append((char)(scv.fxi >> 24));    // MSB3
                packet.append((char)(scv.fxi >> 16));    // MSB2
                packet.append((char)(scv.fxi >> 8));     // MSB1
                packet.append((char)(scv.fxi & 0x00FF)); // LSB
                packet.append((char)(scv.channel >> 8));     // MSB
                packet.append((char)(scv.channel & 0x00FF)); // LSB
                packet.append((char)(scv.value & 0x00FF));   // LSB
            }
            else if (value.canConvert<UIntPair>())
            {
                UIntPair pairVal = value.value<UIntPair>();
                packet.append(UIntPairType);
                packet.append((char)(pairVal.first >> 24));    // MSB3
                packet.append((char)(pairVal.first >> 16));    // MSB2
                packet.append((char)(pairVal.first >> 8));     // MSB1
                packet.append((char)(pairVal.first & 0x00FF)); // LSB
                packet.append((char)(pairVal.second >> 24));    // MSB3
                packet.append((char)(pairVal.second >> 16));    // MSB2
                packet.append((char)(pairVal.second >> 8));     // MSB1
                packet.append((char)(pairVal.second & 0x00FF)); // LSB
            }
            else if (value.canConvert<StringIntPair>())
            {
                StringIntPair pairVal = value.value<StringIntPair>();
                packet.append(StringIntPairType);
                QByteArray strVal = pairVal.first.toUtf8();
                packet.append((char)(strVal.length() >> 8));     // string length MSB
                packet.append((char)(strVal.length() & 0x00FF)); // string length LSB
                packet.append(strVal);
                packet.append((char)(pairVal.second >> 24));    // MSB3
                packet.append((char)(pairVal.second >> 16));    // MSB2
                packet.append((char)(pairVal.second >> 8));     // MSB1
                packet.append((char)(pairVal.second & 0x00FF)); // LSB
            }
            else if (value.canConvert<StringDoublePair>())
            {
                StringDoublePair pairVal = value.value<StringDoublePair>();
                packet.append(StringDoublePairType);
                QByteArray strVal = pairVal.first.toUtf8();
                packet.append((char)(strVal.length() >> 8));     // string length MSB
                packet.append((char)(strVal.length() & 0x00FF)); // string length LSB
                packet.append(strVal);
                char *byteArray = (char*)&pairVal.second;
                for (int ds = 0; ds < int(sizeof(double)); ds++)
                    packet.append(byteArray[ds]);
            }
            else if (value.canConvert<StringStringPair>())
            {
                StringStringPair pairVal = value.value<StringStringPair>();
                packet.append(StringStringPairType);
                QByteArray strVal = pairVal.first.toUtf8();
                packet.append((char)(strVal.length() >> 8));     // string length MSB
                packet.append((char)(strVal.length() & 0x00FF)); // string length LSB
                packet.append(strVal);
                strVal = pairVal.second.toUtf8();
                packet.append((char)(strVal.length() >> 8));     // string length MSB
                packet.append((char)(strVal.length() & 0x00FF)); // string length LSB
                packet.append(strVal);
            }
            else
            {
                qDebug() << "Unsupported data metatype" << value.typeName() << "Implement me";
            }
        }
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
        return 1;

    /* Check the protocol ID presence */
    if (packet.at(0) != (char)0xE6 || packet.at(1) != (char)0x86)
        return 1;

    bytes_read += 2;

    opCode = ((quint8)packet.at(bytes_read) << 8) + (quint8)packet.at(bytes_read + 1);
    bytes_read += 2;

    sections_number = packet.at(bytes_read++);

    sections_length = ((quint8)packet.at(bytes_read) << 8) + (quint8)packet.at(bytes_read + 1);
    bytes_read += 2;

    if (packet.length() < HEADER_LENGTH + sections_length)
        return -1;

    if (decrypter)
    {
        QByteArray payload = packet.mid(bytes_read, sections_length);
        qDebug() << "section length:" << sections_length << "payload len:" << payload.length();
        ba = decrypter->decryptToByteArray(payload);
        if (ba.length() == 0)
        {
            qDebug() << "decryption error:" << decrypter->lastError();
            return bytes_read + sections_length;
        }
        bytes_read = 0;
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
            {
                sections.append(QVariant((bool)ba.at(bytes_read++)));
            }
            break;
            case IntType:
            {
                int intVal = ((quint8)ba.at(bytes_read) << 24) + ((quint8)ba.at(bytes_read + 1) << 16) +
                             ((quint8)ba.at(bytes_read + 2) << 8) + (quint8)ba.at(bytes_read + 3);
                bytes_read += 4;
                sections.append(QVariant(intVal));
            }
            break;
            case FloatType:
            {
                float val = *reinterpret_cast<const float *>(ba.data() + bytes_read);
                bytes_read += sizeof(val);
                sections.append(QVariant(val));
            }
            break;
            case StringType:
            case FontType:
            {
                QString strVal;
                quint16 sLength = ((quint16)ba.at(bytes_read) << 8) + (quint16)ba.at(bytes_read + 1);
                bytes_read += 2;

                strVal.append(ba.mid(bytes_read, sLength));
                if (sType == FontType)
                {
                    QFont font;
                    font.fromString(strVal);
                    sections.append(QVariant(font));
                }
                else
                {
                    sections.append(QVariant(strVal));
                }

                bytes_read += sLength;
            }
            break;
            case ByteArrayType:
            {
                quint16 sLength = ((quint16)ba.at(bytes_read) << 8) + (quint16)ba.at(bytes_read + 1);
                bytes_read += 2;

                sections.append(QVariant(ba.mid(bytes_read, sLength)));
                bytes_read += sLength;
            }
            break;
            case Vector3DType:
            {
                float x = *reinterpret_cast<const float *>(ba.data() + bytes_read);
                bytes_read += sizeof(x);
                float y = *reinterpret_cast<const float *>(ba.data() + bytes_read);
                bytes_read += sizeof(y);
                float z = *reinterpret_cast<const float *>(ba.data() + bytes_read);
                bytes_read += sizeof(z);
                sections.append(QVariant(QVector3D(x, y, z)));
            }
            break;
            case RectFType:
            {
                float x = *reinterpret_cast<const float *>(ba.data() + bytes_read);
                bytes_read += sizeof(x);
                float y = *reinterpret_cast<const float *>(ba.data() + bytes_read);
                bytes_read += sizeof(y);
                float w = *reinterpret_cast<const float *>(ba.data() + bytes_read);
                bytes_read += sizeof(w);
                float h = *reinterpret_cast<const float *>(ba.data() + bytes_read);
                bytes_read += sizeof(h);
                sections.append(QVariant(QRectF(x, y, w, h)));
            }
            break;
            case ColorType:
            {
                QRgb rgbVal = ((quint8)ba.at(bytes_read) << 24) + ((quint8)ba.at(bytes_read + 1) << 16) +
                             ((quint8)ba.at(bytes_read + 2) << 8) + (quint8)ba.at(bytes_read + 3);
                bytes_read += 4;
                sections.append(QVariant(QColor(rgbVal)));
            }
            break;
            case SceneValueType:
            {
                SceneValue scv;
                scv.fxi = ((quint8)ba.at(bytes_read) << 24) + ((quint8)ba.at(bytes_read + 1) << 16) +
                           ((quint8)ba.at(bytes_read + 2) << 8) + (quint8)ba.at(bytes_read + 3);
                bytes_read += 4;
                scv.channel = ((quint8)ba.at(bytes_read) << 8) + (quint8)ba.at(bytes_read + 1);
                bytes_read += 2;
                scv.value = (quint8)ba.at(bytes_read++);

                QVariant var;
                var.setValue(scv);
                sections.append(var);
            }
            break;
            case UIntPairType:
            {
                UIntPair pairVal;
                pairVal.first = ((quint8)ba.at(bytes_read) << 24) + ((quint8)ba.at(bytes_read + 1) << 16) +
                                ((quint8)ba.at(bytes_read + 2) << 8) + (quint8)ba.at(bytes_read + 3);
                bytes_read += 4;
                pairVal.second = ((quint8)ba.at(bytes_read) << 24) + ((quint8)ba.at(bytes_read + 1) << 16) +
                                 ((quint8)ba.at(bytes_read + 2) << 8) + (quint8)ba.at(bytes_read + 3);
                bytes_read += 4;
                QVariant var;
                var.setValue(pairVal);
                sections.append(var);
            }
            break;
            case StringIntPairType:
            {
                StringIntPair pairVal;
                QString strVal;
                quint16 sLength = ((quint16)ba.at(bytes_read) << 8) + (quint16)ba.at(bytes_read + 1);
                bytes_read += 2;

                strVal.append(ba.mid(bytes_read, sLength));
                pairVal.first = strVal;
                bytes_read += sLength;

                pairVal.second = ((quint8)ba.at(bytes_read) << 24) + ((quint8)ba.at(bytes_read + 1) << 16) +
                                 ((quint8)ba.at(bytes_read + 2) << 8) + (quint8)ba.at(bytes_read + 3);
                bytes_read += 4;

                QVariant var;
                var.setValue(pairVal);
                sections.append(var);
            }
            break;
            case StringDoublePairType:
            {
                StringDoublePair pairVal;
                QString strVal;
                quint16 sLength = ((quint16)ba.at(bytes_read) << 8) + (quint16)ba.at(bytes_read + 1);
                bytes_read += 2;

                strVal.append(ba.mid(bytes_read, sLength));
                pairVal.first = strVal;
                bytes_read += sLength;

                QByteArray dba = ba.mid(bytes_read, sizeof(double));

                pairVal.second = *((double*)dba.data());
                bytes_read += dba.length();

                QVariant var;
                var.setValue(pairVal);
                sections.append(var);
            }
            break;
            case StringStringPairType:
            {
                StringStringPair pairVal;
                QString strVal;
                quint16 sLength = ((quint16)ba.at(bytes_read) << 8) + (quint16)ba.at(bytes_read + 1);
                bytes_read += 2;

                strVal.append(ba.mid(bytes_read, sLength));
                pairVal.first = strVal;
                bytes_read += sLength;

                sLength = ((quint16)ba.at(bytes_read) << 8) + (quint16)ba.at(bytes_read + 1);
                bytes_read += 2;

                strVal.clear();
                strVal.append(ba.mid(bytes_read, sLength));
                pairVal.second = strVal;
                bytes_read += sLength;

                QVariant var;
                var.setValue(pairVal);
                sections.append(var);
            }
            break;
            default:
                qDebug() << "Unknown section type" << sType;
            break;
        }
    }

    return HEADER_LENGTH + sections_length;
}
