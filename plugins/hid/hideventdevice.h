/*
  Q Light Controller
  hideventdevice.h

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

#ifndef HIDEVENTDEVICE_H
#define HIDEVENTDEVICE_H

#include <QObject>
#include <QFile>
#include <QHash>

#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/types.h>
#include <stdint.h>

#include "hiddevice.h"

class HIDEventDevice;
class HID;

/*****************************************************************************
 * HIDEventDevice
 *****************************************************************************/

class HIDEventDevice : public HIDDevice
{
    Q_OBJECT

public:
    HIDEventDevice(HID* parent, quint32 line, const QString& path);
    virtual ~HIDEventDevice();

protected:
    /** @reimp */
    void init();

    /**
     * Find out the HID device's capabilities
     */
    void getCapabilities(uint8_t* mask);

    /**
     * Find out the capabilities of absolute axes
     */
    void getAbsoluteAxesCapabilities();

    /*********************************************************************
     * File operations
     *********************************************************************/
public:
    /** @reimp */
    bool open();

    /** @reimp */
    void close();

    /** @reimp */
    QString path() const;

    /** @reimp */
    bool readEvent();

protected:
    /** Scaling values for absolute/relative axes */
    QHash <int, struct input_absinfo> m_scales;

    /*********************************************************************
     * Device info
     *********************************************************************/
public:
    /** @reimp */
    QString infoText();

    /*********************************************************************
     * Input data
     *********************************************************************/
public:
    /** @reimp */
    void feedBack(quint32 channel, uchar value);
};

#endif
