/*
  Q Light Controller
  mastertimer-win32.h

  Copyright (C) Heikki Junnila

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

#ifndef MASTERTIMER_PRIVATE_H
#define MASTERTIMER_PRIVATE_H

#include <Windows.h>

class MasterTimer;

class MasterTimerPrivate
{
public:
    MasterTimerPrivate(MasterTimer* masterTimer);
    ~MasterTimerPrivate();

    void start();
    void stop();
    bool isRunning() const;

    void timerTick();

private:
    MasterTimer* m_masterTimer;
    UINT m_systemTimerResolution;
    HANDLE m_phTimer;
    bool m_run;
};

#endif