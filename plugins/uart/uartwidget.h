/*
  Q Light Controller Plus
  uartwidget.h

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

#ifndef UARTWIDGET_H
#define UARTWIDGET_H

#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <QThread>

class UARTWidget : public QThread
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /**
     * Construct a new UARTWidget object with the given parent and
     * QSerialPort information. Neither can be null.
     *
     * @param info the serial port descriptor
     * @param parent The owner of this object
     */
    UARTWidget(QSerialPortInfo &info, QObject* parent = 0);

    /** Destructor */
    virtual ~UARTWidget();

    QString name() const;

    /************************************************************************
     * Open/Close
     ************************************************************************/
public:
    /** Interface mode specification */
    enum WidgetMode
    {
        Closed = 1 << 0,
        Output = 1 << 1,
        Input  = 1 << 2
    };

    bool open(WidgetMode mode);

    bool close(WidgetMode mode);

protected:
    void updateMode();

    /** The device current open mode */
    int m_mode;

    /************************************************************************
     * Thread methods
     ************************************************************************/
public:
    void writeUniverse(const QByteArray& data);

protected:
    enum TimerGranularity { Unknown, Good, Bad };

    /** Stop the writer thread */
    void stop();

    /** DMX writer thread worker method */
    void run();

protected:
    bool m_running;
    QByteArray m_outputBuffer;
    QByteArray m_inputBuffer;
    double m_frequency;
    TimerGranularity m_granularity;

    QSerialPortInfo m_serialInfo;
    QSerialPort *m_serialPort;
};

#endif
