/*
  Q Light Controller
  genericdmxsource.cpp

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

#include "genericdmxsource.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "qlcchannel.h"
#include "universe.h"
#include "doc.h"

GenericDMXSource::GenericDMXSource(Doc* doc)
    : m_doc(doc)
    , m_outputEnabled(false)
{
    Q_ASSERT(m_doc != NULL);
    m_doc->masterTimer()->registerDMXSource(this);
}

GenericDMXSource::~GenericDMXSource()
{
    m_mutex.lock();
    m_doc->masterTimer()->unregisterDMXSource(this);
    m_mutex.unlock();
}

void GenericDMXSource::set(quint32 fxi, quint32 ch, uchar value)
{
    m_mutex.lock();
    m_values[QPair<quint32,quint32>(fxi, ch)] = value;
    m_mutex.unlock();
}

void GenericDMXSource::unset(quint32 fxi, quint32 ch)
{
    m_mutex.lock();
    m_values.remove(QPair<quint32,quint32>(fxi, ch));
    m_mutex.unlock();
}

void GenericDMXSource::setOutputEnabled(bool enable)
{
    m_outputEnabled = enable;
}

bool GenericDMXSource::isOutputEnabled() const
{
    return m_outputEnabled;
}

void GenericDMXSource::writeDMX(MasterTimer* timer, QList<Universe *> ua)
{
    Q_UNUSED(timer);

    m_mutex.lock();
    QMutableMapIterator <QPair<quint32,quint32>,uchar> it(m_values);
    while (it.hasNext() == true && m_outputEnabled == true)
    {
        it.next();

        FadeChannel fc;
        fc.setFixture(m_doc, it.key().first);
        fc.setChannel(it.key().second);

        QLCChannel::Group grp = fc.group(m_doc);
        quint32 address = fc.address();
        quint32 universe = fc.universe();

        if (address != QLCChannel::invalid())
            ua[universe]->write(address, it.value());
        if (grp != QLCChannel::Intensity)
            it.remove();
    }
    m_mutex.unlock();
}
