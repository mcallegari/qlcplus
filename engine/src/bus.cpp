/*
  Q Light Controller Plus
  bus.cpp

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#include <QXmlStreamReader>
#include <QStringList>
#include <QDebug>

#include "bus.h"

#define KBusCount 32
#define KBusIDDefaultFade 0
#define KBusIDDefaultHold 1
#define KBusIDDefaultPalette KBusCount - 1
#define KBusIDInvalid UINT_MAX

/****************************************************************************
 * BusEntry
 ****************************************************************************/

/**
 * BusEntry is practically THE bus, while Bus is only the common access point
 * for individual BusEntry instances. BusEntry instances are entities that have
 * only a name and a value that can be set thru Bus with the entries' index
 * numbers that correspond to their position in Bus::m_buses list. Since
 * BusEntry is such a simple class, it's not a QObject and therefore it cannot
 * emit signals. BusEntries act just as storing locations for the values and
 * names for each of the buses, while Bus itself handles signal emission and
 * set/get methods.
 */
class BusEntry
{
public:
    BusEntry()
    {
        value = 0;
    }

    ~BusEntry()
    {
    }

    BusEntry(const BusEntry& entry)
    {
        name = entry.name;
        value = entry.value;
    }

    QString name;
    quint32 value;
};

/****************************************************************************
 * Initialization
 ****************************************************************************/

Bus* Bus::s_instance = NULL;

void Bus::init(QObject* parent)
{
    if (s_instance == NULL)
    {
        Q_ASSERT(parent != NULL);
        s_instance = new Bus(parent);
    }
}

Bus* Bus::instance()
{
    return s_instance;
}

Bus::Bus(QObject* parent) : QObject(parent)
{
    for (quint32 i = 0; i < Bus::count(); i++)
        m_buses.append(new BusEntry);

    m_buses[defaultFade()]->name = QString("Fade");
    m_buses[defaultHold()]->name = QString("Hold");
    m_buses[defaultPalette()]->name = QString("Palette");
}

quint32 Bus::count()
{
    return KBusCount;
}

quint32 Bus::defaultFade()
{
    return KBusIDDefaultFade;
}

quint32 Bus::defaultHold()
{
    return KBusIDDefaultHold;
}

quint32 Bus::defaultPalette()
{
    return KBusIDDefaultPalette;
}

quint32 Bus::invalid()
{
    return KBusIDInvalid;
}

Bus::~Bus()
{
    while (m_buses.isEmpty() == false)
        delete m_buses.takeFirst();

    s_instance = NULL;
}

/****************************************************************************
 * Name
 ****************************************************************************/

QString Bus::name(quint32 bus) const
{
    if (bus < KBusCount)
        return m_buses[bus]->name;
    else
        return QString();
}

QString Bus::idName(quint32 bus) const
{
    if (bus < KBusCount)
    {
        QString nomen(name(bus));
        if (nomen.simplified().isEmpty() == true)
            return QString("Bus %1").arg(bus + 1);
        else
            return nomen;
    }
    else
    {
        return QString();
    }
}

QStringList Bus::idNames() const
{
    QStringList list;
    for (quint32 bus = 0; bus < KBusCount; bus++)
        list << idName(bus);
    return list;
}

void Bus::setName(quint32 bus, const QString& name)
{
    if (bus < KBusCount)
    {
        m_buses[bus]->name = name;
        emit nameChanged(bus, name);
    }
}

/****************************************************************************
 * Value
 ****************************************************************************/

quint32 Bus::value(quint32 bus) const
{
    if (bus < KBusCount)
        return m_buses[bus]->value;
    else
        return 0;
}

void Bus::setValue(quint32 bus, quint32 value)
{
    if (bus < KBusCount)
    {
        m_buses[bus]->value = value;
        emit valueChanged(bus, value);
    }
}

/****************************************************************************
 * Tap
 ****************************************************************************/

void Bus::tap(quint32 bus)
{
    if (bus < KBusCount)
        emit tapped(bus);
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool Bus::loadXML(QXmlStreamReader &doc)
{
    if (doc.name() != KXMLQLCBus)
    {
        qWarning() << Q_FUNC_INFO << "Bus node not found!";
        return false;
    }

    quint32 id = doc.attributes().value(KXMLQLCBusID).toString().toUInt();
    if (id >= KBusCount)
    {
        qWarning() << Q_FUNC_INFO << "Bus ID" << id << "out of bounds.";
        return false;
    }

    while (doc.readNextStartElement())
    {
        if (doc.name() == KXMLQLCBusName)
            setName(id, doc.readElementText());
        else if (doc.name() == KXMLQLCBusValue)
            setValue(id, doc.readElementText().toULong());
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Bus tag:" << doc.name();
            doc.skipCurrentElement();
        }
    }

    return true;
}
