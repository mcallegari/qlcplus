/*
  Q Light Controller
  alsamidiutil.h

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

#ifndef ALSAMIDIUTIL_H
#define ALSAMIDIUTIL_H

#include <QVariant>

struct _snd_seq;
typedef _snd_seq snd_seq_t;

struct snd_seq_addr;
typedef snd_seq_addr snd_seq_addr_t;

class AlsaMidiUtil
{
private:
    AlsaMidiUtil() { }
    ~AlsaMidiUtil() { }

public:
    static QVariant addressToVariant(const snd_seq_addr_t* addr);
    static bool variantToAddress(const QVariant& var, snd_seq_addr_t* addr);
    static QString extractName(snd_seq_t* alsa, const snd_seq_addr_t* addr);
};

#endif
