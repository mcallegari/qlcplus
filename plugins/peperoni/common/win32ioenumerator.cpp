/*
  Q Light Controller
  win32ioenumerator.cpp

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

#include <Windows.h>
#include <QDebug>

#include "win32peperonidevice.h"
#include "win32ioenumerator.h"
#include "usbdmx-dynamic.h"
#include "peperonidefs.h"

#define MAX_USBDMX_DEVICES 16

Win32IOEnumerator::Win32IOEnumerator(QObject* parent)
    : IOEnumerator(parent)
    , m_usbdmx(NULL)
{
    qDebug() << Q_FUNC_INFO;

    m_usbdmx = usbdmx_init();
    if (m_usbdmx == NULL)
    {
        qWarning() << "Loading USBDMX.DLL failed.";
    }
    else if (USBDMX_DLL_VERSION_CHECK(m_usbdmx) == FALSE)
    {
        /* verify USBDMX dll version */
        qWarning() << "USBDMX.DLL version does not match. Abort.";
        qWarning() << "Found" << m_usbdmx->version() << "but expected"
                   << USBDMX_DLL_VERSION;
        m_usbdmx = NULL;
    }
    else
    {
        qDebug() << "Using USBDMX.DLL version" << m_usbdmx->version();
        rescan();
    }
}

Win32IOEnumerator::~Win32IOEnumerator()
{
    qDebug() << Q_FUNC_INFO;

    while (m_outputDevices.isEmpty() == false)
        delete m_outputDevices.takeFirst();
}

QVariant Win32IOEnumerator::extractUID(HANDLE handle)
{
    Q_ASSERT(handle != NULL);
    USHORT uid = 0;
    if (m_usbdmx->device_id(handle, &uid) == TRUE)
        return QVariant(uid);
    else
        return QVariant();
}

QString Win32IOEnumerator::extractName(HANDLE handle)
{
    QString name;
    if (m_usbdmx->is_xswitch(handle) == TRUE)
        name = QString("X-Switch");
    else if (m_usbdmx->is_rodin1(handle) == TRUE)
        name = QString("Rodin 1");
    else if (m_usbdmx->is_rodin2(handle) == TRUE)
        name = QString("Rodin 2");
    else if (m_usbdmx->is_rodint(handle) == TRUE)
        name = QString("Rodin T");
    else if (m_usbdmx->is_usbdmx21(handle) == TRUE)
        name = QString("USBDMX21");
    else
        name = QString("?");

    return name;
}

void Win32IOEnumerator::rescan()
{
    if (m_usbdmx == NULL)
        return;

    QList <OutputDevice*> destroyOutputs(m_outputDevices);
    bool changed = false;

    for (USHORT id = 0; id < MAX_USBDMX_DEVICES; id++)
    {
        HANDLE handle = NULL;
        if (m_usbdmx->open(id, &handle) == TRUE)
        {
            QVariant uid = extractUID(handle);
            QString name = extractName(handle);

            // We don't need the handle anymore for now
            m_usbdmx->close(handle);
            handle = NULL;

            OutputDevice* od = outputDevice(uid);
            if (od == NULL)
            {
                // New device
                od = new Win32PeperoniDevice(uid, name, id, m_usbdmx, this);
                m_outputDevices << od;
                changed = true;
            }
            else
            {
                // We know this device and it's still present
                destroyOutputs.removeAll(od);
            }
        }
        else
        {
            /* This device ID doesn't exist and neither does any
               consecutive id, so we can stop looking. */
            break;
        }
    }

    while (destroyOutputs.isEmpty() == false)
    {
        delete destroyOutputs.takeFirst();
        changed = true;
    }

    if (changed == true)
        emit configurationChanged();
}

QList <OutputDevice*> Win32IOEnumerator::outputDevices() const
{
    return m_outputDevices;
}

QList <InputDevice*> Win32IOEnumerator::inputDevices() const
{
    return m_inputDevices;
}

OutputDevice* Win32IOEnumerator::outputDevice(const QVariant& uid) const
{
    QListIterator <OutputDevice*> it(m_outputDevices);
    while (it.hasNext() == true)
    {
        OutputDevice* dev(it.next());
        if (dev->uid() == uid)
            return dev;
    }

    return NULL;
}

InputDevice* Win32IOEnumerator::inputDevice(const QVariant& uid) const
{
    Q_UNUSED(uid);
    return NULL;
}
