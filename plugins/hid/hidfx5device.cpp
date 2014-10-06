/*
  Q Light Controller Plus
  hidfx5device.cpp

  Copyright (c) Massimo Callegari
                Florian Euchner

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

#include "hidfx5device.h"
#include "qlcmacros.h"
#include "hidapi.h"
#include "hidplugin.h"

HIDFX5Device::HIDFX5Device(HIDPlugin* parent, quint32 line, const QString &name, const QString& path)
    : HIDDevice(parent, line, name, path)
{
    m_capabilities = QLCIOPlugin::Output;
    m_mode = FX5_MODE_NONE;
    init();
}

HIDFX5Device::~HIDFX5Device()
{
    closeInput();
    closeOutput();
    hid_close(m_handle);
}

void HIDFX5Device::init()
{
    /* Device name */
    m_handle = hid_open_path(path().toUtf8().constData());
    
    if (!m_handle)
    {
        QMessageBox::warning(NULL, (tr("FX5 USB DMX Interface Error")),
            (tr("Unable to open the FX5 Interface. Make sure the udev rule is installed.")),
             QMessageBox::AcceptRole, QMessageBox::AcceptRole);
    }

    /** Reset channels when opening the interface: */
    m_dmx_cmp.fill(0, 512);
    m_dmx_in_cmp.fill(0, 512);
    outputDMX(m_dmx_cmp, true);
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDFX5Device::openInput()
{
    m_mode |= FX5_MODE_INPUT;
    updateMode();
    
    return true;
}

void HIDFX5Device::closeInput()
{
    m_mode &= ~FX5_MODE_INPUT;
    updateMode();
}

bool HIDFX5Device::openOutput()
{
    m_mode |= FX5_MODE_OUTPUT;
    updateMode();

    return true;
}

void HIDFX5Device::closeOutput()
{
    m_mode &= ~FX5_MODE_OUTPUT;
    updateMode();
}

QString HIDFX5Device::path() const
{
    return m_file.fileName();
}

bool HIDFX5Device::readEvent()
{
    return true;
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDFX5Device::infoText()
{
    QString info;

    info += QString("<B>%1</B><P>").arg(m_name);

    return info;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDFX5Device::feedBack(quint32 channel, uchar value)
{
    /* HID devices don't (yet) support feedback */
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void HIDFX5Device::run()
{
    while(m_running == true)
    {
        unsigned char buffer[35];
        int size;

        size = hid_read_timeout(m_handle, buffer, 33, FX5_READ_TIMEOUT);

        /**
        * Protocol: 33 bytes in buffer[33]
        * [0]      = chunk, which is the offset by which the channel is calculated
        *            from, the nth chunk starts at address n * 32
        * [1]-[32] = channel values, where the nth value is the offset + n
        */
        while(size > 0)
        {
            if(size == 33)
            {
                unsigned short startOff = buffer[0] * 32;
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

            size = hid_read_timeout(m_handle, buffer, 33, FX5_READ_TIMEOUT);
        }
    }
}

/*****************************************************************************
 * Output data
 *****************************************************************************/

void HIDFX5Device::outputDMX(const QByteArray &universe, bool forceWrite)
{
    for (int i = 0; i < 16; i++)
    {
        int startOff = i * 32;
        if (startOff >= universe.size())
            return;
        QByteArray chunk = universe.mid(startOff, 32);
        if (chunk.size() < 32)
            chunk.append(QByteArray(32 - chunk.size(), (char)0x0));
        if(forceWrite == true || chunk != m_dmx_cmp.mid(startOff, 32))
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
 * FX5 - specific functions / driver
 *****************************************************************************/

void HIDFX5Device::updateMode()
{
    /**
    *  Send chosen mode to the FX5 / DE device
    */
    unsigned char driver_mode = 0;
    if (m_mode & FX5_MODE_OUTPUT)
        driver_mode += 2;
    if (m_mode & FX5_MODE_INPUT)
        driver_mode += 4;

    unsigned char buffer[34];

    memset(buffer, 0, 34);
    buffer[1] = 16;
    buffer[2] = driver_mode;

    hid_write(m_handle, buffer, 34);

    /**
    *  Start / stop input polling thread based on whether the input is activated
    */
    if (m_mode & FX5_MODE_INPUT)
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
