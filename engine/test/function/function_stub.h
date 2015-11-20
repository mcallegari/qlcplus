/*
  Q Light Controller
  function_stub.h

  Copyright (C) Heikki Junnila

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

#ifndef FUNCTION_STUB_H
#define FUNCTION_STUB_H

#include "function.h"

class Doc;

class Function_Stub : public Function
{
    Q_OBJECT

public:
    Function_Stub(Doc* doc);
    ~Function_Stub();

    Function* createCopy(Doc* parent, bool addToDoc = true);

    bool saveXML(QXmlStreamWriter *doc);
    bool loadXML(QXmlStreamReader &root);

    void preRun(MasterTimer* timer);
    void write(MasterTimer* timer, QList<Universe*> universes);
    void postRun(MasterTimer* timer, QList<Universe*> universes);

public slots:
    void slotFixtureRemoved(quint32 id);

public:
    int m_preRunCalls;
    int m_writeCalls;
    int m_postRunCalls;

    quint32 m_slotFixtureRemovedId;
};

#endif

