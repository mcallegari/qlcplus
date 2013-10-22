/*
  Q Light Controller
  velleman_mock.cpp

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

#include <stdlib.h>
#if defined(WIN32) || defined(Q_OS_WIN)
#       include <stdint.h> // int32_t
#endif

extern "C"
{
    int _StartDeviceCalled = 0;
    void StartDevice()
    {
        _StartDeviceCalled++;
    }

    int _StopDeviceCalled = 0;
    void StopDevice()
    {
        _StopDeviceCalled++;
    }

    int _ChannelCount = 0;
    void SetChannelCount(int32_t Count)
    {
        _ChannelCount = Count;
    }

    int* _SetAllData = NULL;
    void SetAllData(int32_t Data[])
    {
        _SetAllData = Data;
    }
}
