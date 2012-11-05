/*
  Q Light Controller
  hpmprivate-iokit.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef HPMPRIVATE_IOKIT_H
#define HPMPRIVATE_IOKIT_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <mach/mach_port.h>

#include <QThread>

class HotPlugMonitor;

class HPMPrivate : public QThread
{
    Q_OBJECT

    friend void onHPMPrivateRawDeviceAdded(void* refCon, io_iterator_t iterator);
    friend void onHPMPrivateRawDeviceRemoved(void* refCon, io_iterator_t iterator);

public:
    HPMPrivate(HotPlugMonitor* parent);
    ~HPMPrivate();

public slots:
    void stop();

private:
    void extractVidPid(io_service_t usbDevice, UInt16* vid, UInt16* pid);
    void deviceAdded(io_iterator_t iterator);
    void deviceRemoved(io_iterator_t iterator);
    void run();

private:
    bool m_run;
    CFRunLoopRef loop;
};

#endif
