/*
  Q Light Controller Plus
  hiddmxdevice.cpp

  Copyright (c) Massimo Callegari
                Florian Euchner
                Stefan Krupop

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

#include <errno.h>
#if !defined(WIN32) && !defined(Q_OS_WIN)
  #include <unistd.h>
#endif

#include <QApplication>
#include <QMessageBox>
#include <QByteArray>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "hiddmxdevice.h"
#include "qlcmacros.h"
#include "hidapi.h"
#include "hidplugin.h"

HIDDMXDevice::HIDDMXDevice(HIDPlugin* parent, quint32 line, const QString &name, const QString& path)
    : HIDDevice(parent, line, name, path)
{
    m_capabilities = QLCIOPlugin::Output;
    m_mode = DMX_MODE_NONE;
    init();
}

HIDDMXDevice::~HIDDMXDevice()
{
    closeInput();
    closeOutput();
    hid_close(m_handle);
}

void HIDDMXDevice::init()
{
    /* Device name */
    m_handle = hid_open_path(path().toUtf8().constData());

    if (!m_handle)
    {
        QMessageBox::warning(NULL, (tr("HID DMX Interface Error")),
            (tr("Unable to open %1. Make sure the udev rule is installed.").arg(name())),
             QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    /** Reset channels when opening the interface: */
    m_dmx_cmp.fill(0, 512);
    m_dmx_in_cmp.fill(0, 512);
    outputDMX(m_dmx_cmp, true);
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDDMXDevice::isMergerModeEnabled()
{
    return (m_mode & DMX_MODE_MERGER);
}

void HIDDMXDevice::enableMergerMode(bool mergerModeEnabled)
{
    if (mergerModeEnabled)
        m_mode |= DMX_MODE_MERGER;
    else
        m_mode &= ~DMX_MODE_MERGER;
    updateMode();
}

bool HIDDMXDevice::openInput()
{
    m_mode |= DMX_MODE_INPUT;
    updateMode();

    return true;
}

void HIDDMXDevice::closeInput()
{
    m_mode &= ~DMX_MODE_INPUT;
    updateMode();
}

bool HIDDMXDevice::openOutput()
{
    m_mode |= DMX_MODE_OUTPUT;
    updateMode();

    return true;
}

void HIDDMXDevice::closeOutput()
{
    m_mode &= ~DMX_MODE_OUTPUT;
    updateMode();
}

bool HIDDMXDevice::readEvent()
{
    return true;
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDDMXDevice::infoText()
{
    QString info;

    info += QString("<H3>%1</H3><P>").arg(m_name);

    return info;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDDMXDevice::feedBack(quint32 channel, uchar value)
{
    /* HID devices don't support feedback (yet) */
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void HIDDMXDevice::run()
{
    while (m_running == true)
    {
        unsigned char buffer[35];
        int size;

        size = hid_read_timeout(m_handle, buffer, 33, HID_DMX_READ_TIMEOUT);

        /**
        * Protocol: 33 bytes in buffer[33]
        * [0]      = chunk, which is the offset by which the channel is calculated
        *            from, the nth chunk starts at address n * 32
        * [1]-[32] = channel values, where the nth value is the offset + n
        */
        while (size > 0)
        {
            if (size == 33)
            {
                unsigned short startOff = buffer[0] * 32;
                if (buffer[0] < 16)
                {
                    for (int i = 0; i < 32; i++)
                    {
                        unsigned short channel = startOff + i;
                        unsigned char value = buffer[i + 1];
                        if ((unsigned char)m_dmx_in_cmp.at(channel) != value)
                        {
                            emit valueChanged(UINT_MAX, m_line, channel, value);
                            m_dmx_in_cmp[channel] = value;
                        }
                    }
                }
            }

            size = hid_read_timeout(m_handle, buffer, 33, HID_DMX_READ_TIMEOUT);
        }
    }
}

/*****************************************************************************
 * Output data
 *****************************************************************************/

void HIDDMXDevice::outputDMX(const QByteArray &universe, bool forceWrite)
{
    for (int i = 0; i < 16; i++)
    {
        int startOff = i * 32;
        if (startOff >= universe.size())
            return;

        QByteArray chunk = universe.mid(startOff, 32);
        if (chunk.size() < 32)
            chunk.append(QByteArray(32 - chunk.size(), (char)0x0));

        if (forceWrite == true || chunk != m_dmx_cmp.mid(startOff, 32))
        {
            /** Save different data to m_dmx_cmp */
            m_dmx_cmp.replace(startOff, 32, chunk);

            chunk.prepend((char)i);
            chunk.prepend((char)0x0);

            /** Output new data */
            hid_write(m_handle, (const unsigned char *)chunk.data(), chunk.size());
        }
    }
}

/*****************************************************************************
 * HID DMX - specific functions / driver
 *****************************************************************************/

void HIDDMXDevice::updateMode()
{
    /**
    *  Send chosen mode to the HID DMX device
    */
    unsigned char driver_mode = 0;
    if (m_mode & DMX_MODE_OUTPUT)
        driver_mode += 2;
    if (m_mode & DMX_MODE_INPUT)
        driver_mode += 4;
    if (m_mode & DMX_MODE_MERGER)
        driver_mode += 1;

    unsigned char buffer[34];

    memset(buffer, 0, 34);
    buffer[1] = 16;
    buffer[2] = driver_mode;

    hid_write(m_handle, buffer, 34);

    /**
    *  Start / stop input polling thread based on whether the input is activated
    */
    if (m_mode & DMX_MODE_INPUT)
    {
        m_running = true;
        start();
    }
    else if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}
