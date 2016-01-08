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
    data.append(path);

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

    data.append(path);
    // add trailing zeros to reach a multiple of 4
    int zeroNumber = 4 - (path.length() % 4);
    if (zeroNumber > 0)
        data.append(QByteArray(zeroNumber, 0x00));

    data.append(",");
    data.append(types);

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
bool OSCPacketizer::parseMessage(QByteArray &data, QString &path, QByteArray &values)
{
    path.clear();
    values.clear();

    QList<TagType> typeArray;
    bool tagsEnded = false;

    // first of all look for a comma
    int commaPos = data.indexOf(0x2C);
    if (commaPos == -1)
        return false;

    path = QString(data.mid(0, commaPos));
    qDebug() << " [OSC] path extracted:" << path;

    int currPos = commaPos + 1;
    while (tagsEnded == false)
    {
        switch (data.at(currPos))
        {
            case 0x00: tagsEnded = true; break;
            case 0x62: /* 'b' */ typeArray.append(Blob); break;
            case 0x66: /* 'f' */ typeArray.append(Float); break;
            case 0x69: /* 'i' */ typeArray.append(Integer); break;
            case 0x73: /* 's' */ typeArray.append(String); break;
            default: break;
        }
        currPos++;
    }
    // round current position to 4
    if (typeArray.count() < 4)
        currPos += (2 - typeArray.count());

    qDebug () << "[OSC] Tags found:" << typeArray.count() << "currpos at" << currPos;

    foreach(TagType tag, typeArray)
    {
        switch (tag)
        {
            case Integer:
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
            case Float:
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
            case String:
            {
                int firstZeroPos = data.indexOf('\0', currPos);
                QString str = QString(data.mid(currPos, firstZeroPos - currPos));
                qDebug() << "[OSC] sVal:" << str;
                // align current position to a multiple of 4
                int zeroNumber = 4 - (str.length() % 4);
                currPos = firstZeroPos + zeroNumber;
            }
            break;
            default: break;
        }
    }

    return true;
}



