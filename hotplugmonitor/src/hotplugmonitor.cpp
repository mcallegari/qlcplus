/*
  Q Light Controller
  hotplugmonitor.cpp

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

#include <QCoreApplication>
#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>

#include "hotplugmonitor.h"

#if defined(WIN32) || defined(Q_OS_WIN)
#   include "hpmprivate-win32.h"
#elif defined( __APPLE__) || defined(Q_OS_MAC)
#   include "hpmprivate-iokit.h"
#else
#   include "hpmprivate-udev.h"
#endif

HotPlugMonitor* HotPlugMonitor::s_instance = NULL;

HotPlugMonitor::HotPlugMonitor(QObject* parent)
    : QObject(parent)
    , d_ptr(new HPMPrivate(this))
{
    qWarning() << Q_FUNC_INFO;
}

HotPlugMonitor::~HotPlugMonitor()
{
    qWarning() << Q_FUNC_INFO;

    stop();
    delete d_ptr;
    d_ptr = NULL;
}

void HotPlugMonitor::connectListener(QObject* listener)
{
    qWarning() << Q_FUNC_INFO;

    QByteArray added = QMetaObject::normalizedSignature("slotDeviceAdded(uint,uint)");
    QByteArray removed = QMetaObject::normalizedSignature("slotDeviceRemoved(uint,uint)");

    if (listener->metaObject()->indexOfMethod(added.constData()) != -1)
    {
        qWarning() << Q_FUNC_INFO << "connect added";
        connect(instance(), SIGNAL(deviceAdded(uint,uint)),
                listener, SLOT(slotDeviceAdded(uint,uint)));
    }

    if (listener->metaObject()->indexOfMethod(removed.constData()) != -1)
    {
        qWarning() << Q_FUNC_INFO << "connect removed";
        connect(instance(), SIGNAL(deviceRemoved(uint,uint)),
                listener, SLOT(slotDeviceRemoved(uint,uint)));
    }
}

HotPlugMonitor* HotPlugMonitor::instance()
{
    qWarning() << Q_FUNC_INFO;

    if (s_instance == NULL)
    {
        Q_ASSERT(QCoreApplication::instance() != NULL);
        s_instance = new HotPlugMonitor(QCoreApplication::instance());
        s_instance->start();
    }

    return s_instance;
}

void HotPlugMonitor::start()
{
    qWarning() << Q_FUNC_INFO;
    d_ptr->start();
}

void HotPlugMonitor::stop()
{
    qWarning() << Q_FUNC_INFO;
    d_ptr->stop();
}

void HotPlugMonitor::emitDeviceAdded(uint vid, uint pid)
{
    qWarning() << Q_FUNC_INFO << vid << pid;
    emit deviceAdded(vid, pid);
}

void HotPlugMonitor::emitDeviceRemoved(uint vid, uint pid)
{
    qWarning() << Q_FUNC_INFO << vid << pid;
    emit deviceRemoved(vid, pid);
}
