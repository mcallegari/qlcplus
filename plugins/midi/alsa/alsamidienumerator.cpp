/*
  Q Light Controller
  alsamidienumerator.cpp

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

#include "alsamidioutputdevice.h"
#include "alsamidiinputdevice.h"
#include "alsamidiinputthread.h"
#include "alsamidienumerator.h"
#include "midienumerator.h"
#include "alsamidiutil.h"

/****************************************************************************
 * MidiEnumeratorPrivate
 ****************************************************************************/

MidiEnumeratorPrivate::MidiEnumeratorPrivate(MidiEnumerator* parent)
    : QObject(parent)
    , m_alsa(NULL)
    , m_address(NULL)
    , m_inputThread(NULL)
{
    qDebug() << Q_FUNC_INFO;
    initAlsa();
}

MidiEnumeratorPrivate::~MidiEnumeratorPrivate()
{
    qDebug() << Q_FUNC_INFO;

    if (m_inputThread != NULL)
    {
        m_inputThread->stop();

        while (m_outputDevices.isEmpty() == false)
            delete m_outputDevices.takeFirst();

        while (m_inputDevices.isEmpty() == false)
            delete m_inputDevices.takeFirst();

        delete m_inputThread;
        m_inputThread = NULL;
    }
}

void MidiEnumeratorPrivate::initAlsa()
{
    qDebug() << Q_FUNC_INFO;

    if (snd_seq_open(&m_alsa, "default", SND_SEQ_OPEN_DUPLEX, 0) != 0)
    {
        qWarning() << "Unable to open ALSA interface!";
        m_alsa = NULL;
        return;
    }

    /* Set current client information */
    snd_seq_client_info_t* client = NULL;
    snd_seq_client_info_alloca(&client);
    snd_seq_set_client_name(m_alsa, "qlcplus");
    snd_seq_get_client_info(m_alsa, client);

    /* Create an application-level port */
    m_address = new snd_seq_addr_t;
    m_address->port = snd_seq_create_simple_port(m_alsa, "__QLC__",
                      SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ |
                      SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                      SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    m_address->client = snd_seq_client_info_get_client(client);

    /* Create input thread */
    m_inputThread = new AlsaMidiInputThread(m_alsa, m_address, this);
}

void MidiEnumeratorPrivate::rescan()
{
    qDebug() << Q_FUNC_INFO;

    if (m_alsa == NULL)
        return;

    bool changed = false;
    QList <MidiOutputDevice*> destroyOutputs(m_outputDevices);
    QList <MidiInputDevice*> destroyInputs(m_inputDevices);

    snd_seq_client_info_t* clientInfo = NULL;
    snd_seq_client_info_alloca(&clientInfo);

    snd_seq_port_info_t* portInfo = NULL;
    snd_seq_port_info_alloca(&portInfo);

    snd_seq_client_info_set_client(clientInfo, 0);
    while (snd_seq_query_next_client(m_alsa, clientInfo) == 0)
    {
        /* Get the client ID */
        int client = snd_seq_client_info_get_client(clientInfo);

        /* Ignore our own client */
        if (m_address->client == client)
            continue;

        /* Go thru all available ports in the client */
        snd_seq_port_info_set_client(portInfo, client);
        snd_seq_port_info_set_port(portInfo, -1);
        while (snd_seq_query_next_port(m_alsa, portInfo) == 0)
        {
            const snd_seq_addr_t* address = snd_seq_port_info_get_addr(portInfo);
            if (address == NULL)
                continue;

            uint caps = snd_seq_port_info_get_capability(portInfo);
            if (caps & SND_SEQ_PORT_CAP_READ)
            {
                // Don't expose own ports
                QString name = AlsaMidiUtil::extractName(m_alsa, address);
                if (name.contains("__QLC__") == true)
                    continue;

                QVariant uid = AlsaMidiUtil::addressToVariant(address);
                MidiInputDevice* dev = inputDevice(uid);
                if (dev == NULL)
                {
                    AlsaMidiInputDevice* dev = new AlsaMidiInputDevice(
                            uid, name, address, m_alsa, m_inputThread, this);
                    m_inputDevices << dev;
                    changed = true;
                }
                else
                {
                    destroyInputs.removeAll(dev);
                }
            }

            if (caps & SND_SEQ_PORT_CAP_WRITE)
            {
                // Don't expose own ports
                QString name = AlsaMidiUtil::extractName(m_alsa, address);
                if (name.contains("__QLC__") == true)
                    continue;

                QVariant uid = AlsaMidiUtil::addressToVariant(address);
                MidiOutputDevice* dev = outputDevice(uid);
                if (dev == NULL)
                {
                    AlsaMidiOutputDevice* dev = new AlsaMidiOutputDevice(
                                    uid, name, address, m_alsa, m_address, this);
                    m_outputDevices << dev;
                    changed = true;
                }
                else
                {
                    destroyOutputs.removeAll(dev);
                }
            }
        }
    }

    foreach (MidiOutputDevice* dev, destroyOutputs)
    {
        m_outputDevices.removeAll(dev);
        delete dev;
        changed = true;
    }

    foreach (MidiInputDevice* dev, destroyInputs)
    {
        m_inputDevices.removeAll(dev);
        delete dev;
        changed = true;
    }

    if (changed == true)
        emit configurationChanged();
}

MidiOutputDevice* MidiEnumeratorPrivate::outputDevice(const QVariant& uid) const
{
    QListIterator <MidiOutputDevice*> it(m_outputDevices);
    while (it.hasNext() == true)
    {
        MidiOutputDevice* dev(it.next());
        if (dev->uid() == uid)
            return dev;
    }

    return NULL;
}

MidiInputDevice* MidiEnumeratorPrivate::inputDevice(const QVariant& uid) const
{
    QListIterator <MidiInputDevice*> it(m_inputDevices);
    while (it.hasNext() == true)
    {
        MidiInputDevice* dev(it.next());
        if (dev->uid() == uid)
            return dev;
    }

    return NULL;
}

QList <MidiOutputDevice*> MidiEnumeratorPrivate::outputDevices() const
{
    return m_outputDevices;
}

QList <MidiInputDevice*> MidiEnumeratorPrivate::inputDevices() const
{
    return m_inputDevices;
}

/****************************************************************************
 * MIDIEnumerator
 ****************************************************************************/

MidiEnumerator::MidiEnumerator(QObject* parent)
    : QObject(parent)
    , d_ptr(new MidiEnumeratorPrivate(this))
{
    qDebug() << Q_FUNC_INFO;
    connect(d_ptr, SIGNAL(configurationChanged()), this, SIGNAL(configurationChanged()));
}

MidiEnumerator::~MidiEnumerator()
{
    qDebug() << Q_FUNC_INFO;
    delete d_ptr;
    d_ptr = NULL;
}

void MidiEnumerator::rescan()
{
    qDebug() << Q_FUNC_INFO;
    d_ptr->rescan();
}

QList <MidiOutputDevice*> MidiEnumerator::outputDevices() const
{
    return d_ptr->outputDevices();
}

QList <MidiInputDevice*> MidiEnumerator::inputDevices() const
{
    return d_ptr->inputDevices();
}

MidiOutputDevice* MidiEnumerator::outputDevice(const QVariant& uid) const
{
    return d_ptr->outputDevice(uid);
}

MidiInputDevice* MidiEnumerator::inputDevice(const QVariant& uid) const
{
    return d_ptr->inputDevice(uid);
}
