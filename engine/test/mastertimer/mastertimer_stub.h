/*
  Q Light Controller - Unit test
  mastertimer_stub.h

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

#ifndef MASTERTIMER_STUB_H
#define MASTERTIMER_STUB_H

#include <QObject>

#include "mastertimer.h"
#include "dmxsource.h"

/****************************************************************************
 * MasterTimer Stub
 ****************************************************************************/

class MasterTimerStub : public MasterTimer
{
    Q_OBJECT

public:
    MasterTimerStub(Doc* doc, QList<Universe *> universes);
    ~MasterTimerStub();

    virtual void startFunction(Function* function);
    void stopFunction(Function* function);
    QList <Function*> m_functionList;

    void registerDMXSource(DMXSource* source);
    void unregisterDMXSource(DMXSource* source);
    QList <DMXSource*> m_dmxSourceList;

    QList<Universe*> m_universes;
};

#endif

