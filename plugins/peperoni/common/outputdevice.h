/*
  Q Light Controller
  outputdevice.h

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

#ifndef OUTPUTDEVICE_H
#define OUTPUTDEVICE_H

#include "iodevice.h"

class OutputDevice : public IODevice
{
    Q_OBJECT

public:
    OutputDevice(const QVariant& uid, const QString& name, QObject* parent = 0);
    virtual ~OutputDevice();

public:
    virtual void writeChannel(ushort channel, uchar value) = 0;
    virtual void writeUniverse(const QByteArray& universe) = 0;
};

#endif

