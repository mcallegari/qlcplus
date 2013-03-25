/*
  Q Light Controller
  qlcftdi.h

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

#ifndef QLCFTDI_H
#define QLCFTDI_H

#include <QByteArray>
#include <QSettings>
#include <QString>
#include <QMutex>
#include <QList>
#include <QMap>

#ifdef FTD2XX
#   ifdef WIN32
#       include <windows.h>
#   endif
#   include <ftd2xx.h>
#else
#   include <ftdi.h>
#endif

#define SETTINGS_TYPE_MAP "qlcftdi/typemap"

class DMXUSBWidget;

class QLCFTDI
{
    /************************************************************************
     * Widget enumeration
     ************************************************************************/
public:
    static const int VID = 0x0403; //! FTDI Vendor ID
    static const int PID = 0x6001; //! FTDI Product ID
    static const int DMX4ALLPID = 0xC850; //! DMX4ALL Product ID

#ifdef FTD2XX
    static QString readLabel(quint32 id, uchar label, int *ESTA_code);
#else
    static QString readLabel(struct ftdi_context *ftdi, char *name, char *serial, uchar label, int *ESTA_code);
#endif
    /**
     * Compose a list of available widgets
     *
     * @return A list of enttec-compabitble devices
     */
    static QList<DMXUSBWidget *> widgets();

    /**
     * Get a map of [serial = type] bindings that tells which serials should
     * be used to force the plugin to use pro/open method on which widget.
     */
    static QMap <QString,QVariant> typeMap()
    {
        QMap <QString,QVariant> typeMap;
        QSettings settings;
        QVariant var(settings.value(SETTINGS_TYPE_MAP));
        if (var.isValid() == true)
            typeMap = var.toMap();
        return typeMap;
    }

    static void storeTypeMap(const QMap <QString,QVariant> map)
    {
        QSettings settings;
        settings.setValue(SETTINGS_TYPE_MAP, map);
    }

    /************************************************************************
     * Construction & Generic Information
     ************************************************************************/
public:
    /**
     * Construct a new QLCFTDI instance for one widget.
     *
     * @param serial The widget's USB serial number
     * @param name The widget's USB name (description)
     * @param id The ID of the device (used only when FTD2XX is the backend)
     */
    QLCFTDI(const QString& serial, const QString& name, const QString &vendor, quint32 id = 0);

    /** Destructor */
    virtual ~QLCFTDI();

    /** Get the widget's USB serial number */
    QString serial() const { return m_serial; }

    /** Get the widget's USB name */
    QString name() const { return m_name; }

    /** Get the widget's USB vendor */
    QString vendor() const { return m_vendor; }

    /** Get the widget's FTD2XX ID number */
    quint32 id() const { return m_id; }

private:
    QString m_serial;
    QString m_name;
    QString m_vendor;
    quint32 m_id;
    int m_refCount;
    int m_openCount;

    /************************************************************************
     * FTDI Interface Methods
     ************************************************************************/
public:
    /** Open the widget */
    bool open();

    /** Open the widget using a specific Product ID */
    bool openByPID(const int PID);

    /** Close the widget */
    bool close();

    /** Check if the widget is open */
    bool isOpen() const;

    /** Reset the communications line */
    bool reset();

    /** Setup communications line for 8N2 traffic */
    bool setLineProperties();

    /** Set 250kbps baud rate */
    bool setBaudRate();

    /** Disable flow control */
    bool setFlowControl();

    /** Clear the RTS bit */
    bool clearRts();

    /** Purge TX & RX buffers */
    bool purgeBuffers();

    /** Toggle communications line BREAK condition on/off */
    bool setBreak(bool on);

    /** Write data to a previously-opened line */
    bool write(const QByteArray& data);

    /** Read data from a previously-opened line. Optionally provide own data buffer. */
    QByteArray read(int size, uchar* buffer = NULL);

    /** Read exactly one byte. $ok tells if a byte was read or not. */
    uchar readByte(bool* ok = NULL);

    /** Modify the reference count by 'amount'. Used when sharing a FTDI handle */
    void modifyRefCount(int amount);

    /** Returns the number of references this class is pointed by */
    int refCount();

private:
#ifdef FTD2XX
    FT_HANDLE m_handle;
#else
    struct ftdi_context m_handle;
#endif
};

#endif
