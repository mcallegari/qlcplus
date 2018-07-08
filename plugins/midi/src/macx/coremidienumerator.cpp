/*
  Q Light Controller
  coremidienumerator.cpp

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

#include <QDebug>

#include "coremidienumeratorprivate.h"
#include "coremidioutputdevice.h"
#include "coremidiinputdevice.h"
#include "midienumerator.h"

extern "C"
{
    void onMIDINotify(const MIDINotification* message, void* refCon)
    {
        qDebug() << "[MIDI notification] ID:" << message->messageID;
        MidiEnumeratorPrivate* self = (MidiEnumeratorPrivate*) refCon;
        if (message->messageID == kMIDIMsgObjectAdded ||
            message->messageID == kMIDIMsgObjectRemoved)
        {
            self->rescan();
        }
    }
} // extern "C"

MidiEnumeratorPrivate::MidiEnumeratorPrivate(MidiEnumerator* parent)
    : QObject(parent)
{
    qDebug() << Q_FUNC_INFO;

    OSStatus s = MIDIClientCreate(CFSTR("QLC MIDI Plugin"), onMIDINotify, this, &m_client);
    if (s != 0)
        qWarning() << Q_FUNC_INFO << "Unable to create a MIDI client!";
}

MidiEnumeratorPrivate::~MidiEnumeratorPrivate()
{
    qDebug() << Q_FUNC_INFO;

    while (m_outputDevices.isEmpty() == false)
        delete m_outputDevices.takeFirst();

    while (m_inputDevices.isEmpty() == false)
        delete m_inputDevices.takeFirst();

    if (m_client != 0)
        MIDIClientDispose(m_client);
    m_client = 0;
}

QString MidiEnumeratorPrivate::extractName(MIDIEndpointRef endpoint)
{
    CFStringRef str = NULL;
    QString name;

    /* Get the name property */
    OSStatus s = MIDIObjectGetStringProperty(endpoint, kMIDIPropertyModel, &str);
    if (s != 0)
    {
        s = MIDIObjectGetStringProperty(endpoint, kMIDIPropertyDisplayName, &str);
        if (s != 0)
            qWarning() << "Unable to get manufacturer for MIDI endpoint:" << endpoint;
    }

    if (s == 0)
    {
        /* Convert the name into a QString. */
        CFIndex size = CFStringGetLength(str) + 1;
        char* buf = (char*) malloc(size);
        if (CFStringGetCString(str, buf, size, kCFStringEncodingISOLatin1))
            name = QString(buf);

        free(buf);
        CFRelease(str);
    }

    return name;
}

QVariant MidiEnumeratorPrivate::extractEndpointUID(MIDIEndpointRef endpoint)
{
    SInt32 uid = 0;
    if (MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyUniqueID, &uid) != 0)
    {
        qWarning() << Q_FUNC_INFO << "Unable to get UID from MIDI endpoint" << endpoint;
        return QVariant();
    }
    else
    {
        return QVariant(int(uid));
    }
}

QVariant MidiEnumeratorPrivate::extractEntityUID(MIDIEntityRef entity)
{
    SInt32 uid = 0;
    if (MIDIObjectGetIntegerProperty(entity, kMIDIPropertyUniqueID, &uid) != 0)
    {
        qWarning() << Q_FUNC_INFO << "Unable to get UID from MIDI entity" << entity;
        return QVariant();
    }
    else
    {
        return QVariant(int(uid));
    }
}

void MidiEnumeratorPrivate::rescan()
{
    qDebug() << Q_FUNC_INFO;

    bool changed = false;
    QList <MidiOutputDevice*> destroyOutputs(m_outputDevices);
    QList <MidiInputDevice*> destroyInputs(m_inputDevices);

    /* Find out which devices are still present */
    ItemCount sourceDevices = MIDIGetNumberOfSources();

    for (ItemCount devIndex = 0; devIndex < sourceDevices; devIndex++)
    {
        MIDIEndpointRef sourceDev = MIDIGetSource(devIndex);
        MIDIEntityRef entity = 0;
        QVariant uid;

        MIDIEndpointGetEntity(sourceDev, &entity);

        /* Get the entity's UID */
        if (entity)
            uid = extractEntityUID(entity); // physical device
        else
            uid = extractEndpointUID(sourceDev); // virtual port

        if (uid.isValid() == false)
            continue;

        QString name = extractName(sourceDev);
        qDebug() << Q_FUNC_INFO << "Found source device:" << name << "UID:" << QString::number(uid.toUInt(), 16);

        MidiInputDevice* dev = inputDevice(uid);
        if (dev == NULL)
        {
            CoreMidiInputDevice* dev = new CoreMidiInputDevice(uid, name, sourceDev, m_client, this);
            m_inputDevices << dev;
            changed = true;
        }
        else
        {
            destroyInputs.removeAll(dev);
        }
    }

    ItemCount destDevices = MIDIGetNumberOfDestinations();

    for (ItemCount devIndex = 0; devIndex < destDevices; devIndex++)
    {
        MIDIEndpointRef destDev = MIDIGetDestination(devIndex);
        MIDIEntityRef entity = 0;
        QVariant uid;

        MIDIEndpointGetEntity(destDev, &entity);

        /* Get the entity's UID */
        if (entity)
            uid = extractEntityUID(entity);
        else
            uid = extractEndpointUID(destDev);

        if (uid.isValid() == false)
            continue;

        QString name = extractName(destDev);
        qDebug() << Q_FUNC_INFO << "Found destination device:" << name << "UID:" << QString::number(uid.toUInt(), 16);

        MidiOutputDevice* dev = outputDevice(uid);
        if (dev == NULL)
        {
            CoreMidiOutputDevice* dev = new CoreMidiOutputDevice(uid, name, destDev, m_client, this);
            m_outputDevices << dev;
            changed = true;
        }
        else
        {
            destroyOutputs.removeAll(dev);
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
