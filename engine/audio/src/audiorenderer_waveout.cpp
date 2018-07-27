/*
  Q Light Controller Plus
  audiorenderer_waveout.cpp

  Copyright (c) Massimo Callegari

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

/******************************************************
 *   Based on qmmp project                            *
 *                                                    *
 *   Copyright (C) 2007-2009 by Ilya Kotov            *
 *   forkotov02@hotmail.ru                            *
 ******************************************************/

#include <QtGlobal>
#include <QSettings>

#include <stdio.h>
#include <string.h>
#include <iostream>

#include "audiorenderer_waveout.h"

#define MAX_WAVEBLOCKS    256

static CRITICAL_SECTION  cs;
static HWAVEOUT          dev                    = NULL;
static unsigned int      ScheduledBlocks        = 0;
static int               PlayedWaveHeadersCount = 0;          // free index
static WAVEHDR*          PlayedWaveHeaders [MAX_WAVEBLOCKS];


static void CALLBACK wave_callback (HWAVE hWave, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    Q_UNUSED(hWave)
    Q_UNUSED(dwInstance)
    Q_UNUSED(dwParam2)

    if (uMsg == WOM_DONE)
    {
        EnterCriticalSection (&cs);
        PlayedWaveHeaders [PlayedWaveHeadersCount++] = (WAVEHDR*) dwParam1;
        LeaveCriticalSection (&cs);
    }
}

static void
free_memory (void)
{
    WAVEHDR*  wh;
    HGLOBAL   hg;

    EnterCriticalSection (&cs);
    wh = PlayedWaveHeaders [--PlayedWaveHeadersCount];
    ScheduledBlocks--;                        // decrease the number of USED blocks
    LeaveCriticalSection (&cs);

    waveOutUnprepareHeader (dev, wh, sizeof (WAVEHDR));

    hg = GlobalHandle (wh -> lpData);       // Deallocate the buffer memory
    GlobalUnlock (hg);
    GlobalFree   (hg);

    hg = GlobalHandle (wh);                 // Deallocate the header memory
    GlobalUnlock (hg);
    GlobalFree   (hg);
}

AudioRendererWaveOut::AudioRendererWaveOut(QString device, QObject * parent)
	: AudioRenderer(parent)
{
    deviceID = WAVE_MAPPER;
    if (device.isEmpty())
    {
        QSettings settings;
        QVariant var = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE);
        if (var.isValid() == true)
            deviceID = QString(var.toString()).toUInt();
    }
    else
        deviceID = device.toUInt();
}

AudioRendererWaveOut::~AudioRendererWaveOut()
{
    uninitialize();
}

bool AudioRendererWaveOut::initialize(quint32 freq, int chan, AudioFormat format)
{
    Q_UNUSED(format)

    if (!waveOutGetNumDevs ())
    {
        qWarning("AudioRendererWaveOut: no audio device found");
        return false;
    }
    WAVEFORMATEX fmt;

    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.wBitsPerSample  = 16;
    fmt.nChannels       = chan;
    fmt.nSamplesPerSec  = (unsigned long)(freq);
    fmt.nBlockAlign     = fmt.nChannels * fmt.wBitsPerSample/8;
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nChannels * fmt.wBitsPerSample/8;

    switch (waveOutOpen (&dev, deviceID, &fmt, (DWORD)wave_callback, 0, CALLBACK_FUNCTION))
    {
    case MMSYSERR_ALLOCATED:
        qWarning("AudioRendererWaveOut: Device is already open.");
        return false;
    case MMSYSERR_BADDEVICEID:
        qWarning("AudioRendererWaveOut: The specified device is out of range.");
        return false;
    case MMSYSERR_NODRIVER:
        qWarning("AudioRendererWaveOut: There is no audio driver in this system.");
        return false;
    case MMSYSERR_NOMEM:
        qWarning("AudioRendererWaveOut: Unable to allocate sound memory.");
        return false;
    case WAVERR_BADFORMAT:
        qWarning("AudioRendererWaveOut: This audio format is not supported.");
        return false;
    case WAVERR_SYNC:
        qWarning("AudioRendererWaveOut: The device is synchronous.");
        return false;
    default:
        qWarning("AudioRendererWaveOut: Unknown media error.");
        return false;
    case MMSYSERR_NOERROR:
        break;
    }

    waveOutReset (dev);
    InitializeCriticalSection (&cs);

    return true;
}


qint64 AudioRendererWaveOut::latency()
{
    return 0;
}

