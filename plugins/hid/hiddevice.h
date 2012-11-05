/*
  Q Light Controller
  hiddevice.h

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

#ifndef HIDDEVICE_H
#define HIDDEVICE_H

#include <QObject>
#include <QFile>

class HID;

/*****************************************************************************
 * HIDDevice
 *****************************************************************************/

class HIDDevice : public QObject
{
    Q_OBJECT

public:
    HIDDevice(HID* parent, quint32 line, const QString& path);
    virtual ~HIDDevice();

    /*************************************************************************
     * File operations
     *************************************************************************/
public:
    /**
     * Attempt to open the HID device in RW mode and fall back to RO
     * if that fails.
     *
     * @return true if the file was opened RW/RO
     */
    virtual bool open();

    /**
     * Close the HID device
     */
    virtual void close();

    /**
     * Get the full path of this HID device
     */
    virtual QString path() const;

    /**
     * Get the device's file descriptor
     */
    virtual int handle() const;

    /**
     * Read one event and emit it.
     */
    virtual bool readEvent() = 0;

protected:
    QFile m_file;

    /*************************************************************************
     * Line
     *************************************************************************/
public:
    quint32 line() const {
        return m_line;
    }

protected:
    quint32 m_line;

    /*************************************************************************
     * Device info
     *************************************************************************/
public:
    /**
     * Get HID device information string to be used in plugin manager
     */
    virtual QString infoText();

    /**
     * Get the device's name
     */
    virtual QString name();

protected:
    QString m_name;

    /*************************************************************************
     * Input data
     *************************************************************************/
signals:
    /**
     * Signal that is emitted when an input channel's value is changed
     *
     * @param device The eventing HIDDevice
     * @param channel The channel whose value has changed
     * @param value The changed value
     */
    void valueChanged(HIDDevice* device, quint32 channel, uchar value);

public:
    /**
     * Send an input value back the HID device to move motorized sliders
     * and such.
     */
    virtual void feedBack(quint32 channel, uchar value);
};

#endif
