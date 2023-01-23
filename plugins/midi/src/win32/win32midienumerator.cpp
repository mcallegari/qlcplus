/*
  Q Light Controller
  win32midienumeratorprivate.cpp

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

#include <windows.h>
#include <QDebug>

#include "win32midienumeratorprivate.h"
#include "win32midioutputdevice.h"
#include "win32midiinputdevice.h"

/****************************************************************************
 * MIDIEnumeratorPrivate
 ****************************************************************************/

MidiEnumeratorPrivate::MidiEnumeratorPrivate(MidiEnumerator* parent)
    : QObject(parent)
{
    qDebug() << Q_FUNC_INFO;
}

MidiEnumeratorPrivate::~MidiEnumeratorPrivate()
{
    qDebug() << Q_FUNC_INFO;

    while (m_outputDevices.isEmpty() == false)
        delete m_outputDevices.takeFirst();

    while (m_inputDevices.isEmpty() == false)
        delete m_inputDevices.takeFirst();
}

MidiEnumerator* MidiEnumeratorPrivate::enumerator() const
{
    return qobject_cast<MidiEnumerator*> (parent());
}

QVariant MidiEnumeratorPrivate::extractUID(UINT id)
{
    return QVariant(id);
}

QString MidiEnumeratorPrivate::extractInputName(UINT id)
{
    MIDIINCAPS caps;
    MMRESULT result = midiInGetDevCaps(id, &caps, sizeof(MIDIINCAPS));
    if (result == MMSYSERR_NOERROR)
    {
#ifdef UNICODE
        return QString::fromUtf16(reinterpret_cast<char16_t*>(caps.szPname));
#else
        return QString::fromLocal8Bit(caps.szPname);
#endif
    }
    else
    {
        return QString();
    }
}

QString MidiEnumeratorPrivate::extractOutputName(UINT id)
{
    MIDIOUTCAPS caps;
    MMRESULT result = midiOutGetDevCaps(id, &caps, sizeof(MIDIOUTCAPS));
    if (result == MMSYSERR_NOERROR)
    {
#ifdef UNICODE
        return QString::fromUtf16(reinterpret_cast<char16_t*>(caps.szPname));
#else
        return QString::fromLocal8Bit(caps.szPname);
#endif
    }
    else
    {
        return QString();
    }
}

void MidiEnumeratorPrivate::rescan()
{
    qDebug() << Q_FUNC_INFO;

    // OUTPUT
    // Destroy existing outputs since there is no way of knowing if something has
    // disappeared or appeared in the middle between 0 - midiOutGetNumDevs().
    while (m_outputDevices.isEmpty() == false)
        delete m_outputDevices.takeFirst();

    // Create new device instances for each valid midi output
    for (UINT id = 0; id < midiOutGetNumDevs(); id++)
    {
        QVariant uid = extractUID(id);
        QString name = extractOutputName(id);
        Win32MidiOutputDevice* dev = new Win32MidiOutputDevice(uid, name, id, this);
        m_outputDevices << dev;
    }

    // INPUT
    // Destroy existing inputs since there is no way of knowing if something has
    // disappeared or appeared in the middle between 0 - midiInGetNumDevs().
    while (m_inputDevices.isEmpty() == false)
        delete m_inputDevices.takeFirst();

    // Create new device instances for each valid midi input
    for (UINT id = 0; id < midiInGetNumDevs(); id++)
    {
        QVariant uid = extractUID(id);
        QString name = extractInputName(id);
        Win32MidiInputDevice* dev = new Win32MidiInputDevice(uid, name, id, this);
        m_inputDevices << dev;
    }

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