QList<AudioDeviceInfo> AudioRendererWaveOut::getDevicesInfo()
{
    QList<AudioDeviceInfo> devList;

    WAVEINCAPS wic;
    WAVEOUTCAPS woc;
    unsigned long iNumDevs, i;

    /* Get the number of Digital Audio In devices in this computer */
    iNumDevs = waveInGetNumDevs();

    /* Go through all of those devices, displaying their names */
    for (i = 0; i < iNumDevs; i++)
    {
        /* Get info about the next device */
        if (!waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS)))
        {
            /* Display its Device ID and name */
            //printf("Device ID #%u: %s\r\n", i, wic.szPname);
            AudioDeviceInfo info;
            info.deviceName = QString((const QChar *)wic.szPname);
            info.privateName = QString::number(i);
            info.capabilities = AUDIO_CAP_INPUT;
            devList.append(info);
        }
    }

    /* Get the number of Digital Audio Out devices in this computer */
    iNumDevs = waveOutGetNumDevs();

    /* Go through all of those devices, displaying their names */
    for (i = 0; i < iNumDevs; i++)
    {
        /* Get info about the next device */
        if (!waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS)))
        {
            /* Display its Device ID and name */
            //printf("Device ID #%u: %s\r\n", i, woc.szPname);
            AudioDeviceInfo info;
            info.deviceName = QString((const QChar *)woc.szPname);
            info.privateName = QString::number(i);
            info.capabilities = AUDIO_CAP_OUTPUT;
            devList.append(info);
        }
    }

    return devList;
}

qint64 AudioRendererWaveOut::writeAudio(unsigned char *data, qint64 len)
{
    HGLOBAL    hg;
    HGLOBAL    hg2;
    LPWAVEHDR  wh;
    void*      allocptr;
    len = qMin(len, (qint64)1024);


    while (PlayedWaveHeadersCount > 0) // free used blocks ...
        free_memory ();

    if (ScheduledBlocks >= sizeof(PlayedWaveHeaders)/sizeof(*PlayedWaveHeaders)) // wait for a free block ...
    {
        usleep(500);
        return 0;
    }

    if ((hg2 = GlobalAlloc (GMEM_MOVEABLE, len)) == NULL)   // allocate some memory for a copy of the buffer
    {
        qWarning("AudioRendererWaveOut: GlobalAlloc failed");
        return 0;
    }

    allocptr = GlobalLock (hg2);
    CopyMemory (allocptr, data, len);                         // Here we can call any modification output functions we want....

    if ((hg = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (WAVEHDR))) == NULL) // now make a header and WRITE IT!
        return -1;

    wh                   = (wavehdr_tag*)GlobalLock (hg);
    wh->dwBufferLength   = len;
    wh->lpData           = (CHAR *)allocptr;

    if (waveOutPrepareHeader (dev, wh, sizeof (WAVEHDR)) != MMSYSERR_NOERROR)
    {
        GlobalUnlock (hg);
        GlobalFree   (hg);
        return -1;
    }

    if (waveOutWrite (dev, wh, sizeof (WAVEHDR)) != MMSYSERR_NOERROR)
    {
        GlobalUnlock (hg);
        GlobalFree   (hg);
        return -1;
    }

    EnterCriticalSection (&cs);
    ScheduledBlocks++;
    LeaveCriticalSection (&cs);

    return len;
}

void AudioRendererWaveOut::drain()
{
    while (ScheduledBlocks > 0)
    {
        Sleep(ScheduledBlocks);
        while (PlayedWaveHeadersCount > 0)                        // free used blocks ...
            free_memory();
    }
}

void AudioRendererWaveOut::suspend()
{
    waveOutPause(dev);
}

void AudioRendererWaveOut::resume()
{
    waveOutRestart(dev);
}

void AudioRendererWaveOut::reset()
{
   while (PlayedWaveHeadersCount > 0)                        // free used blocks ...
      free_memory ();
   waveOutReset (dev);
}

void AudioRendererWaveOut::uninitialize()
{
    if (dev)
    {
        while (ScheduledBlocks > 0)
        {
            Sleep (ScheduledBlocks);
            while (PlayedWaveHeadersCount > 0)                        // free used blocks ...
                free_memory ();
        }

        waveOutReset (dev);      // reset the device
        waveOutClose (dev);      // close the device
        dev = 0;
    }

    DeleteCriticalSection (&cs);
    ScheduledBlocks = 0;
    return;
}
