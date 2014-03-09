/*
  Q Light Controller
  hiddevice.cpp

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

#include <QString>

#include "hiddevice.h"
#include "hid.h"

HIDDevice::HIDDevice(HID* parent, quint32 line, const QString &name, const QString& path)
    : QThread(parent)
{
    m_name = QString("%1: %2").arg(line + 1).arg(name);
    m_file.setFileName(path);
    m_line = line;
    m_running = false;
}

HIDDevice::~HIDDevice()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
    closeInput();
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDDevice::openInput()
{
    return false;
}

void HIDDevice::closeInput()
{
}

void HIDDevice::openOutput()
{
}

void HIDDevice::closeOutput()
{
}

QString HIDDevice::path() const
{
    return QString();
}

int HIDDevice::handle() const
{
    return m_file.handle();
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDDevice::infoText()
{
    return QString();
}

QString HIDDevice::name()
{
    return m_name;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDDevice::feedBack(quint32 channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void HIDDevice::run()
{
}

void HIDDevice::outputDMX(const QByteArray &data, bool forceWrite)
{
    Q_UNUSED(data);
    Q_UNUSED(forceWrite);
}
