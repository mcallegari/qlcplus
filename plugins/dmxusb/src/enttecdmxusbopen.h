/*
  Q Light Controller
  enttecdmxusbopen.h

  Copyright (C) Heikki Junnila
                Christopher Staite

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

#ifndef ENTTECDMXUSBOPEN_H
#define ENTTECDMXUSBOPEN_H

#include <QByteArray>
#include <QThread>

#include "dmxusbwidget.h"

class EnttecDMXUSBOpen : public QThread, public DMXUSBWidget
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /**
     * Construct a new DMXUSBOpen object with the given parent and
     * FTDI device information. Neither can be null.
     *
     * @param serial The device's unique(ish) serial
     * @param name The friendly name of the device
     * @param id The device's unique ID (FTD2XX only)
     * @param parent The owner of this object
     */
    EnttecDMXUSBOpen(DMXInterface *iface,
                     quint32 outputLine, QObject* parent = 0);

    /** Destructor */
    virtual ~EnttecDMXUSBOpen();

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

    /** Stop the writer thread */
    void stop();

    /** DMX writer thread worker method */
    void run();

protected:
    bool m_running;
    TimerGranularity m_granularity;
};

#endif
