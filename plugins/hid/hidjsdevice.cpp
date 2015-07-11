/*
  Q Light Controller
  hidjsdevice.cpp

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

#include <QApplication>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "hidjsdevice.h"
#include "hidplugin.h"

HIDJsDevice::HIDJsDevice(HIDPlugin* parent, quint32 line, struct hid_device_info *info)
    : HIDDevice(parent, line,
                QString::fromWCharArray(info->manufacturer_string) + " " +
                QString::fromWCharArray(info->product_string),
                QString(info->path))
{
    m_dev_info = (struct hid_device_info*) malloc (sizeof(struct hid_device_info));
    memcpy(m_dev_info, info, sizeof(struct hid_device_info));
    m_capabilities = QLCIOPlugin::Input;
}

HIDJsDevice::~HIDJsDevice()
{
    free(m_dev_info);
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDJsDevice::openInput()
{
    qDebug() << Q_FUNC_INFO;
    m_running = true;
    start();

    return true;
}

void HIDJsDevice::closeInput()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
    if (m_file.isOpen())
        m_file.close();
}

QString HIDJsDevice::path() const
{
    return m_file.fileName();
}

bool HIDJsDevice::readEvent()
{
    return false;
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDJsDevice::infoText()
{
    QString info;

    info += QString("<B>%1</B><P>").arg(m_name);
    info += tr("Axes: %1").arg(m_axesNumber);
    info += QString("<BR/>");
    info += tr("Buttons: %1").arg(m_buttonsNumber);
    info += QString("</P>");

    return info;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDJsDevice::feedBack(quint32 channel, uchar value)
{
    /* HID devices don't (yet) support feedback */
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void HIDJsDevice::run()
{
    while (m_running == true)
    {
        readEvent();
        msleep(50);
    }
}

