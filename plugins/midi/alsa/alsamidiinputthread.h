/*
  Q Light Controller
  alsamidiinputthread.h

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

#ifndef ALSAMIDIINPUTTHREAD_H
#define ALSAMIDIINPUTTHREAD_H

#include <QVariant>
#include <QThread>
#include <QMutex>
#include <QHash>

struct _snd_seq;
typedef _snd_seq snd_seq_t;

struct snd_seq_addr;
typedef snd_seq_addr snd_seq_addr_t;

class AlsaMidiInputDevice;

class AlsaMidiInputThread : public QThread
{
    Q_OBJECT

public:
    AlsaMidiInputThread(snd_seq_t* alsa, const snd_seq_addr_t* destinationAddress,
                        QObject* parent = 0);
    ~AlsaMidiInputThread();

private:
    snd_seq_t* m_alsa;
    snd_seq_addr_t* m_destinationAddress;

    /*************************************************************************
     * Devices
     *************************************************************************/
public:
    /** Add a new MIDI device to be polled for events */
    bool addDevice(AlsaMidiInputDevice* device);

    /** Remove the given device from the poller list */
    bool removeDevice(AlsaMidiInputDevice* device);

private:
    /** Subscribe a device's events to come thru to the plugin's port */
    void subscribeDevice(AlsaMidiInputDevice* device);

    /** Unsubscribe a device's events */
    void unsubscribeDevice(AlsaMidiInputDevice* device);

private:
    QHash <uint,AlsaMidiInputDevice*> m_devices;

    /*************************************************************************
     * Poller thread
     *************************************************************************/
public:
    void stop();

private:
    void run();

    void readEvent();

private:
    bool m_running;
    bool m_changed;
    QMutex m_mutex;
};

#endif
