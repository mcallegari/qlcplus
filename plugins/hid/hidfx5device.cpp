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

// TODO: If error messagebox is shown while opening devices, rescan & update list
// TODO: The whole input polling stuff
// TODO: Use res in case of error in void HIDFX5Device::outputDMX(const QByteArray &universe)

#include <linux/input.h>
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
#include "hid.h"

HIDFX5Device::HIDFX5Device(HID* parent, quint32 line, const QString &name, const QString& path)
    : HIDDevice(parent, line, name, path)
{
    m_capabilities = QLCIOPlugin::Output;
    init();
}

HIDFX5Device::~HIDFX5Device()
{
    closeInput();
    closeOutput();
    hid_close(m_handle);
    //qobject_cast<HID*> (parent())->removePollDevice(this); // TODO
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

void HIDFX5Device::openOutput()
{
    m_mode |= FX5_MODE_OUTPUT;
    updateMode();
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
            chunk.append((char)0x0, 32 - chunk.size());
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
    int driver_mode = 0;
    if (m_mode & FX5_MODE_OUTPUT)
        driver_mode += 2;
    if (m_mode & FX5_MODE_INPUT)
        driver_mode += 4;

    unsigned char buffer[34];

    memset(buffer, 0, 34);
    buffer[1] = 16;
    buffer[2] = driver_mode;
    hid_write(m_handle, buffer, 34);
}
