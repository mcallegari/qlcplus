/*
  Q Light Controller Plus
  oscpacketizer.cpp

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

#include "oscpacketizer.h"

#include <QStringList>
#include <QDebug>

OSCPacketizer::OSCPacketizer()
{
}

OSCPacketizer::~OSCPacketizer()
{
}

/*********************************************************************
 * Sender functions
 *********************************************************************/

void OSCPacketizer::setupOSCDmx(QByteArray &data, quint32 universe, quint32 channel, uchar value)
{
    data.clear();
    QString path = QString("/%1/dmx/%2").arg(universe).arg(channel);
    data.append(path.toUtf8());

    // add trailing zeros to reach a multiple of 4
    int zeroNumber = 4 - (path.length() % 4);
    if (zeroNumber > 0)
        data.append(QByteArray(zeroNumber, 0x00));

    data.append(",f");
    data.append((char)0x00);
    data.append((char)0x00);
    // Output value
    float fVal = (float)value / 255.f;
    data.append(*(((char *)&fVal) + 3));
    data.append(*(((char *)&fVal) + 2));
    data.append(*(((char *)&fVal) + 1));
    data.append(*(((char *)&fVal) + 0));
}

void OSCPacketizer::setupOSCGeneric(QByteArray &data, QString &path, QString types, QByteArray &values)
{
    data.clear();
    if (path.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "Empty path, can't create packet";
        return;
    }

    data.append(path.toUtf8());
    // add trailing zeros to reach a multiple of 4
    int zeroNumber = 4 - (path.length() % 4);
    if (zeroNumber > 0)
        data.append(QByteArray(zeroNumber, 0x00));

    data.append(",");
    data.append(types.toUtf8());

    zeroNumber = 4 - ((types.length() + 1) % 4);
    if (zeroNumber > 0)
        data.append(QByteArray(zeroNumber, 0x00));

    for (int i = 0; i < types.length() && i < values.length(); i++)
    {
        if (types.at(i) == 'f')
        {
            uchar val = (uchar)values.at(i);
            float fVal = (float)val / 255.f;
            data.append(*(((char *)&fVal) + 3));
            data.append(*(((char *)&fVal) + 2));
            data.append(*(((char *)&fVal) + 1));
            data.append(*(((char *)&fVal) + 0));
        }
    }
}

/*********************************************************************
 * Receiver functions
 *********************************************************************/
