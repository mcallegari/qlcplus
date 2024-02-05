/*
  Q Light Controller Plus
  bus.h

  Copyright (c) Heikki Junnila
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

#ifndef BUS_H
#define BUS_H

#include <QObject>
#include <QString>
#include <QHash>

class QXmlStreamReader;
class BusEntry;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCBus          QString("Bus")
#define KXMLQLCBusID        QString("ID")
#define KXMLQLCBusName      QString("Name")
#define KXMLQLCBusValue     QString("Value")
#define KXMLQLCBusLowLimit  QString("LowLimit")
#define KXMLQLCBusHighLimit QString("HighLimit")

#define KXMLQLCBusRole QString("Role")
#define KXMLQLCBusFade QString("Fade")
#define KXMLQLCBusHold QString("Hold")

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
    bool loadXML(QXmlStreamReader &doc);
};

/** @} */

#endif
