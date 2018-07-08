/*
  Q Light Controller - Unit test
  mastertimer_stub.cpp

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

#include "mastertimer_stub.h"
#include "function.h"

/****************************************************************************
 * MasterTimer Stub
 ****************************************************************************/

MasterTimerStub::MasterTimerStub(Doc* doc, QList<Universe*> universes)
    : MasterTimer(doc)
    , m_universes(universes)
{
}

MasterTimerStub::~MasterTimerStub()
{
}

void MasterTimerStub::startFunction(Function* function)
{
    m_functionList.append(function);
    function->preRun(this);
}

void MasterTimerStub::stopFunction(Function* function)
{
    function->stop(FunctionParent::master());
    m_functionList.removeAll(function);
    function->postRun(this, m_universes);
}

void MasterTimerStub::registerDMXSource(DMXSource* source)
{
    m_dmxSourceList.append(source);
}

void MasterTimerStub::unregisterDMXSource(DMXSource* source)
{
    m_dmxSourceList.removeAll(source);
}