bool OSCPacketizer::parseMessage(QByteArray const& data, QString& path, QByteArray& values)
{
    path.clear();
    values.clear();

    QList<TagType> typeArray;
    bool tagsEnded = false;

    //qDebug() << "[OSC] data:" << data;

    // first of all look for a comma
    int commaPos = data.indexOf(0x2C);
    if (commaPos == -1)
        return false;

    path = QString(data.mid(0, commaPos));
    // remove possible trailing zeroes used for padding
    path.remove(QChar(0x00));
    //qDebug() << "[OSC] path extracted:" << path;

    int currPos = commaPos + 1;
    while (tagsEnded == false)
    {
        switch (data.at(currPos))
        {
            case '\0': tagsEnded = true; break;
            case 'b': typeArray.append(BlobTag); break;
            case 'f': typeArray.append(FloatTag); break;
            case 'd': typeArray.append(DoubleTag); break;
            case 'i': typeArray.append(IntegerTag); break;
            case 's': typeArray.append(StringTag); break;
            case 't': typeArray.append(TimeTag); break;
            default: break;
        }
        currPos++;
    }
    // round current position to 4
    int left = (typeArray.count() + 1) % 4;
    currPos += 3 - left;

    //qDebug () << "[OSC] Tags found:" << typeArray.count() << "currpos at" << currPos;

    foreach (TagType tag, typeArray)
    {
        switch (tag)
        {
            case IntegerTag:
            {
                if (currPos + 4 > data.size())
                    break;
                quint32 iVal;

                *((uchar*)(&iVal) + 3) = data.at(currPos);
                *((uchar*)(&iVal) + 2) = data.at(currPos + 1);
                *((uchar*)(&iVal) + 1) = data.at(currPos + 2);
                *((uchar*)(&iVal) + 0) = data.at(currPos + 3);

                if (iVal < 256)
                    values.append((char)iVal);
                else
                    values.append((char)(iVal / 0xFFFFFF));

                qDebug() << "[OSC] iVal:" << iVal;
                currPos += 4;
            }
            break;
            case FloatTag:
            {
                if (currPos + 4 > data.size())
                    break;
                float fVal;

                *((uchar*)(&fVal) + 3) = data.at(currPos);
                *((uchar*)(&fVal) + 2) = data.at(currPos + 1);
                *((uchar*)(&fVal) + 1) = data.at(currPos + 2);
                *((uchar*)(&fVal) + 0) = data.at(currPos + 3);

                values.append((char)(255.0 * fVal));

                qDebug() << "[OSC] fVal:" << fVal;

                currPos += 4;
            }
            break;
            case DoubleTag:
            {
                if (currPos + 8 > data.size())
                    break;

                double dVal;
                *((uchar*)(&dVal) + 7) = data.at(currPos);
                *((uchar*)(&dVal) + 6) = data.at(currPos + 1);
                *((uchar*)(&dVal) + 5) = data.at(currPos + 2);
                *((uchar*)(&dVal) + 4) = data.at(currPos + 3);
                *((uchar*)(&dVal) + 3) = data.at(currPos + 4);
                *((uchar*)(&dVal) + 2) = data.at(currPos + 5);
                *((uchar*)(&dVal) + 1) = data.at(currPos + 6);
                *((uchar*)(&dVal) + 0) = data.at(currPos + 7);

                values.append((char)(255.0 * dVal));

                qDebug() << "[OSC] dVal:" << dVal;
                currPos += 8;
            }
            break;
            case StringTag:
            {
                int firstZeroPos = data.indexOf('\0', currPos);
                QString str = QString(data.mid(currPos, firstZeroPos - currPos));
                qDebug() << "[OSC] string:" << str;
                // align current position to a multiple of 4
                int zeroNumber = 4 - (str.length() % 4);
                currPos = firstZeroPos + zeroNumber;
            }
            break;
            case TimeTag:
            {
                // A OSC timestamp would be helpful to defer
                // value changes, but since QLC+ plugins don't support
                // such thing, let's skip it.
                currPos += 8;
            }
            break;
            default: break;
        }
    }

    return true;
}

QList<QPair<QString, QByteArray> > OSCPacketizer::parsePacket(QByteArray const& data)
{
    int bufPos = 0;
    QList<QPair<QString, QByteArray> > messages;

    while (bufPos < data.size())
    {
        QString path;
        QByteArray values;

        // check wether we need to parse a bundle or a single message
        if (data.at(bufPos) == '#')
        {
            if (data.size() < 20 || data.startsWith("#bundle") == false)
            {
                qWarning() << "[OSC] Found an unsupported message type!" << data;
                return messages;
            }
            // 8 bytes for '#bundle\0' and 8 bytes for a timestamp that we don't handle
            bufPos += 16;

            // now, a bundle can contain another bundle or a message, starting with the message size
            // Check where we are here
            while (bufPos < data.size() && data.at(bufPos) != '#')
            {
                quint32 msgSize = (uchar(data.at(bufPos)) << 24) + (uchar(data.at(bufPos + 1)) << 16) + (uchar(data.at(bufPos + 2)) << 8) + uchar(data.at(bufPos + 3));
                qDebug() << "[OSC] Bundle message size:" << msgSize;
                bufPos += 4;

                if (data.size() >= bufPos + (int)msgSize)
                {
                    QByteArray msgData = data.mid(bufPos, msgSize);
                    if (parseMessage(msgData, path, values) == true)
                        messages.append(QPair<QString, QByteArray>(path, values));
                }
                bufPos += msgSize;
            }
            //qDebug() << "Buf pos=" << bufPos << "data size=" << data.size();
        }
        else
        {
            if (parseMessage(data, path, values) == true)
                messages.append(QPair<QString, QByteArray>(path, values));

            bufPos += data.size();
        }
    }

    return messages;
}



