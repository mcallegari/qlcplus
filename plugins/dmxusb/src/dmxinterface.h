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

#include <QtCore>

#define SETTINGS_TYPE_MAP "qlcftdi/typemap"
#define SETTINGS_FREQ_MAP "qlcftdi/freqmap"

class DMXInterface
{
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
                 quint16 VID, quint16 PID, quint32 id = 0);

    /** Destructor */
    virtual ~DMXInterface();

    virtual bool readLabel(uchar label, int &ESTA_code, QString &strParam) = 0;

    /** Get the widget's USB serial number */
    QString serial() const;

    /** Get the widget's USB name */
    QString name() const;

    /** Get the widget's USB vendor name */
    QString vendor() const;

    /** Get the widget's USB vendor ID */
    quint16 vendorID() const;

    /** Get the widget's USB product ID */
    quint16 productID() const;

    /** Get the widget's FTD2XX ID number */
    quint32 id() const;

    /** Virtual method to retrieve the original USB
     *  bus location of the device.
     *  Used only in Linux to perform a sysfs lookup */
    virtual quint8 busLocation();

private:
    QString m_serial;
    QString m_name;
    QString m_vendor;
    quint16 m_vendorID;
    quint16 m_productID;
    quint32 m_id;

    /************************************************************************
     * Widget enumeration
     ************************************************************************/
public:
    static const int FTDIVID = 0x0403;       //! FTDI Vendor ID
    static const int ATMELVID = 0x03EB;      //! Atmel Vendor ID
    static const int MICROCHIPVID = 0x04D8;  //! Microchip Vendor ID
    static const int NXPVID = 0x1FC9;        //! NXP Vendor ID
    static const int FTDIPID = 0x6001;       //! FTDI Product ID
    static const int FTDI2PID = 0x6010;      //! FTDI COM485-PLUS2 Product ID
    static const int DMX4ALLPID = 0xC850;    //! DMX4ALL FTDI Product ID
    static const int NANODMXPID = 0x2018;    //! DMX4ALL Nano DMX Product ID
    static const int EUROLITEPID = 0xFA63;   //! Eurolite USB DMX Product ID
    static const int ELECTROTASPID = 0x0000; //! ElectroTAS USB DMX Product ID
    static const int DMXKINGMAXPID = 0x0094; //! DMXKing ultraDMX MAX Product ID

    /** Driver types */
    enum Type
    {
        libFTDI = 0,
        FTD2xx,
        QtSerial
    };

    /**
     * Check if an interface is supported by QLC+
     *
     * @return true if supported, false if unsupported
     */
    static bool validInterface(quint16 vendor, quint16 product);

    bool checkInfo(QString &serial, QString &name, QString &vendor);

    /**
     * Get a map of [serial = type] bindings that tells which serials should
     * be used to force the plugin to use pro/open method on which widget
     */
    static QMap <QString,QVariant> typeMap();

    static void storeTypeMap(const QMap <QString,QVariant> map);

    /**
     * Get a map of [serial = frequency] bindings that tells which
     * output frequency should be used on a specifi serail number
     */
    static QMap <QString,QVariant> frequencyMap();

    static void storeFrequencyMap(const QMap <QString,QVariant> map);

    /************************************************************************
     * DMX/Serial Interface Methods
     ************************************************************************/
public:
    virtual DMXInterface::Type type() = 0;

    virtual QString typeString() = 0;

    /** Open the widget */
    virtual bool open() = 0;

    /** Open the widget using a specific Product ID */
    virtual bool openByPID(const int FTDIPID) = 0;

    /** Close the widget */
    virtual bool close() = 0;

    /** Check if the widget is open */
    virtual bool isOpen() const = 0;

    /** Reset the communications line */
    virtual bool reset() = 0;

    /** Setup communications line for 8N2 traffic */
    virtual bool setLineProperties() = 0;

    /** Set 250kbps baud rate */
    virtual bool setBaudRate() = 0;

    /** Disable flow control */
    virtual bool setFlowControl() = 0;

    /**
     * Set the widget in "low latency mode". Some DMX controllers send DMX
     * frames at a much higher rate than the specified value. USB widget may
     * have difficulties to read independant frames in this case and need
     * some configuration.
     *
     * @param lowLatency true for low latency, false otherwise
     * @return true if the interface was set in low latency state
     */
    virtual bool setLowLatency(bool lowLatency) = 0;

    /** Clear the RTS bit */
    virtual bool clearRts() = 0;

    /** Purge TX & RX buffers */
    virtual bool purgeBuffers() = 0;

    /** Toggle communications line BREAK condition on/off */
    virtual bool setBreak(bool on) = 0;

    /** Write data to a previously-opened line */
    virtual bool write(const QByteArray& data) = 0;

    /** Read data from a previously-opened line. Optionally provide own data buffer. */
    virtual QByteArray read(int size, uchar* buffer = NULL) = 0;

    /** Read exactly one byte. $ok tells if a byte was read or not. */
    virtual uchar readByte(bool* ok = NULL) = 0;

protected:
    /** Latency amount in ms for FTDI devices */
    unsigned char m_defaultLatency;
};

#endif
