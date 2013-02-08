/*
  Q Light Controller
  wing.h

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

#ifndef WING_H
#define WING_H

#include <QHostAddress>
#include <QByteArray>
#include <QObject>

#include "qlcmacros.h"

#define WING_INVALID_CHANNEL -1

/****************************************************************************
 * Header data common to all wings
 ****************************************************************************/

#define WING_BYTE_HEADER   0 /* 4 bytes */
#define WING_HEADER_SIZE   4
#define WING_HEADER_OUTPUT "WODD"
#define WING_HEADER_INPUT  "WIDD"
#define WING_PAGE_MIN      0
#define WING_PAGE_MAX      99

/****************************************************************************
 * Status data common to all wings
 ****************************************************************************/

#define WING_BYTE_FIRMWARE   4 /* Firmware version, 8bit value (0-255) */

#define WING_BYTE_FLAGS      5 /* Wing flags */
#define WING_FLAGS_MASK_TYPE 0x3

/****************************************************************************
 * Wing
 ****************************************************************************/

class QLC_DECLSPEC Wing : public QObject
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** The UDP port of ENTTEC wing devices */
    static const int UDPPort;

    /** There are currently three types of wings from ENTTEC. */
    enum Type
    {
        Unknown  = 0,
        Playback = 1,
        Shortcut = 2,
        Program  = 3
    };

    /**
     * Construct a new Wing object. Since Wing contains pure virtual
     * functions, the Wing class cannot be instantiated without
     * inheritance.
     *
     * @param parent The parent object that owns the new wing object.
     * @param address The address of the physical wing board.
     * @param data A UDP datagram packet originating from a wing.
     */
    Wing(QObject* parent, const QHostAddress& address, const QByteArray& data);

    /**
     * Destructor.
     */
    virtual ~Wing();

public:
    /**
     * Check, if the type of data is output (from the wing's perspective).
     *
     * @param datagram The data, whose type to check.
     */
    static bool isOutputData(const QByteArray& datagram);

    /********************************************************************
     * Wing data
     ********************************************************************/
public:
    /**
     * Get the address of the device.
     *
     * @return The IP address of the wing board.
     */
    QHostAddress address() const;

    /**
     * Get the type of the device (see Type enum).
     *
     * @return The type of the device
     */
    Type type() const;

    /**
     * Return the name of the wing.
     *
     * This function is pure virtual and must be implemented in each of the
     * inheriting classes.
     *
     * @return The name of the device in the form "<type> at <address>"
     */
    virtual QString name() const = 0;

    /**
     * Return an info string describing the device's state.
     *
     * This function is pure virtual and must be implemented in each of the
     * inheriting classes.
     *
     * @return Information string.
     */
    virtual QString infoText() const;

    /**
     * Get the wing's firmware version.
     *
     * @return Firmware version
     */
    uchar firmware() const;

public:
    /**
     * Resolve the exact type of the wing from the given data packet.
     *
     * @param data The data packet to resolve
     * @return The wing type
     */
    static Type resolveType(const QByteArray& data);

protected:
    /**
     * Resolve the wing's firmware version.
     *
     * @param data The data packet to resolve
     * @return Firmware version (0-255)
     */
    static uchar resolveFirmware(const QByteArray& data);

protected:
    QHostAddress m_address;
    Type m_type;
    uchar m_firmware;

    /********************************************************************
     * Page
     ********************************************************************/
public:
    void nextPage();
    void previousPage();

    uchar page() const;
    virtual quint32 pageSize() const;

private:
    uchar m_page;

    /********************************************************************
     * Input data
     ********************************************************************/
public:
    /**
     * Parse input data and generate signals for each changed value.
     *
     * This function is pure virtual and must be implemented in each of the
     * inheriting classes.
     *
     * @param data The data packet to parse
     */
    virtual void parseData(const QByteArray& data) = 0;

    /**
     * Send feedback data to the wing.
     *
     * @param channel The channel to send feedback data to
     * @param value The feedback value to set to the given channel
     */
    virtual void feedBack(quint32 channel, uchar value);

    /**
     * Get the cached value of the given channel.
     *
     * @param channel A channel whose value to get
     * @return The channel's value (0 if not found)
     */
    uchar cacheValue(int channel);

    /**
     * Convert the given number to a BCD number (0-99).
     *
     * @param number The number to convert
     * @return BCD-encoded representation of the given number
     */
    static uchar toBCD(quint8 number);

protected:
    /**
     * Set the value of a channel and emit valueChanged() if the value has
     * changed from its previous state.
     *
     * @param channel A channel, whose value to change
     * @param value The value to set
     */
    void setCacheValue(int channel, uchar value);

signals:
    /**
     * Changed values are signalled with this signal.
     *
     * @param channel The number of the changed channel
     * @param value The new value for the channel
     */
    void valueChanged(quint32 channel, uchar value);

    /**
     * page changes are signalled with this signal.
     *
     * @param pagesize The number of channels per page
     * @param page The current page
     */
    void pageChanged(quint32 pagesize, quint32 page);

protected:
    QByteArray m_values;
};

#endif
