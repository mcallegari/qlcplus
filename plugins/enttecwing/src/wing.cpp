/*
  Q Light Controller
  wing.cpp

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

#include <QHostAddress>
#include <QMessageBox>
#include <QByteArray>
#include <QString>

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
    emit pageChanged(pageSize(), m_page);
}

void Wing::previousPage()
{
    if (m_page == WING_PAGE_MIN)
        m_page = WING_PAGE_MAX;
    else
        m_page--;
    emit pageChanged(pageSize(), m_page);
}

uchar Wing::page() const
{
    return m_page;
}

quint32 Wing::pageSize() const
{
    return 0;
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
