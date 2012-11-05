/*
  Q Light Controller
  unixioenumerator.h

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

#ifndef UNIXIOENUMERATOR_H
#define UNIXIOENUMERATOR_H

#include <QVariant>
#include <QObject>
#include "ioenumerator.h"

struct libusb_device;

class UnixIOEnumerator : public IOEnumerator
{
    Q_OBJECT

public:
    UnixIOEnumerator(QObject* parent = 0);
    ~UnixIOEnumerator();

    void rescan();

    QList <OutputDevice*> outputDevices() const;
    QList <InputDevice*> inputDevices() const;

    OutputDevice* outputDevice(const QVariant& uid) const;
    InputDevice* inputDevice(const QVariant& uid) const;

signals:
    void configurationChanged();

private:
    void extractData(struct libusb_device* device,
                     struct libusb_device_descriptor* desc,
                     QVariant* uid, QString* name);

    static bool isPeperoniDevice(const struct libusb_device_descriptor* descriptor);

private:
    struct libusb_context* m_ctx;
    QList <OutputDevice*> m_outputDevices;
    QList <InputDevice*> m_inputDevices;
};

#endif
