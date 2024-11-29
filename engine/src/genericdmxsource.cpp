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
#include "genericfader.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "universe.h"
#include "doc.h"

#include <cmath>

GenericDMXSource::GenericDMXSource(Doc* doc)
    : m_doc(doc)
    , m_outputEnabled(false)
    , m_clearRequest(false)
    , m_changed(false)
{
    Q_ASSERT(m_doc != NULL);
    m_doc->masterTimer()->registerDMXSource(this);
}

GenericDMXSource::~GenericDMXSource()
{
    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->requestDelete();
    }
    m_fadersMap.clear();

    m_doc->masterTimer()->unregisterDMXSource(this);
}

void GenericDMXSource::set(quint32 fxi, quint32 ch, uchar value)
{
    QMutexLocker locker(&m_mutex);
    m_values[QPair<quint32,quint32>(fxi, ch)] = value;
    m_changed = true;
}

void GenericDMXSource::unset(quint32 fxi, quint32 ch)
{
    QMutexLocker locker(&m_mutex);
    m_values.remove(QPair<quint32,quint32>(fxi, ch));
    m_changed = true;
}

void GenericDMXSource::unsetAll()
{
    QMutexLocker locker(&m_mutex);
    // will be processed at the next writeDMX
    m_clearRequest = true;
    m_changed = true;
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

    if (m_outputEnabled && m_changed)
    {

        QMutableMapIterator <QPair<quint32,quint32>,uchar> it(m_values);
        while (it.hasNext())
        {
            it.next();
            Fixture *fixture = m_doc->fixture(it.key().first);
            if (fixture == NULL)
                continue;

            quint32 channelIndex = it.key().second;
            int universeIndex = floor((fixture->universeAddress() + channelIndex) / 512);

            if (universeIndex >= ua.count())
                continue;

            Universe *universe = ua[universeIndex];

            QSharedPointer<GenericFader> fader = m_fadersMap.value(universe->id(), QSharedPointer<GenericFader>());
            if (fader.isNull())
            {
                fader = universe->requestFader();
                m_fadersMap[universe->id()] = fader;
            }

            FadeChannel *fc = fader->getChannelFader(m_doc, universe, fixture->id(), channelIndex);
            fc->setCurrent(it.value());
            fc->setTarget(it.value());
        }
    }
    if (m_clearRequest)
    {
        m_clearRequest = false;
        m_values.clear();

        QMapIterator <quint32, QSharedPointer<GenericFader> > it(m_fadersMap);
        while (it.hasNext() == true)
        {
            it.next();
            quint32 universe = it.key();
            QSharedPointer<GenericFader> fader = it.value();
            ua[universe]->dismissFader(fader);
        }
        m_fadersMap.clear();
    }
}
