/*
  Q Light Controller
  alsamidiinputthread.cpp

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
#include "alsamidiutil.h"
#include "midiprotocol.h"

#define POLL_TIMEOUT_MS 1000

AlsaMidiInputThread::AlsaMidiInputThread(snd_seq_t* alsa,
                                         const snd_seq_addr_t* destinationAddress,
                                         QObject* parent)
    : QThread(parent)
    , m_alsa(alsa)
    , m_destinationAddress(new snd_seq_addr_t)
    , m_running(false)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(alsa != NULL);
    Q_ASSERT(destinationAddress != NULL);
    m_destinationAddress->client = destinationAddress->client;
    m_destinationAddress->port = destinationAddress->port;
}

AlsaMidiInputThread::~AlsaMidiInputThread()
{
    qDebug() << Q_FUNC_INFO;

    m_devices.clear();
    stop();

    delete m_destinationAddress;
    m_destinationAddress = NULL;
}

/****************************************************************************
 * Devices
 ****************************************************************************/

bool AlsaMidiInputThread::addDevice(AlsaMidiInputDevice* device)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(device != NULL);

    QMutexLocker locker(&m_mutex);

    /* Check, whether the hash table already contains the device */
    uint uid = device->uid().toUInt();
    if (m_devices.contains(uid) == true)
    {
        return false;
    }

    /* Subscribe the device's events */
    subscribeDevice(device);

    /* Insert the device into the hash map for later retrieval */
    m_devices.insert(uid, device);
    m_changed = true;

    /* Start the poller thread in case it's not running */
    if (m_running == false && isRunning() == false)
        start();

    return true;
}

bool AlsaMidiInputThread::removeDevice(AlsaMidiInputDevice* device)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(device != NULL);

    bool empty = false;

    {
        QMutexLocker locker(&m_mutex);

        uint uid = device->uid().toUInt();
        if (m_devices.remove(uid) > 0)
        {
           unsubscribeDevice(device);
           m_changed = true;
        }

        empty = (m_devices.size() == 0);
    }

    if (empty)
        stop();

    return true;
}

void AlsaMidiInputThread::subscribeDevice(AlsaMidiInputDevice* device)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(device != NULL);

    /* Subscribe events coming from the the device's MIDI port to get
       patched to the plugin's own MIDI port */
    snd_seq_port_subscribe_t* sub = NULL;
    snd_seq_port_subscribe_alloca(&sub);
    snd_seq_port_subscribe_set_sender(sub, device->address());
    snd_seq_port_subscribe_set_dest(sub, m_destinationAddress);
    snd_seq_subscribe_port(m_alsa, sub);
}

void AlsaMidiInputThread::unsubscribeDevice(AlsaMidiInputDevice* device)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(device != NULL);

    /* Unsubscribe events from the device */
    snd_seq_port_subscribe_t* sub = NULL;
    snd_seq_port_subscribe_alloca(&sub);
    snd_seq_port_subscribe_set_sender(sub, device->address());
    snd_seq_port_subscribe_set_dest(sub, m_destinationAddress);
    snd_seq_unsubscribe_port(m_alsa, sub);
}

/****************************************************************************
 * Poller thread
 ****************************************************************************/

void AlsaMidiInputThread::stop()
{
    qDebug() << Q_FUNC_INFO;

    {
        QMutexLocker locker(&m_mutex);
        m_running = false;
    }

    wait();
}

void AlsaMidiInputThread::run()
{
    qDebug() << Q_FUNC_INFO << "begin";

    struct pollfd* pfd = 0;
    int npfd = 0;

    QMutexLocker locker(&m_mutex);
    m_running = true;
    while (m_running == true)
    {
        if (m_changed == true)
        {
            // Poll descriptors must be re-evaluated
            npfd = snd_seq_poll_descriptors_count(m_alsa, POLLIN);
            pfd = (struct pollfd*) alloca(npfd * sizeof(struct pollfd));
            snd_seq_poll_descriptors(m_alsa, pfd, npfd, POLLIN);
            m_changed = false;
        }

        locker.unlock();

        // Poll for MIDI events from the polled descriptors outside of mutex lock
        if (poll(pfd, npfd, POLL_TIMEOUT_MS) > 0)
            readEvent();

        locker.relock();
    }

    qDebug() << Q_FUNC_INFO << "end";
}

