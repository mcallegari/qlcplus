/*
  Q Light Controller
  alsamidiinputthread.h

  Copyright (c) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
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
