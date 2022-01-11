/*
  Q Light Controller
  velleman_mock.cpp

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

#include <stdlib.h>
#include <stdint.h> // int32_t

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
