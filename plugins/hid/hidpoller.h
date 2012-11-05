/*
  Q Light Controller
  hidpoller.h

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

#ifndef HIDPOLLER_H
#define HIDPOLLER_H

#include <QThread>
#include <QMutex>
#include <QMap>

class HIDDevice;
class HID;

class HIDPoller : public QThread
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    HIDPoller(HID* parent);
    ~HIDPoller();

    /*********************************************************************
     * Polled devices
     *********************************************************************/
public:
    bool addDevice(HIDDevice* device);
    bool removeDevice(HIDDevice* device);

protected:
    QMap <int, HIDDevice*> m_devices;
    bool m_changed;
    QMutex m_mutex;

    /*********************************************************************
     * Poller thread
     *********************************************************************/
public:
    virtual void stop();

protected:
    virtual void run();
    void readEvent(struct pollfd pfd);

protected:
    bool m_running;
};

#endif
