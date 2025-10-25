/*
  Q Light Controller Plus
  utils.h

  Copyright (c) Massimo Callegari
                David Garyga

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

#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>

/** @addtogroup engine Engine
 * @{
 */

struct Utils
{

    static bool vectorRemove(QVector<int>& vec, int val)
    {
        bool ret = false;
        for (int i = 0; i < vec.size(); ++i)
        {
            if (val == vec.at(i))
            {
                vec.remove(i);
                ret = true;
            }
        }
        return ret;
    }

    static void vectorSortedAddUnique(QVector<int>& vec, int val)
    {
        for (int i = 0; i < vec.size(); ++i)
        {
            if (val < vec.at(i))
            {
                vec.insert(i, val);
                return;
            }
            if (val == vec.at(i))
                return;
        }
        vec.append(val);
    }

/*!
    Inherited from the Qt core source code.
    Returns the CRC-16 checksum of \a data.

    The checksum is independent of the byte order (endianness) and will
    be calculated accorded to the algorithm published in \a standard.
    By default the algorithm published in ISO 3309 (Qt::ChecksumIso3309) is used.

    \note This function is a 16-bit cache conserving (16 entry table)
    implementation of the CRC-16-CCITT algorithm.
*/
    static constexpr quint16 crc_tbl[16] = {
        0x0000, 0x1081, 0x2102, 0x3183,
        0x4204, 0x5285, 0x6306, 0x7387,
        0x8408, 0x9489, 0xa50a, 0xb58b,
        0xc60c, 0xd68d, 0xe70e, 0xf78f
    };

    static quint16 getChecksum(QByteArray data)
    {
#if 1
        QByteArrayView bav(data.constData(), data.length());
        return qChecksum(bav);
#else
        quint16 crc = 0xffff;
        uchar c;
        const uchar *p = reinterpret_cast<const uchar *>(data.data());
        qsizetype len = data.size();
        while (len--) {
            c = *p++;
            crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
            c >>= 4;
            crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
        }
        crc = ~crc;
        return crc & 0xffff;
#endif
    }
};

/** @} */

#endif
