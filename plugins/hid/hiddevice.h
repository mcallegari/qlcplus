/*
  Q Light Controller
  hiddevice.h

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

#ifndef HIDDEVICE_H
#define HIDDEVICE_H

#include <QThread>
#include <QFile>

class HIDPlugin;

/*****************************************************************************
 * HIDDevice
 *****************************************************************************/

class HIDDevice : public QThread
{
    Q_OBJECT

public:
    HIDDevice(HIDPlugin* parent, quint32 line, const QString& name, const QString& path);
    virtual ~HIDDevice();

    /*************************************************************************
     * File operations
     *************************************************************************/
public:
    /**
     * Check if the device offers a built-in merger mode.
     *
     * Merger mode means that all DMX data from the device's input port is
     * copied to its output port and - if output from QLC+ is enabled -
     * merged with this output in a HTP manner on the device itself. 
     *
     * @return true if the device offers a merger mode, false otherwise
     */
    virtual bool hasMergerMode();

    /**
     * Check if device's built-in merger mode is enabled.
     *
     * @return true if the device's merger mode is enabled, false otherwise
     */
    virtual bool isMergerModeEnabled();

    /**
     * Enable or disable the built-in merger mode.
     */
    virtual void enableMergerMode(bool mergerModeEnabled);

    /**
     * Attempt to open the HID device as input in RW mode and fall back
     * to RO if that fails.
     *
     * @return true if the file was opened RW/RO
     */
    virtual bool openInput();

    /**
     * Close the HID device's input
     */
    virtual void closeInput();

     /**
     * Open HID device as output
     *
     */
    virtual bool openOutput();

    /**
     * Close the HID device's output
     */
    virtual void closeOutput();

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
    QString m_filename;
    QFile m_file;

    /*************************************************************************
     * Line
     *************************************************************************/
public:
    quint32 line() const {
        return m_line;
    }

    virtual bool hasInput() { return false; }
    virtual bool hasOutput() { return false; }

protected:
    quint32 m_line;
    int m_capabilities;

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
     * @param universe The universe where the event happened
     * @param line The input line that received the signal
     * @param channel The channel whose value has changed
     * @param value The changed value
     */
    void valueChanged(quint32 universe, quint32 line, quint32 channel, uchar value);

public:
    /**
     * Send an input value back the HID device to move motorized sliders
     * and such.
     */
    virtual void feedBack(quint32 channel, uchar value);

protected:
    bool m_running;

private:
    /** Input data thread worker method */
    virtual void run();

    /*************************************************************************
     * Output data
     *************************************************************************/
public:

    /** Output data, which is a DMX universe */
    virtual void outputDMX(const QByteArray &data, bool forceWrite = false);
};

#endif
