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
    , m_clearRequest(false)
{
    Q_ASSERT(m_doc != NULL);
    m_doc->masterTimer()->registerDMXSource(this);
}

GenericDMXSource::~GenericDMXSource()
{
    m_doc->masterTimer()->unregisterDMXSource(this);
}

void GenericDMXSource::set(quint32 fxi, quint32 ch, uchar value)
{
    QMutexLocker locker(&m_mutex);
    m_values[QPair<quint32,quint32>(fxi, ch)] = value;
}

void GenericDMXSource::unset(quint32 fxi, quint32 ch)
{
    QMutexLocker locker(&m_mutex);
    m_values.remove(QPair<quint32,quint32>(fxi, ch));
}

void GenericDMXSource::unsetAll()
{
    QMutexLocker locker(&m_mutex);
    // will be processed at the next writeDMX
    m_clearRequest = true;
}

void GenericDMXSource::setOutputEnabled(bool enable)
{
    m_outputEnabled = enable;
}

bool GenericDMXSource::isOutputEnabled() const
{
    return m_outputEnabled;
}

quint32 GenericDMXSource::channelsCount() const
{
    return m_values.count();
}

QList<SceneValue> GenericDMXSource::channels()
{
    QList<SceneValue> chList;
    QMutableMapIterator <QPair<quint32,quint32>,uchar> it(m_values);
    while (it.hasNext() == true)
    {
        it.next();
        SceneValue sv;
        sv.fxi = it.key().first;
        sv.channel = it.key().second;
        sv.value = it.value();
        chList.append(sv);
    }
    return chList;
}

void GenericDMXSource::writeDMX(MasterTimer* timer, QList<Universe *> ua)
{
    Q_UNUSED(timer);

    QMutexLocker locker(&m_mutex);
    QMutableMapIterator <QPair<quint32,quint32>,uchar> it(m_values);
    while (it.hasNext() == true && m_outputEnabled == true)
    {
        it.next();

        FadeChannel fc(m_doc, it.key().first, it.key().second);

        QLCChannel::Group grp = fc.group(m_doc);
        quint32 address = fc.address();
        quint32 universe = fc.universe();

        if (address != QLCChannel::invalid())
            ua[universe]->write(address, it.value());
        if (grp != QLCChannel::Intensity)
            it.remove();
    }
    if (m_clearRequest)
    {
        m_clearRequest = false;
        m_values.clear();
    }
}
