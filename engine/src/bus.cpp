/*
  Q Light Controller
  bus.cpp

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

#include <QDomElement>
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

bool Bus::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCBus)
    {
        qWarning() << Q_FUNC_INFO << "Bus node not found!";
        return false;
    }

    quint32 id = root.attribute(KXMLQLCBusID).toUInt();
    if (id >= KBusCount)
    {
        qWarning() << Q_FUNC_INFO << "Bus ID" << id << "out of bounds.";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCBusName)
            setName(id, tag.text());
        else if (tag.tagName() == KXMLQLCBusValue)
            setValue(id, tag.text().toULong());
        else
            qWarning() << Q_FUNC_INFO << "Unknown Bus tag:" << tag.tagName();
        node = node.nextSibling();
    }

    return true;
}
