/*
  Q Light Controller Plus
  dmxinterface.cpp

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

#include "dmxinterface.h"

DMXInterface::DMXInterface(const QString& serial, const QString& name, const QString &vendor,
                           quint16 VID, quint16 PID, quint32 id)
    : m_serial(serial)
    , m_name(name)
    , m_vendor(vendor)
    , m_vendorID(VID)
    , m_productID(PID)
    , m_id(id)
{
}

DMXInterface::~DMXInterface()
{
}

QString DMXInterface::serial() const
{
    return m_serial;
}

QString DMXInterface::name() const
{
    return m_name;
}

QString DMXInterface::vendor() const
{
    return m_vendor;
}

quint16 DMXInterface::vendorID() const
{
    return m_vendorID;
}

quint16 DMXInterface::productID() const
{
    return m_productID;
}

quint32 DMXInterface::id() const
{
    return m_id;
}

quint8 DMXInterface::busLocation()
{
    return 0;
}

bool DMXInterface::validInterface(quint16 vendor, quint16 product)
{
    if (vendor != DMXInterface::FTDIVID &&
        vendor != DMXInterface::ATMELVID &&
        vendor != DMXInterface::MICROCHIPVID &&
        vendor != DMXInterface::NXPVID)
            return false;

    if (product != DMXInterface::FTDIPID &&
        product != DMXInterface::FTDI2PID &&
        product != DMXInterface::DMX4ALLPID &&
        product != DMXInterface::NANODMXPID &&
        product != DMXInterface::EUROLITEPID &&
        product != DMXInterface::ELECTROTASPID &&
        product != DMXInterface::DMXKINGMAXPID)
            return false;

    return true;
}

bool DMXInterface::checkInfo(QString &serial, QString &name, QString &vendor)
{
    if (m_serial == serial && m_name == name && m_vendor == vendor)
        return true;
    return false;
}

QMap <QString,QVariant> DMXInterface::typeMap()
{
    QMap <QString,QVariant> typeMap;
    QSettings settings;
    QVariant var(settings.value(SETTINGS_TYPE_MAP));
    if (var.isValid() == true)
        typeMap = var.toMap();
    return typeMap;
}

void DMXInterface::storeTypeMap(const QMap <QString,QVariant> map)
{
    QSettings settings;
    settings.setValue(SETTINGS_TYPE_MAP, map);
}

QMap<QString, QVariant> DMXInterface::frequencyMap()
{
    QMap <QString,QVariant> typeMap;
    QSettings settings;
    QVariant var(settings.value(SETTINGS_FREQ_MAP));
    if (var.isValid() == true)
        typeMap = var.toMap();
    return typeMap;
}

void DMXInterface::storeFrequencyMap(const QMap<QString, QVariant> map)
{
    QSettings settings;
    settings.setValue(SETTINGS_FREQ_MAP, map);
}
