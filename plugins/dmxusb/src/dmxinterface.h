/*
  Q Light Controller Plus
  dmxinterface.h

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

#ifndef DMXINTERFACE_H
#define DMXINTERFACE_H

#include <QByteArray>
#include <QSettings>
#include <QString>
#include <QMutex>
#include <QList>
#include <QMap>

#define SETTINGS_TYPE_MAP "qlcftdi/typemap"

class DMXInterface
{
    /************************************************************************
     * Widget enumeration
     ************************************************************************/
public:
    static const int FTDIVID = 0x0403;      //! FTDI Vendor ID
    static const int ATMELVID = 0x03EB;     //! Atmel Vendor ID
    static const int MICROCHIPVID = 0x04D8; //! Microchip Vendor ID
    static const int FTDIPID = 0x6001;      //! FTDI Product ID
    static const int DMX4ALLPID = 0xC850;   //! DMX4ALL FTDI Product ID
    static const int NANODMXPID = 0x2018;   //! DMX4ALL Nano DMX Product ID
    static const int EUROLITEPID = 0xFA63;  //! Eurolite USB DMX Product ID

    /** Driver types */
    enum Type
    {
        libFTDI = 0,
        FTD2xx,
        QtSerial
    };

    /** Comparator function for matching DMXInterfaces */
    bool operator== (const DMXInterface& iface) const
    {
        if (m_name == iface.m_name &&
            m_serial == iface.m_serial &&
            m_vendor == iface.m_vendor)
                return true;
        return false;
    }

    /**
     * Check if an interface is supported by QLC+
     *
     * @return true if supported, false if unsupported
     */
    static bool validInterface(quint16 vendor, quint16 product)
    {
        if (vendor != DMXInterface::FTDIVID &&
            vendor != DMXInterface::ATMELVID &&
            vendor != DMXInterface::MICROCHIPVID)
                return false;

        if (product != DMXInterface::FTDIPID &&
            product != DMXInterface::DMX4ALLPID &&
            product != DMXInterface::NANODMXPID &&
            product != DMXInterface::EUROLITEPID)
                return false;

        return true;
    }

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
     * Construct a new DMXInterface instance for one DMX adapter.
     *
     * @param serial The interface USB serial number
     * @param name The interface USB name (description)
     * @param vendor The interface USB vendor name (description)
     * @param VID The interface USB vendor ID
     * @param PID The interfce USB product ID
     * @param id The ID of the device (used only when FTD2XX is the backend)
     */
    DMXInterface(const QString& serial, const QString& name, const QString &vendor,
                 quint16 VID, quint16 PID, quint32 id = 0)
        : m_serial(serial)
        , m_name(name)
        , m_vendor(vendor)
        , m_vendorID(VID)
        , m_productID(PID)
        , m_id(id)
    { }

    /** Destructor */
    virtual ~DMXInterface() { }

    virtual QString readLabel(uchar label, int *ESTA_code);

    /** Get the widget's USB serial number */
    QString serial() const { return m_serial; }

    /** Get the widget's USB name */
    QString name() const { return m_name; }

    /** Get the widget's USB vendor name */
    QString vendor() const { return m_vendor; }

    /** Get the widget's USB vendor ID */
    quint16 vendorID() const { return m_vendorID; }

    /** Get the widget's USB product ID */
    quint16 productID() const { return m_productID; }

    /** Get the widget's FTD2XX ID number */
    quint32 id() const { return m_id; }

    /** Virtual method to retrieve the original USB
     *  bus location of the device.
     *  Used only in Linux to perform a sysfs lookup */
    virtual quint8 busLocation() { return 0; }

private:
    QString m_serial;
    QString m_name;
    QString m_vendor;
    quint16 m_vendorID;
    quint16 m_productID;
    quint32 m_id;

    /************************************************************************
     * DMX/Serial Interface Methods
     ************************************************************************/
public:
    virtual DMXInterface::Type type() = 0;

    /** Open the widget */
    virtual bool open();

    /** Open the widget using a specific Product ID */
    virtual bool openByPID(const int FTDIPID);

    /** Close the widget */
    virtual bool close();

    /** Check if the widget is open */
    virtual bool isOpen() const;

    /** Reset the communications line */
    virtual bool reset();

    /** Setup communications line for 8N2 traffic */
    virtual bool setLineProperties();

    /** Set 250kbps baud rate */
    virtual bool setBaudRate();

    /** Disable flow control */
    virtual bool setFlowControl();

    /** Clear the RTS bit */
    virtual bool clearRts();

    /** Purge TX & RX buffers */
    virtual bool purgeBuffers();

    /** Toggle communications line BREAK condition on/off */
    virtual bool setBreak(bool on);

    /** Write data to a previously-opened line */
    virtual bool write(const QByteArray& data);

    /** Read data from a previously-opened line. Optionally provide own data buffer. */
    virtual QByteArray read(int size, uchar* buffer = NULL);

    /** Read exactly one byte. $ok tells if a byte was read or not. */
    virtual uchar readByte(bool* ok = NULL);
};

#endif
