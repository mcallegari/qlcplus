/*
  Q Light Controller Plus
  dmx4all.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef STAGEPROFI_H
#define STAGEPROFI_H

#include "dmxusbwidget.h"

class QLCFTDI;

class DMX4ALL : public DMXUSBWidget
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    DMX4ALL(const QString& serial, const QString& name, const QString& vendor,
                    QLCFTDI *ftdi = NULL, quint32 id = 0);
    virtual ~DMX4ALL();

    /** @reimp */
    DMXUSBWidget::Type type() const;

    /************************************************************************
     * Widget functions
     ************************************************************************/
public:
    /** @reimp */
    bool open();

    /** @reimp */
    QString uniqueName() const;

    /** @reimp */
    QString additionalInfo() const;

    /** @reimp */
    bool writeUniverse(const QByteArray& universe);

private:
    bool checkReply();
    bool sendChannelValue(int channel, uchar value);

private:
    QByteArray m_universe;
};

#endif
