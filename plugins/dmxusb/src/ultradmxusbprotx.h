/*
  Q Light Controller
  ultradmxusbprotx.h

  Copyright (C) Massimo Callegari

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

#ifndef ULTRADMXUSBPROTX_H
#define ULTRADMXUSBPROTX_H

#include "enttecdmxusbpro.h"

#define SEND_DMX_PORT1  0x64
#define SEND_DMX_PORT2  0x65

class UltraDMXUSBProTx : public EnttecDMXUSBPro
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    UltraDMXUSBProTx(const QString& serial, const QString& name, const QString& vendor,
                     int port = 0, QLCFTDI *ftdi = NULL, quint32 id = 0);
    ~UltraDMXUSBProTx();

    /** @reimp */
    Type type() const;

private:
    Type m_type;
    int m_port;

    /************************************************************************
     * Open & Close
     ************************************************************************/
public:
    /** @reimp */
    bool open();

    /** @reimp */
    QString uniqueName() const;

    /****************************************************************************
     * Name & Serial
     ****************************************************************************/
public:
    /** @reimp */
    QString additionalInfo() const;

    /************************************************************************
     * Write universe
     ************************************************************************/
public:
    /** @reimp */
    bool writeUniverse(const QByteArray& universe);
};

#endif // ULTRADMXUSBPROTX_H
