/*
  Q Light Controller
  enttecdmxusbpro.h

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

#ifndef ENTTECDMXUSBPRO_H
#define ENTTECDMXUSBPRO_H

#include <QByteArray>
#include <QThread>

#include "dmxusbwidget.h"

#define ENTTEC_PRO_DMX_ZERO      char(0x00)
#define ENTTEC_PRO_RECV_DMX_PKT  char(0x05)
#define ENTTEC_PRO_SEND_DMX_RQ   char(0x06)
#define ENTTEC_PRO_READ_SERIAL   char(0x0A)
#define ENTTEC_PRO_ENABLE_API2   char(0x0D)
#define ENTTEC_PRO_SEND_DMX_RQ2  char(0xA9)
#define ENTTEC_PRO_PORT_ASS_REQ  char(0xCB)
#define ENTTEC_PRO_START_OF_MSG  char(0x7E)
#define ENTTEC_PRO_END_OF_MSG    char(0xE7)
#define ENTTEC_PRO_MIDI_OUT_MSG  char(0xBE)
#define ENTTEC_PRO_MIDI_IN_MSG   0xE8

#define DMXKING_ESTA_ID          0x6A6B
#define ULTRADMX_DMX512A_DEV_ID  0x00
#define ULTRADMX_PRO_DEV_ID      0x02
#define ULTRADMX_MICRO_DEV_ID    0x03

#define DMXKING_USB_DEVICE_MANUFACTURER 0x4D
#define DMXKING_USB_DEVICE_NAME         0x4E
#define DMXKING_SEND_DMX_PORT1          char(0x64)
#define DMXKING_SEND_DMX_PORT2          char(0x65)

/**
 * This is the base interface class for ENTTEC USB DMX Pro widgets.
 */
class EnttecDMXUSBPro : public QThread, public DMXUSBWidget
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    EnttecDMXUSBPro(DMXInterface *interface,
                    quint32 outputLine, quint32 inputLine = 0);

    virtual ~EnttecDMXUSBPro();

    /** @reimp */
    Type type() const;

    void setMidiPortsNumber(int inputs, int outputs);

    void setDMXKingMode();

    /** @reimp */
    QString additionalInfo() const;

private:
    QHash<quint32, ushort> m_midiInputsMap;
    QHash<quint32, ushort> m_mididOutputsMap;
    bool m_dmxKingMode;

    /****************************************************************************
     * Open & Close
     ****************************************************************************/
private:
     bool configureLine(ushort dmxLine, ushort midiLine);

public:
    /** @reimp */
    virtual bool open(quint32 line, bool input = false);

     /** @reimp */
     virtual bool close(quint32 line = 0, bool input = false);

    /************************************************************************
     * Name & Serial
     ************************************************************************/
public:
    /** @reimp */
    QString uniqueName(ushort line = 0, bool input = false) const;

private:
    /** Extract the widget's unique serial number (printed on the bottom) */
    bool extractSerial();

private:
    QString m_proSerial;

    /************************************************************************
     * DMX reception
     ************************************************************************/
signals:
    /** Tells that the value of a received DMX channel has changed */
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value);

private:
    /** Stop DMX receiver thread */
    void stopThread();

    /** DMX receiver thread worker method */
    void run();

private:
    bool m_running;
    QMutex m_mutex;
    QByteArray m_universe;

    /************************************************************************
     * Write universe
     ************************************************************************/
public:
    /** @reimp */
    bool writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

private:
    QByteArray m_outUniverse;
};

#endif
