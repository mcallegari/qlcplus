/*
  Q Light Controller
  enttecdmxusbprotx.h

  Copyright (C) Heikki Junnila

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

#ifndef ENTTECDMXUSBPROTX_H
#define ENTTECDMXUSBPROTX_H

#include "enttecdmxusbpro.h"

class QByteArray;
class EnttecDMXUSBProTX : public EnttecDMXUSBPro
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    EnttecDMXUSBProTX(const QString& serial, const QString& name, quint32 id = 0);
    ~EnttecDMXUSBProTX();

    /** @reimp */
    Type type() const;

private:
    Type m_type;

    /************************************************************************
     * Open & Close
     ************************************************************************/
public:
    /** @reimp */
    bool open();

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

#endif
