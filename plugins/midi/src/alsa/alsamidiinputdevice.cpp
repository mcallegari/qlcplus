/*
  Q Light Controller
  alsamidiinputdevice.cpp

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

#include <alsa/asoundlib.h>
#include <QDebug>

#include "alsamidiinputdevice.h"
#include "alsamidiinputthread.h"
#include "midiprotocol.h"

/****************************************************************************
 * AlsaMidiInputDevice
 ****************************************************************************/

AlsaMidiInputDevice::AlsaMidiInputDevice(const QVariant& uid,
                                         const QString& name,
                                         const snd_seq_addr_t* address,
                                         snd_seq_t* alsa,
                                         AlsaMidiInputThread* thread,
                                         QObject* parent)
    : MidiInputDevice(uid, name, parent)
    , m_alsa(alsa)
    , m_address(new snd_seq_addr_t)
    , m_thread(thread)
    , m_open(false)
    , m_mbc_counter(UINT_MAX)
{
    Q_ASSERT(alsa != NULL);
    Q_ASSERT(thread != NULL);

    Q_ASSERT(address != NULL);
    m_address->client = address->client;
    m_address->port = address->port;
    qDebug() << "[AlsaMidiInputDevice] client: " << m_address->client << ", port: " << m_address->port;
}

AlsaMidiInputDevice::~AlsaMidiInputDevice()
{
    qDebug() << Q_FUNC_INFO;
    close();

    delete m_address;
    m_address = NULL;
}

bool AlsaMidiInputDevice::open()
{
    qDebug() << Q_FUNC_INFO;
    m_thread->addDevice(this);
    m_open = true;
    return true;
}

void AlsaMidiInputDevice::close()
{
    qDebug() << Q_FUNC_INFO;
    m_thread->removeDevice(this);
    m_open = false;
}

bool AlsaMidiInputDevice::isOpen() const
{
    qDebug() << Q_FUNC_INFO;
    return m_open;
}

const snd_seq_addr_t* AlsaMidiInputDevice::address() const
{
    return m_address;
}

bool AlsaMidiInputDevice::processMBC(snd_seq_event_type_t type)
{
    if (type == SND_SEQ_EVENT_START || type == SND_SEQ_EVENT_STOP)
    {
        m_mbc_counter = 1;
        return true;
    }
    else if (type == SND_SEQ_EVENT_CONTINUE)
    {
        return true;
    }
    else if (type == SND_SEQ_EVENT_CLOCK)
    {
        if (m_mbc_counter == UINT_MAX)
        {
            m_mbc_counter = 1;
            return true;
        }
        m_mbc_counter++;
        if (m_mbc_counter == MIDI_BEAT_CLOCK_PPQ)
        {
            m_mbc_counter = 0;
            return true;
        }
    }
    return false;
}
