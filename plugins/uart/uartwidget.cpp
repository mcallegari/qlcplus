/*
  Q Light Controller Plus
  uartwidget.cpp

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

#include <QDebug>
#include <QElapsedTimer>
#include <math.h>

#include <sys/ioctl.h>
#include <asm/termbits.h>

#include "uartwidget.h"

#define DMX_MAB 16
#define DMX_BREAK 110

UARTWidget::UARTWidget(QSerialPortInfo &info, QObject *parent)
    : QThread(parent)
    , m_running(false)
    , m_granularity(Unknown)
{
    m_serialInfo = info;
}

UARTWidget::~UARTWidget()
{
    stop();
}

QString UARTWidget::name() const
{
    return m_serialInfo.portName();
}

bool UARTWidget::open(UARTWidget::WidgetMode mode)
{
    if (mode == Output)
        m_outputBuffer.fill(0, 513);
    else if (mode == Input)
        m_inputBuffer.fill(0, 513);

    m_mode |= mode;
    updateMode();

    return true;
}

bool UARTWidget::close(UARTWidget::WidgetMode mode)
{
    m_mode &= ~mode;
    updateMode();

    return true;
}

void UARTWidget::updateMode()
{
    if (m_mode != Closed && m_running == false)
        start();
    else if (m_mode == Closed && m_running == true)
        stop();
}

void UARTWidget::writeUniverse(const QByteArray &data)
{
    m_outputBuffer.replace(1, qMin(data.size(), m_outputBuffer.size()), data);
}

void UARTWidget::stop()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

void UARTWidget::run()
{
    qDebug() << "[UARTWidget] Thread created.";
    m_serialPort = new QSerialPort(m_serialInfo);

    m_serialPort->setBaudRate(250000);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setStopBits(QSerialPort::TwoStop);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort->open(QIODevice::ReadWrite))
    {
        qWarning() << QString("[UARTWidget] Failed to open port %1, error: %2")
                      .arg(m_serialInfo.portName()).arg(m_serialPort->errorString());
        return;
    }

// If the "new" custom baud rate method is available, then use it to
// make sure the baud rate is properly set to 250Kbps

#if defined(TCGETS2)
    int fd = m_serialPort->handle();
    static const int rate = 250000;

    struct termios2 tio;  // linux-specific terminal stuff

    if (ioctl(fd, TCGETS2, &tio) < 0)
    {
        qDebug() << "[UARTWidget] Error in getting termios2 data";
        return;
    }

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = rate;
    tio.c_ospeed = rate;  // set custom speed directly
    if (ioctl(fd, TCSETS2, &tio) < 0)
    {
        qDebug() << "[UARTWidget] Error in setting termios2 data";
        return;
    }
#endif

    m_serialPort->clear();
    m_serialPort->setRequestToSend(false);

    // One "official" DMX frame can take (1s/44Hz) = 23ms
    int frameTime = (int) floor(((double)1000 / 30) + (double)0.5);
    m_granularity = Bad;

    QElapsedTimer time;
    time.start();
    usleep(1000);
    if (time.elapsed() <= 3)
        m_granularity = Good;

    m_running = true;
    while (m_running == true)
    {
        time.restart();

        if (m_mode & Output)
        {
            m_serialPort->setBreakEnabled(true);
            if (m_granularity == Good)
                usleep(DMX_BREAK);
            m_serialPort->setBreakEnabled(false);
            if (m_granularity == Good)
                usleep(DMX_MAB);

            if (m_serialPort->write(m_outputBuffer) == 0)
                qDebug() << "[UARTWidget] Error in writing output buffer";
            m_serialPort->waitForBytesWritten(10);

            // Sleep for the rest of the DMX frame time
            if (m_granularity == Good)
                while (time.elapsed() < frameTime) { usleep(1000); }
            else
                while (time.elapsed() < frameTime) { /* Busy sleep */ }
        }
    }
}
