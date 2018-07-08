/*
  Q Light Controller
  wing.cpp

  Copyright (c) Heikki Junnila

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

#include <QHostAddress>
#include <QMessageBox>
#include <QByteArray>
#include <QString>
#include <QDebug>

#include "wing.h"

/** ENTTEC wings send data thru UDP port 3330 */
const int Wing::UDPPort = 3330;

/****************************************************************************
 * Initialization
 ****************************************************************************/

Wing::Wing(QObject* parent, const QHostAddress& address, const QByteArray& data)
    : QObject(parent)
{
    m_address = address;
    m_type = resolveType(data);
    m_firmware = resolveFirmware(data);
    m_page = WING_PAGE_MIN;
}

Wing::~Wing()
{
}

bool Wing::isOutputData(const QByteArray& data)
{
    /* Check, if there's enough bytes for the header */
    if (data.size() < WING_HEADER_SIZE)
        return false;

    QByteArray header(data.mid(WING_BYTE_HEADER, WING_HEADER_SIZE));
    return (header == WING_HEADER_OUTPUT);
}

QString Wing::infoText() const
{
    QString str;
    str  = QString("<B>%1</B>").arg(name());
    str += QString("<P>");
    str += tr("Firmware version %1").arg(int(m_firmware));
    str += QString("<BR>");
    str += tr("Device is operating correctly.");
    str += QString("</P>");
    return str;
}

/****************************************************************************
 * Wing data
 ****************************************************************************/

QHostAddress Wing::address() const
{
    return m_address;
}

Wing::Type Wing::type() const
{
    return m_type;
}

uchar Wing::firmware() const
{
    return m_firmware;
}

Wing::Type Wing::resolveType(const QByteArray& data)
{
    /* Check, if there's enough bytes for wing flags */
    if (data.size() < WING_BYTE_FLAGS)
    {
        qWarning() << Q_FUNC_INFO
                   << "Unable to determine wing type."
                   << "Expected at least" << WING_BYTE_FLAGS
                   << "bytes but got only" << data.size();
        return Unknown;
    }

    unsigned char flags = data[WING_BYTE_FLAGS];
    return Wing::Type(flags & WING_FLAGS_MASK_TYPE);
}

unsigned char Wing::resolveFirmware(const QByteArray& data)
{
    /* Check, if there's enough bytes for wing flags */
    if (data.size() < WING_BYTE_FIRMWARE)
    {
        qWarning() << Q_FUNC_INFO
                   << "Unable to determine firmware version."
                   << "Expected at least" << WING_BYTE_FIRMWARE
                   << "bytes but got only" << data.size();
        return 0;
    }

    return data[WING_BYTE_FIRMWARE];
}

/****************************************************************************
 * Page
 ****************************************************************************/

void Wing::nextPage()
{
    if (m_page == WING_PAGE_MAX)
        m_page = WING_PAGE_MIN;
    else
        m_page++;
}

void Wing::previousPage()
{
    if (m_page == WING_PAGE_MIN)
        m_page = WING_PAGE_MAX;
    else
        m_page--;
}

uchar Wing::page() const
{
    return m_page;
}


/****************************************************************************
 * Input data
 ****************************************************************************/

uchar Wing::cacheValue(int channel)
{
    if (channel >= m_values.size())
        return 0;
    else
        return m_values[channel];
}

void Wing::setCacheValue(int channel, uchar value)
{
    if (channel >= m_values.size())
        return;

    if (channel != WING_INVALID_CHANNEL && m_values[channel] != char(value))
    {
        m_values[channel] = char(value);
        emit valueChanged(channel, value);
    }
}

void Wing::feedBack(quint32 channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

uchar Wing::toBCD(uchar number)
{
    uchar bcd = ((number / 10) & 0x0F) << 4;
    bcd |= (number % 10) & 0x0F;
    return bcd;
}
