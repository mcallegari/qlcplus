/*
  Q Light Controller - Unit test
  dmxsource_stub.h

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

#ifndef DMXSOURCE_STUB_H
#define DMXSOURCE_STUB_H

#include <QObject>
#include "dmxsource.h"

class DMXSource_Stub : public DMXSource
{
public:
    DMXSource_Stub();
    ~DMXSource_Stub();

    void writeDMX(MasterTimer* timer, QList<Universe*> universes);

    /** Number of calls to writeDMX() */
    int m_writeCalls;
};

#endif
