/*
  Q Light Controller
  alsamidiutil.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
