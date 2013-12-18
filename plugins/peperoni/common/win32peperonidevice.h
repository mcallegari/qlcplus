/*
  Q Light Controller
  win32peperonidevice.h

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

#ifndef WIN32PEPERONIDEVICE_H
#define WIN32PEPERONIDEVICE_H

#include <Windows.h>
#include <QObject>

#include "outputdevice.h"

struct usbdmx_functions;

class Win32PeperoniDevice : public OutputDevice
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    Win32PeperoniDevice(const QVariant& uid, const QString& name, USHORT id,
                        struct usbdmx_functions* usbdmx,
                        QObject* parent = 0);
    virtual ~Win32PeperoniDevice();

    /** @reimp */
    void open();

    /** @reimp */
    void close();

    /** @reimp */
    bool isOpen() const;

    /** @reimp */
    void writeChannel(ushort channel, uchar value);

    /** @reimp */
    void writeUniverse(const QByteArray& universe);

private:
    struct usbdmx_functions* m_usbdmx;
    USHORT m_id;
    HANDLE m_handle;
};

#endif
