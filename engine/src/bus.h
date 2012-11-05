/*
  Q Light Controller
  bus.h

  Copyright (c) Heikki Junnila

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

#ifndef BUS_H
#define BUS_H

#include <QObject>
#include <QString>
#include <QHash>

class QDomElement;
class BusEntry;

#define KXMLQLCBus "Bus"
#define KXMLQLCBusID "ID"
#define KXMLQLCBusName "Name"
#define KXMLQLCBusValue "Value"
#define KXMLQLCBusLowLimit "LowLimit"
#define KXMLQLCBusHighLimit "HighLimit"

#define KXMLQLCBusRole "Role"
#define KXMLQLCBusFade "Fade"
#define KXMLQLCBusHold "Hold"

/**
 * WARNING! THIS CLASS IS DEPRECATED AND IS USED ONLY FOR LOADING LEGACY WORKSPACE FILES!
 *
 * Bus is used by functions to get information on their desired running time.
 * The values that bus uses are (1 / MasterTimer::frequency())'ths of a second;
 * If frequency is 50, then a bus value of 25 means half a second, 50 a full
 * second, 100 two seconds etc...
 *
 * Scene functions use bus values for fade time: how long it should take
 * for the channels to fade from their current values to the ones specified in
 * the scene.
 *
 * Chaser functions use bus values for step interval: how long the chaser
 * should wait until the next step is triggered. Chasers utilize also the bus'
 * capability to send tapped() signals, which, when emitted, interrupt the
 * current step waiting and immediately move to the next step.
 *
 * EFX functions use bus values to specify the duration of one full cycle: if
 * an EFX is set to perform a "Circle" algorithm, its bus value defines the time
 * it should take for the function to run a full 360 degree circle.
 */
class Bus : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Bus)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /**
     * Initialize the Bus singleton object, setting the given object as
     * the bus' parent.
     *
     * @param parent A parent object who owns the Bus singleton
     */
    static void init(QObject* parent);

    /**
     * Get the bus singleton object. If Bus::init() has not been called,
     * the debug version asserts.
     *
     * @return The bus singleton or NULL
     */
    static Bus* instance();

    /**
     * Get the maximum number of buses
     */
    static quint32 count();

    /**
     * Get the bus ID of the default fade bus
     */
    static quint32 defaultFade();

    /**
     * Get the bus ID of the default hold bus
     */
    static quint32 defaultHold();

    /**
     * Get the bus ID of the default palette bus
     */
    static quint32 defaultPalette();

    /**
     * Get the invalid bus ID (indicating no bus)
     */
    static quint32 invalid();

    /**
     * Destructor
     */
    ~Bus();

protected:
    /**
     * Create a new Bus instance with the given parent object.
     *
     * @param parent A parent object who owns the Bus
     */
    Bus(QObject* parent);

protected:
    QList <BusEntry*> m_buses;
    static Bus* s_instance;

    /********************************************************************
     * Value
     ********************************************************************/
public:
    /**
     * Get the value of a bus.
     *
     * @param bus The index of the bus, whose value to get.
     * @return Bus value or 0 if the bus does not exist.
     */
    quint32 value(quint32 bus) const;

    /**
     * Set the value of a bus and emit valueChanged(), if the bus is valid.
     *
     * @param bus The index of the bus, whose value to set.
     * @param value The value to set to the bus.
     */
    void setValue(quint32 bus, quint32 value);

signals:
    void valueChanged(quint32 bus, quint32 value);

    /********************************************************************
     * Name
     ********************************************************************/
public:
    /**
     * Get the name of a bus.
     *
     * @param bus The index of the bus, whose name to get.
     * @return Bus name or an empty string if the bus does not exist.
     */
    QString name(quint32 bus) const;

    /**
     * Get the name and index of a bus in one string, to be used in
     * UI elements.
     *
     * @param The index of the bus, whose name to get.
     * @return Bus ID and name (e.g. "1: Fade") or an empty string if the
     *         given bus does not exist.
     */
    QString idName(quint32 bus) const;

    /**
     * Get a list of all buses. Each entry in the list takes the same form
     * as idName() does for a single bus.
     *
     * @return A list of idNames for all available buses.
     */
    QStringList idNames() const;

    /**
     * Set the name of a bus and emit nameChanged() if bus is valid.
     *
     * @param bus The index of the bus, whose name to set.
     * @param name The new name of the bus.
     */
    void setName(quint32 bus, const QString& name);

signals:
    void nameChanged(quint32 bus, const QString& name);

    /********************************************************************
     * Tap
     ********************************************************************/
public:
    /**
     * Emit a tapped signal thru the given bus. If bus does not exist,
     * no signal is emitted. Tap signals are used, for example, in chasers
     * to immediately skip to the next step instead of waiting for the set
     * time to pass.
     *
     * @param bus The index of the bus to tap.
     */
    void tap(quint32 bus);

signals:
    void tapped(quint32 id);

    /********************************************************************
     * Load
     ********************************************************************/
public:
    /** Load all buses from an XML document */
    bool loadXML(const QDomElement& root);
};

#endif
