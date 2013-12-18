/*
  Q Light Controller
  iodevice.cpp

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

#include "iodevice.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

IODevice::IODevice(const QVariant& uid, const QString& name, QObject* parent)
    : QObject(parent)
    , m_uid(uid)
    , m_name(name)
{
}

IODevice::~IODevice()
{
}

QVariant IODevice::uid() const
{
    return m_uid;
}

QString IODevice::name() const
{
    return m_name;
}
