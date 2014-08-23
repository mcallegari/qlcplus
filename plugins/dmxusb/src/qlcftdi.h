/*
  Q Light Controller
  qlcftdi.h

  Copyright (C) Heikki Junnila

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

#ifndef QLCFTDI_H
#define QLCFTDI_H

#include <QByteArray>
#include <QSettings>
#include <QString>
#include <QMutex>
#include <QList>
#include <QMap>

#if defined(FTD2XX)
  #if defined(WIN32) || defined(Q_OS_WIN)
    #include <windows.h>
  #endif
  #include <ftd2xx.h>
#elif defined(LIBFTDI) || defined(LIBFTDI1)
  #include <ftdi.h>
#elif defined(QTSERIAL)
  #include <QtSerialPort/QSerialPort>
  #include <QtSerialPort/QSerialPortInfo>
#endif

#if defined(LIBFTDI1)
  #include <unistd.h>
  #include <libusb.h>
#endif

#define SETTINGS_TYPE_MAP "qlcftdi/typemap"

class DMXUSBWidget;

class QLCFTDI
{
    /************************************************************************
     * Widget enumeration
     ************************************************************************/
public:
    static const int FTDIVID = 0x0403;    //! FTDI Vendor ID
    static const int ATMELVID = 0x03EB;   //! Atmel Vendor ID
    static const int FTDIPID = 0x6001;    //! FTDI Product ID
    static const int DMX4ALLPID = 0xC850; //! DMX4ALL FTDI Product ID
    static const int NANODMXPID = 0x2018; //! DMX4ALL Nano DMX Product ID

#if defined(FTD2XX)
    static QString readLabel(quint32 id, uchar label, int *ESTA_code);
#elif defined(LIBFTDI) || defined(LIBFTDI1)
    static QString readLabel(struct ftdi_context *ftdi, char *name, char *serial, uchar label, int *ESTA_code);
#elif defined(QTSERIAL)
    static QString readLabel(const QSerialPortInfo &info, uchar label, int *ESTA_code);
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

    /************************************************************************
     * FTDI Interface Methods
     ************************************************************************/
public:
    /** Open the widget */
    bool open();

    /** Open the widget using a specific Product ID */
    bool openByPID(const int FTDIPID);

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

private:
#if defined(FTD2XX)
    FT_HANDLE m_handle;
#elif defined(LIBFTDI) || defined(LIBFTDI1)
    struct ftdi_context m_handle;
#elif defined(QTSERIAL)
    QSerialPort *m_handle;
#endif
};

#endif
