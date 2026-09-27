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
#include <setupapi.h>
#include <QDebug>

#include "win32midienumeratorprivate.h"
#include "win32midioutputdevice.h"
#include "win32midiinputdevice.h"

// {6994AD04-93EF-11D0-A3CC-00A0C9223196}
static const GUID kKsCategoryMidi = {
    0x6994AD04, 0x93EF, 0x11D0, {0xA3, 0xCC, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96}
};

// Returns a map of device friendly name -> list of device instance IDs (in
// enumeration order). Two identical devices share a name but get distinct
// instance IDs encoding their USB location, e.g. "USB\VID_1235&PID_000A\5&3A4B…".
// Virtual/software MIDI ports (loopMIDI, etc.) don't appear here.
static QMap<QString, QStringList> buildMidiDeviceMap()
{
    QMap<QString, QStringList> result;

    HDEVINFO devInfo = SetupDiGetClassDevs(
        &kKsCategoryMidi, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devInfo == INVALID_HANDLE_VALUE)
        return result;

    SP_DEVICE_INTERFACE_DATA ifaceData;
    ifaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(devInfo, NULL, &kKsCategoryMidi, i, &ifaceData); i++)
    {
        SP_DEVINFO_DATA devData;
        devData.cbSize = sizeof(SP_DEVINFO_DATA);
        DWORD required = 0;
        // Intentionally passing NULL to get devData populated; call will fail with
        // ERROR_INSUFFICIENT_BUFFER but that's expected.
        SetupDiGetDeviceInterfaceDetail(devInfo, &ifaceData, NULL, 0, &required, &devData);

        WCHAR instanceId[256] = {};
        if (!SetupDiGetDeviceInstanceIdW(devInfo, &devData, instanceId, 256, NULL))
            continue;

        WCHAR friendlyName[256] = {};
        DWORD propType = 0;
        if (!SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_FRIENDLYNAME,
                                               &propType, (PBYTE)friendlyName,
                                               sizeof(friendlyName), NULL))
        {
            SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_DEVICEDESC,
                                              &propType, (PBYTE)friendlyName,
                                              sizeof(friendlyName), NULL);
        }

        QString name = QString::fromWCharArray(friendlyName);
        QString instId = QString::fromWCharArray(instanceId).replace('&', ':');

        if (!name.isEmpty() && !instId.isEmpty() && !result[name].contains(instId))
            result[name].append(instId);
    }

    SetupDiDestroyDeviceInfoList(devInfo);
    return result;
}

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

QVariant MidiEnumeratorPrivate::makeUID(const QString& name, QMap<QString, int>& seen,
                                        const QMap<QString, QStringList>& deviceMap)
{
    int count = seen.value(name, 0);
    seen[name] = count + 1;

    const QStringList& ids = deviceMap.value(name);
    if (count < ids.size())
        return QVariant(ids.at(count));

    // Fallback for virtual/software ports not present in SetupAPI
    if (count == 0)
        return QVariant(name);
    return QVariant(QString("%1:%2").arg(name).arg(count));
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

    // Build instance-ID map once; covers both directions (same physical device)
    QMap<QString, QStringList> deviceMap = buildMidiDeviceMap();

    // Create new device instances for each valid midi output
    QMap<QString, int> outputSeen;
    for (UINT id = 0; id < midiOutGetNumDevs(); id++)
    {
        QString name = extractOutputName(id);
        QVariant uid = makeUID(name, outputSeen, deviceMap);
        Win32MidiOutputDevice* dev = new Win32MidiOutputDevice(uid, name, id, this);
        m_outputDevices << dev;
    }

    // INPUT
    // Destroy existing inputs since there is no way of knowing if something has
    // disappeared or appeared in the middle between 0 - midiInGetNumDevs().
    while (m_inputDevices.isEmpty() == false)
        delete m_inputDevices.takeFirst();

    // Create new device instances for each valid midi input
    QMap<QString, int> inputSeen;
    for (UINT id = 0; id < midiInGetNumDevs(); id++)
    {
        QString name = extractInputName(id);
        QVariant uid = makeUID(name, inputSeen, deviceMap);
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
