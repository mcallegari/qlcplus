/*
  Q Light Controller Plus
  dmxusbopenrx.h

  Copyright (C) Massimo Callegari
                Emmanuel Coirier

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

#ifndef DMXUSBOPENRX_H
#define DMXUSBOPENRX_H

#include <QByteArray>
#include <QThread>

#include "dmxusbwidget.h"

class DMXUSBOpenRx : public QThread, public DMXUSBWidget
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /**
     * @param interface dmx interface
     * @param outputline line number
     * @param parent The owner of this object
     */
    DMXUSBOpenRx(DMXInterface *iface,
                     quint32 inputLine, QObject* parent = 0);

    /** Destructor */
    virtual ~DMXUSBOpenRx();

    /** @reimp */
    DMXUSBWidget::Type type() const;

    /************************************************************************
     * Open & Close
     ************************************************************************/
public:
    /** @reimp */
    bool open(quint32 line = 0, bool input = false);

    /** @reimp */
    bool close(quint32 line = 0, bool input = false);

    /************************************************************************
     * Name & Serial
     ************************************************************************/
public:
    /** @reimp */
    QString additionalInfo() const;

    /************************************************************************
     * Thread
     ************************************************************************/
public:
    /** @reimp */
    bool writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged);

protected:
    enum TimerGranularity { Unknown, Good, Bad };
    enum ReaderState { Calibrating, Idling, Receiving };

    /** Stop the writer thread */
    void stop();

    /** DMX writer thread worker method */
    void run();

    /** Compare frame and only emit differences */
    void compareAndEmit(const QByteArray& last_payload, const QByteArray& current_payload);

protected:
    bool m_running;
    TimerGranularity m_granularity;
    ReaderState m_reader_state;

signals:
    /** Tells that the value of a received DMX channel has changed */
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value);
};

#endif
