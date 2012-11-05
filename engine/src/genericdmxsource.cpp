/*
  Q Light Controller
  genericdmxsource.cpp

  Copyright (C) Heikki Junnila

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

#include "genericdmxsource.h"
#include "universearray.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "qlcchannel.h"
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

void GenericDMXSource::writeDMX(MasterTimer* timer, UniverseArray* ua)
{
    Q_UNUSED(timer);

    m_mutex.lock();
    QMutableMapIterator <QPair<quint32,quint32>,uchar> it(m_values);
    while (it.hasNext() == true && m_outputEnabled == true)
    {
        it.next();

        FadeChannel fc;
        fc.setFixture(it.key().first);
        fc.setChannel(it.key().second);

        QLCChannel::Group grp = fc.group(m_doc);
        quint32 address = fc.address(m_doc);

        ua->write(address, it.value(), grp);
        if (grp != QLCChannel::Intensity)
            it.remove();
    }
    m_mutex.unlock();
}
