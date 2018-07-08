/*
  Q Light Controller
  function_stub.cpp

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

#include "doc.h"
#include "fixture.h"
#include "function_stub.h"

Function_Stub::Function_Stub(Doc* doc) : Function(doc, Function::Type(0xDEADBEEF))
{
    m_writeCalls = 0;
    m_preRunCalls = 0;
    m_postRunCalls = 0;
    m_slotFixtureRemovedId = Fixture::invalidId();
}

Function_Stub::~Function_Stub()
{
}

Function* Function_Stub::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Function_Stub(doc);
    Q_ASSERT(copy != NULL);

    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    else if (addToDoc == true && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Function_Stub::saveXML(QXmlStreamWriter *doc)
{
    Q_UNUSED(doc);

    return false;
}

bool Function_Stub::loadXML(QXmlStreamReader &root)
{
    Q_UNUSED(root);
    return false;
}

void Function_Stub::preRun(MasterTimer* timer)
{
    Q_UNUSED(timer);
    m_preRunCalls++;
    Function::preRun(timer);
}

void Function_Stub::write(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    incrementElapsed();
    m_writeCalls++;
}

void Function_Stub::postRun(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);
    m_postRunCalls++;
    Function::postRun(timer, universes);
}

void Function_Stub::slotFixtureRemoved(quint32 id)
{
    m_slotFixtureRemovedId = id;
    Function::slotFixtureRemoved(id);
}