void AlsaMidiInputThread::readEvent()
{
    QMutexLocker locker(&m_mutex);

    /* Wait for input data */
    do
    {
        AlsaMidiInputDevice* device = NULL;
        snd_seq_event_t* ev = NULL;

        /* Receive an event */
        snd_seq_event_input(m_alsa, &ev);

        // Find a device matching the event's address. If one isn't
        // found, skip this event, since we're not interested in it.
        uint uid = AlsaMidiUtil::addressToVariant(&ev->source).toUInt();
        if (m_devices.contains(uid) == true)
            device = m_devices[uid];
        else
            continue;
        Q_ASSERT(device != NULL);

        uchar cmd = 0;
        uchar data1 = 0;
        uchar data2 = 0;

        //qDebug() << "ALSA MIDI event received!" << ev->type;

        if (snd_seq_ev_is_control_type(ev))
        {
            switch (ev->type)
            {
                case SND_SEQ_EVENT_PGMCHANGE:
                    cmd = MIDI_PROGRAM_CHANGE | ev->data.control.channel;
                    data1 = ev->data.control.value;
                    data2 = 127;
                break;

                case SND_SEQ_EVENT_CONTROLLER:
                    cmd = MIDI_CONTROL_CHANGE | ev->data.control.channel;
                    data1 = ev->data.control.param;
                    data2 = ev->data.control.value;
                break;

                case SND_SEQ_EVENT_PITCHBEND:
                    cmd = MIDI_PITCH_WHEEL | ev->data.control.channel;
                    data1 = (ev->data.control.value + 8192) & 0x7f;
                    data2 = (ev->data.control.value + 8192) >> 7;
                break;

                case SND_SEQ_EVENT_KEYPRESS:
                    cmd = MIDI_NOTE_AFTERTOUCH | ev->data.note.channel;
                    data1 = ev->data.note.note;
                    data2 = ev->data.note.velocity;
                break;

                case SND_SEQ_EVENT_CHANPRESS:
                    cmd = MIDI_CHANNEL_AFTERTOUCH | ev->data.control.channel;
                    data1 = ev->data.control.value;
                break;

                default:
                break;
            }
        }
        else if (snd_seq_ev_is_note_type(ev))
        {
            if (ev->type == SND_SEQ_EVENT_NOTEOFF)
                cmd = MIDI_NOTE_OFF | ev->data.note.channel;
            else if (ev->data.note.velocity == 0 && ev->data.note.off_velocity == 0)
                cmd = MIDI_NOTE_OFF | ev->data.note.channel;
            else
                cmd = MIDI_NOTE_ON | ev->data.note.channel;
            data1 = ev->data.note.note;
            data2 = ev->data.note.velocity;
        }
        else if (snd_seq_ev_is_queue_type(ev))
        {
            if (device->processMBC(ev->type) == false)
                continue;
            if (ev->type == SND_SEQ_EVENT_START)
                cmd = MIDI_BEAT_START;
            else if (ev->type == SND_SEQ_EVENT_STOP)
                cmd = MIDI_BEAT_STOP;
            else if (ev->type == SND_SEQ_EVENT_CONTINUE)
                cmd = MIDI_BEAT_CONTINUE;
            else if (ev->type == SND_SEQ_EVENT_CLOCK)
                cmd = MIDI_BEAT_CLOCK;

            qDebug()  << "MIDI clock: " << cmd;
        }

        // ALSA API is a bit controversial on this. snd_seq_event_input() says
        // it ALLOCATES the event but snd_seq_free_event() says this is not
        // needed because the event IS NOT allocated. No crashes observed
        // either way, so I guess freeing nevertheless is a bit safer.
        snd_seq_free_event(ev);

        uint channel = 0;
        uchar value = 0;
        //qDebug() << "MIDI cmd" << cmd << "data1" << data1 << "data2" << data2
        //         << "channel" << MIDI_CH(cmd) << "devch" << device->midiChannel();

        if (QLCMIDIProtocol::midiToInput(cmd, data1, data2, uchar(device->midiChannel()),
                                         &channel, &value) == true)
        {
            device->emitValueChanged(channel, value);
            // for MIDI beat clock signals,
            // generate a synthetic release event
            if (cmd >= MIDI_BEAT_CLOCK && cmd <= MIDI_BEAT_STOP)
                device->emitValueChanged(channel, 0);
        }
    } while (snd_seq_event_input_pending(m_alsa, 0) > 0);
}

