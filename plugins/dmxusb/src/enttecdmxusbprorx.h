/*
  Q Light Controller
  enttecdmxusbprorx.cpp

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

#ifndef ENTTECDMXUSBPRORX_H
#define ENTTECDMXUSBPRORX_H

#include <QThread>
#include <QMutex>
#include "enttecdmxusbpro.h"

class EnttecDMXUSBProRX : public QThread, public EnttecDMXUSBPro
{
    Q_OBJECT

public:
    EnttecDMXUSBProRX(const QString& serial, const QString& name, const QString& vendor,
                      quint32 input, QLCFTDI *ftdi = NULL, quint32 id = 0);
    ~EnttecDMXUSBProRX();

    /** @reimp */
    Type type() const;

private:
    const quint32 m_input;

    /************************************************************************
     * Open & Close
     ************************************************************************/
public:
    /** @reimp */
    bool open();

    /** @reimp */
    bool close();

    /************************************************************************
     * Name & Serial
     ************************************************************************/
public:
    /** @reimp */
    QString additionalInfo() const;

    /************************************************************************
     * DMX reception
     ************************************************************************/
    /** @reimp */
    QString uniqueName() const;

signals:
    /** Tells that the value of a received DMX channel has changed */
    void valueChanged(quint32 input, quint32 channel, uchar value);

private:
    /** Stop DMX receiver thread */
    void stop();

    /** DMX receiver thread worker method */
    void run();

private:
    bool m_running;
    QMutex m_mutex;
    QByteArray m_universe;
};

#endif
