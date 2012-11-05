/*
  Q Light Controller
  hotplugmonitor.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QCoreApplication>
#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>

#include "hotplugmonitor.h"

#ifdef WIN32
#   include "hpmprivate-win32.h"
#elif __APPLE__
#   include "hpmprivate-iokit.h"
#else
#   include "hpmprivate-udev.h"
#endif

HotPlugMonitor* HotPlugMonitor::s_instance = NULL;

HotPlugMonitor::HotPlugMonitor(QObject* parent)
    : QObject(parent)
    , d_ptr(new HPMPrivate(this))
{
    qDebug() << Q_FUNC_INFO;
}

HotPlugMonitor::~HotPlugMonitor()
{
    qDebug() << Q_FUNC_INFO;

    stop();
    delete d_ptr;
    d_ptr = NULL;
}

void HotPlugMonitor::connectListener(QObject* listener)
{
    QByteArray added = QMetaObject::normalizedSignature("slotDeviceAdded(uint,uint)");
    QByteArray removed = QMetaObject::normalizedSignature("slotDeviceRemoved(uint,uint)");

    if (listener->metaObject()->indexOfMethod(added.constData()) != -1)
        connect(instance(), SIGNAL(deviceAdded(uint,uint)),
                listener, SLOT(slotDeviceAdded(uint,uint)));

    if (listener->metaObject()->indexOfMethod(removed.constData()) != -1)
        connect(instance(), SIGNAL(deviceRemoved(uint,uint)),
                listener, SLOT(slotDeviceRemoved(uint,uint)));
}

HotPlugMonitor* HotPlugMonitor::instance()
{
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
    qDebug() << Q_FUNC_INFO;
    d_ptr->start();
}

void HotPlugMonitor::stop()
{
    qDebug() << Q_FUNC_INFO;
    d_ptr->stop();
}

void HotPlugMonitor::emitDeviceAdded(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << vid << pid;
    emit deviceAdded(vid, pid);
}

void HotPlugMonitor::emitDeviceRemoved(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << vid << pid;
    emit deviceRemoved(vid, pid);
}
