/*
  Q Light Controller
  win32peperonidevice.h

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
