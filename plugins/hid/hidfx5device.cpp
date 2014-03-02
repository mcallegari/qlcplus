/*
  Q Light Controller Plus
  hidfx5device.cpp

  Copyright (c) Massimo Callegari
                Jeija Norrepli

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

#include <linux/input.h>
#include <errno.h>
#if !defined(WIN32) && !defined(Q_OS_WIN)
  #include <unistd.h>
#endif

#include <QApplication>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "hidfx5device.h"
#include "qlcmacros.h"
#include "hid.h"

HIDFX5Device::HIDFX5Device(HID* parent, quint32 line, const QString& path)
    : HIDDevice(parent, line, path)
{
    init();
}

HIDFX5Device::~HIDFX5Device()
{
    qobject_cast<HID*> (parent())->removePollDevice(this);
}

void HIDFX5Device::init()
{
    if (open() == false)
        return;

    m_name = QString("FX5 Output %1: %2").arg(m_line + 1)
             .arg(strerror(errno));

    /* Device name */
/*
    char name[128] = "Unknown";
    if (ioctl(m_file.handle(), JSIOCGNAME(sizeof(name)), name) < 0)
    {
        m_name = QString("FX5 Output %1: %2").arg(m_line + 1)
                 .arg(strerror(errno));
        qWarning() << "Unable to get FX5 name:"
                   << strerror(errno);
    }
    else
    {
        m_name = QString("FX5 Output %1: %2").arg(m_line + 1).arg(name);
    }
*/

    close();
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDFX5Device::open()
{
    bool result = false;

    result = m_file.open(QIODevice::Unbuffered | QIODevice::ReadWrite);
    if (result == false)
    {
        result = m_file.open(QIODevice::Unbuffered |
                             QIODevice::ReadOnly);
        if (result == false)
        {
            qWarning() << "Unable to open" << m_file.fileName()
                       << ":" << m_file.errorString();
        }
        else
        {
            qDebug() << "Opened" << m_file.fileName()
                     << "in read only mode";
        }
    }

    return result;
}

void HIDFX5Device::close()
{
    m_file.close();
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

