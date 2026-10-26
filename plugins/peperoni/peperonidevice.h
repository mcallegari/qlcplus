/*
  Q Light Controller
  peperonidevice.h

  Copyright (c) Heikki Junnila
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

#ifndef PEPERONIDEVICE_H
#define PEPERONIDEVICE_H

#include <QThread>
#include <QMutex>
#include <QHash>
#include <libusb.h>

class QString;
class QByteArray;
class Peperoni;

class PeperoniDevice final : public QThread
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    PeperoniDevice(Peperoni *parent, libusb_device *device,
                   libusb_device_descriptor *desc, quint32 line);
    virtual ~PeperoniDevice();

    /** Find out, whether the given USB device is a Peperoni device */
    static bool isPeperoniDevice(const libusb_device_descriptor *desc);

    /** Find out, whether the given USB VID/PID pair corresponds to a Peperoni device */
    static bool isPeperoniDevice(int vid, int pid);

    /** Returns the number of output universes this device supports */
    static int outputsNumber(const libusb_device_descriptor *desc);

    /** Returns the number of lines (universes) this device occupies */
    int linesCount() const;

    /** Returns the first plugin line used by this device */
    quint32 baseLine() const;

    /** Set the first plugin line used by this device (on rescan) */
    void setBaseLine(quint32 line);

    /********************************************************************
     * Device information
     ********************************************************************/
public:
    QString name(quint32 line) const;
    QString baseInfoText(quint32 line) const;
    QString inputInfoText(quint32 line) const;
    QString outputInfoText(quint32 line) const;

protected:
    void extractName();

    /** Model name derived from the USB product ID */
    QString modelName() const;

protected:
    /** The interface name */
    QString m_name;

    /** The interface name */
    QString m_serial;

    /** Base line of this interface */
    quint32 m_baseLine;

    /** Mutex to synchronize input and output at the same time */
    QMutex m_ioMutex;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    /** Interface operational modes */
    enum OperatingMode
    {
        CloseMode  = 1 << 0,
        OutputMode = 1 << 1,
        InputMode  = 1 << 2
    };

    bool open(quint32 line, OperatingMode mode);
    void close(quint32 line, OperatingMode mode);
    void closeAll();

    const libusb_device *device() const;

protected:
    /** Convert an absolute plugin line to this device's universe index */
    quint32 lineIndex(quint32 line) const;

    /** The device operating mode for each universe index (0 or 1) */
    QHash<quint32, int> m_operatingModes;

    struct libusb_device* m_device;
    struct libusb_device_handle* m_handle;
    struct libusb_device_descriptor m_descriptor;
    int m_firmwareVersion;
    QByteArray m_bulkBuffer;

    /********************************************************************
     * Input worker thread
     ********************************************************************/
protected:
    bool m_running;

    /** Last universe data that has been received */
    QByteArray m_dmxInputBuffer;

private:
    /** @reimp */
    void run() override;

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

    /********************************************************************
     * Write
     ********************************************************************/
public:
    void outputDMX(quint32 line, const QByteArray& universe);
};

#endif
