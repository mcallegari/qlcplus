/*
  Q Light Controller PLus
  dmxusbwidget.h

  Copyright (C) Heikki Junnila
  Copyright (C) Massimo Callegari

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

#ifndef DMXUSBWIDGET_H
#define DMXUSBWIDGET_H

#include <QElapsedTimer>

#if defined(FTD2XX)
  #include "ftd2xx-interface.h"
#endif
#if defined(LIBFTDI) || defined(LIBFTDI1)
  #include "libftdi-interface.h"
#endif
#if defined(QTSERIAL)
  #include "qtserial-interface.h"
#endif

#define DMX_CHANNELS                512
#define DEFAULT_OUTPUT_FREQUENCY    44  // 44 Hertz, according to the DMX specs

/**
 * This is the base interface class for all the USB DMX widgets.
 * It represents a DMX USB widget, with all its specific input/output lines
 */
class DMXUSBWidget
{
public:
    /**
     * Construct a new DMXUSBWidget object.
     *
     * @param interface The widget's DMXInterface instance
     * @param outputLine the specific output line this widget is going to control
     */
    DMXUSBWidget(DMXInterface *iface, quint32 outputLine, int frequency);

    virtual ~DMXUSBWidget();

    /** Widget types */
    enum Type
    {
        ProRXTX,    //! Enttec Pro widget using the TX/RX features of the dongle
        OpenTX,     //! Enttec Open widget (only TX)
        OpenRX,     //! FTDI DMX widget with RX capabilities (only RX, use OpenTX for TX)
        ProMk2,     //! Enttec Pro Mk2 widget using 2 TX, 1 RX, 1 MIDI TX and 1 MIDI RX ports
        UltraPro,   //! DMXKing Ultra Pro widget using 2 TX and 1RX ports
        DMX4ALL,    //! DMX4ALL widget (only TX)
        VinceTX,    //! Vince USB-DMX512 widget using the TX side of the dongle
        Eurolite    //! Eurolite USB DMX512 Pro widget
    };

    /** The possible features of a line */
    enum LineFlags
    {
        None   = 0,
        DMX    = 1 << 0,
        MIDI   = 1 << 1,
        Input  = 1 << 2,
        Output = 1 << 3,
        ArtNet_sACN_Forward = 1 << 4
    };

    struct DMXUSBLineInfo
    {
        /** The device line flags (DMX, MIDI, etc) */
        int m_portFlags;

        /** When open, this is the port directon */
        int m_openDirection;

        /** Data for input/output */
        QByteArray m_universeData;

        /** Data for comparison with m_universeData */
        QByteArray m_compareData;
    };

    /** Get the type of the widget */
    virtual Type type() const = 0;

    /** Get the DMXInterface instance */
    DMXInterface *iface() const;

    /** Get the DMXInterface driver in use as a string */
    QString interfaceTypeString() const;

    static QList<DMXUSBWidget *> widgets();

    bool forceInterfaceDriver(DMXInterface::Type type);

private:
    DMXInterface *m_interface;
	
#if defined(WIN32) || defined(Q_OS_WIN)
    /** Resolution (in ms) that Windows system timer will be set to */
    static uint s_windowsTimerResolution;
    
    /** Request a minimum resolution (in ms) for Windows system timer */
    bool setWindowsTimerResolution(uint resolution);
    
    /** Clear a previously set minimum Windows system timer resolution */
    bool clearWindowsTimerResolution(uint resolution);
#endif

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    /**
     * Open widget for further operations, such as serial() and writeUniverse()
     *
     * @return true if widget was opened successfully (or was already open)
     */
    virtual bool open(quint32 line = 0, bool input = false);

    /**
     * Close widget, preventing any further operations
     *
     * @param true if widget was closed successfully (or was already closed)
     */
    virtual bool close(quint32 line = 0, bool input = false);

    /**
     * Check, whether widget has been opened
     *
     * @return true if widget is open, otherwise false
     */
    virtual bool isOpen();

    /********************************************************************
     * Widget Ports
     ********************************************************************/
public:
    virtual void setPortsMapping(QList<int> ports);

    virtual int portFlagsCount(LineFlags flags);

    virtual int openPortsCount();

    virtual quint32 lineToPortIndex(quint32 line, int type);

protected:
    /** The QLC+ output line number where this widget outputs start */
    quint32 m_outputBaseLine;

    /** The QLC+ input line number where this widget inputs start */
    quint32 m_inputBaseLine;

    QVector<DMXUSBLineInfo> m_portsInfo;

    /********************************************************************
     * Outputs
     ********************************************************************/
public:
    /** Return the number of output lines supported by this widget */
    virtual int outputsNumber();

    /** Return a list of the output line names */
    virtual QStringList outputNames();

    /** Get/Set the output frequency rate to be used by this widget */
    virtual int outputFrequency();
    virtual void setOutputFrequency(int frequency);

protected:
    /** The output frequency in Hertz */
    int m_frequency;

    /** The DMX frame time duration in microseconds */
    int m_frameTimeUs;

    /********************************************************************
     * Inputs
     ********************************************************************/
public:
    /** Return the number of input lines supported by this widget */
    virtual int inputsNumber();

    /** Return a list of the input line names */
    virtual QStringList inputNames();

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
    virtual QString uniqueName(ushort line = 0, bool input = false) const;

    /** Set the real device name extracted from serial using label 78 */
    void setRealName(QString devName);

    /** Retrieve the real device name read from label 78 */
    virtual QString realName() const;

    /**
     * Get the widget's vendor name
     *
     * @return widget's vendor
     */
    virtual QString vendor() const;

    /** Get any additional information pertaining to the device (can be empty) */
    virtual QString additionalInfo() const { return QString(); }

private:
    QString m_realName;

    /********************************************************************
     * RDM
     ********************************************************************/
public:
    virtual bool supportRDM();

    virtual bool sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params);

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
    virtual bool writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged);
};

#endif
