/*
  Q Light Controller
  alsamidiinputdevice.cpp

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

void AlsaMidiInputDevice::open()
{
    qDebug() << Q_FUNC_INFO;
    m_thread->addDevice(this);
    m_open = true;
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
