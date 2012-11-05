/*
  Q Light Controller
  ioenumerator.h

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

#ifndef IOENUMERATOR_H
#define IOENUMERATOR_H

#include <QObject>
#include <QList>

class OutputDevice;
class InputDevice;

class IOEnumerator : public QObject
{
    Q_OBJECT

public:
    IOEnumerator(QObject* parent = 0);
    virtual ~IOEnumerator();

    virtual void rescan() = 0;

    virtual QList <OutputDevice*> outputDevices() const = 0;
    virtual QList <InputDevice*> inputDevices() const = 0;

    virtual OutputDevice* outputDevice(const QVariant& uid) const = 0;
    virtual InputDevice* inputDevice(const QVariant& uid) const = 0;

signals:
    void configurationChanged();
};

#endif
