/*
  Q Light Controller
  win32ioenumerator.h

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

#ifndef WIN32IOENUMERATOR_H
#define WIN32IOENUMERATOR_H

#include <Windows.h>
#include <QVariant>
#include <QObject>

#include "ioenumerator.h"

struct usbdmx_functions;

class Win32IOEnumerator : public IOEnumerator
{
    Q_OBJECT

public:
    Win32IOEnumerator(QObject* parent = 0);
    ~Win32IOEnumerator();

    void rescan();

    QVariant extractUID(HANDLE handle);
    QString extractName(HANDLE handle);

    QList <OutputDevice*> outputDevices() const;
    QList <InputDevice*> inputDevices() const;

    OutputDevice* outputDevice(const QVariant& uid) const;
    InputDevice* inputDevice(const QVariant& uid) const;

signals:
    void configurationChanged();

private:
    struct usbdmx_functions* m_usbdmx;
    QList <OutputDevice*> m_outputDevices;
    QList <InputDevice*> m_inputDevices;
};

#endif
