/*
  Q Light Controller
  alsamidiutil.cpp

  Copyright (c) Heikki Junnila

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

#include <alsa/asoundlib.h>
#include <QDebug>

#include "alsamidiutil.h"

QVariant AlsaMidiUtil::addressToVariant(const snd_seq_addr_t* addr)
{
    Q_ASSERT(addr != NULL);
    uint value = addr->client << 8;
    value = value | addr->port;
    return QVariant(value);
}

bool AlsaMidiUtil::variantToAddress(const QVariant& var, snd_seq_addr_t* addr)
{
    Q_ASSERT(addr != NULL);

    if (var.isValid() == false)
        return false;

    uint value = var.toUInt();
    addr->client = (value >> 8);
    addr->port = (value & 0xFF);
    return true;
}

QString AlsaMidiUtil::extractName(snd_seq_t* alsa, const snd_seq_addr_t* address)
{
    //qDebug() << Q_FUNC_INFO;
    Q_ASSERT(alsa != NULL);
    Q_ASSERT(address != NULL);

    snd_seq_port_info_t* portInfo = NULL;
    snd_seq_port_info_alloca(&portInfo);
    int r = snd_seq_get_any_port_info(alsa, address->client, address->port, portInfo);
    if (r == 0)
    {
        qDebug() << "ALSA Port name: " << QString(snd_seq_port_info_get_name(portInfo));
        return QString(snd_seq_port_info_get_name(portInfo));
    }
    else
        return QString();
}
