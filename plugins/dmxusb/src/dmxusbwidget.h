/*
  Q Light Controller
  dmxusbwidget.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef DMXUSBWIDGET_H
#define DMXUSBWIDGET_H

#include "qlcftdi.h"

/**
 * This is the base interface class for ENTTEC USB DMX [Pro|Open] widgets.
 */
class DMXUSBWidget
{
public:
    /**
     * Construct a new DMXUSBWidget object.
     *
     * @param serial The widget's USB serial
     * @param name The name of the widget
     * @param id The ID of the device in FTD2XX (0 when libftdi is used)
     */
    DMXUSBWidget(const QString& serial, const QString& name, const QString &vendor, QLCFTDI *ftdi = NULL, quint32 id = 0);
    virtual ~DMXUSBWidget();

    /** Widget types */
    enum Type
    {
        ProTX,     //! Enttec Pro widget using the TX side of the dongle
        OpenTX,    //! Enttec Open widget (only TX)
        ProRX,     //! Enttec Pro widget using the RX side of the dongle
        ProMk2,    //! Enttec Pro Mk2 widget using 2 TX outputs
        UltraProTx, //! DMXKing Ultra Pro widget using 2 TX ports
        DMX4ALL
    };

    /** Get the type of the widget */
    virtual Type type() const = 0;

    /** Get the QLCFTDI instance */
    QLCFTDI* ftdi() const;

private:
    QLCFTDI* m_ftdi;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    /**
     * Open widget for further operations, such as serial() and writeUniverse()
     *
     * @return true if widget was opened successfully (or was already open)
     */
    virtual bool open();

    /**
     * Close widget, preventing any further operations
     *
     * @param true if widget was closed successfully (or was already closed)
     */
    virtual bool close();

    /**
     * Check, whether widget has been opened
     *
     * @return true if widget is open, otherwise false
     */
    virtual bool isOpen();

    /********************************************************************
     * Serial & name
     ********************************************************************/
public:
    /**
     * Get the widget's USB serial number as a string.
     *
     * @return widget's serial number in string form
     */
    virtual QString serial() const;

    /**
     * Get the device's friendly name.
     *
     * @return widget's name
     */
    virtual QString name() const;

    /**
     * Get the widget's unique name
     *
     * @return widget's unique name as: "<name> (S/N: <serial>)"
     */
    virtual QString uniqueName() const;

    /** Set the real device name extracted from serial using label 78 */
    void setRealName(QString devName);

    /** retrieve the real device name read from label 78 */
    virtual QString realName() const;

    /**
     * Get the widget's vendor name
     *
     * @return widget's vendor
     */
    virtual QString vendor() const;

    /**
     * Get any additional information pertaining to the device (can be empty)
     */
    virtual QString additionalInfo() const { return QString(); }

private:
    QString m_realName;

    /********************************************************************
     * Write universe
     ********************************************************************/
public:
    /**
     * Send the given universe-ful of DMX data to widget. The universe must
     * be at least 25 bytes but no more than 513 bytes long.
     *
     * The default implementation does nothing.
     *
     * @param universe The DMX universe to send
     * @return true if the values were sent successfully, otherwise false
     */
    virtual bool writeUniverse(const QByteArray& universe);
};

#endif
