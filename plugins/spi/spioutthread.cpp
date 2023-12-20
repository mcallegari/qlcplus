/*
  Q Light Controller Plus
  spioutthread.cpp

  Copyright (c) Massimo Callegari

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

#include <QMutexLocker>
#include <QSettings>
#include <QDebug>

#include <time.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "spioutthread.h"

SPIOutThread::SPIOutThread()
    : m_isRunning(false)
    , m_dataSize(0)
    , m_estimatedWireTime(50000)
{

}

void SPIOutThread::runThread(int fd, int speed)
{
    if (fd < 0)
        return;

    m_spifd = fd;
    m_speed = speed;

    m_bitsPerWord = 8;
    int mode = SPI_MODE_0;
    int status = -1;

    status = ioctl (m_spifd, SPI_IOC_WR_MODE, &mode);
    if (status < 0)
        qWarning() << "Could not set SPIMode (WR)...ioctl fail";

    status = ioctl (m_spifd, SPI_IOC_WR_BITS_PER_WORD, &m_bitsPerWord);
    if (status < 0)
        qWarning() << "Could not set SPI bitsPerWord (WR)...ioctl fail";

    status = ioctl (m_spifd, SPI_IOC_WR_MAX_SPEED_HZ, &m_speed);
    if (status < 0)
        qWarning() << "Could not set SPI speed (WR)...ioctl fail";

    m_isRunning = true;
    start();
}

void SPIOutThread::stopThread()
{
    m_isRunning = false;
    wait();
}

void SPIOutThread::setSpeed(int speed)
{
    if (speed == m_speed)
        return;

    if (isRunning())
    {
        m_isRunning = false;
        wait();
        runThread(m_spifd, speed);
    }
}

void SPIOutThread::run()
{
    while (m_isRunning)
    {
        struct spi_ioc_transfer spi;

        struct timespec ts_start;
        struct timespec ts_end;
#ifdef CLOCK_MONOTONIC
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
#else
        clock_gettime(CLOCK_REALTIME, &ts_start);
#endif

        if (m_spifd != -1 && m_pluginData.size() > 0)
        {
            QMutexLocker locker(&m_mutex);
            memset(&spi, 0, sizeof(spi));
            spi.tx_buf        = reinterpret_cast<__u64>(m_pluginData.data());
            spi.len           = m_pluginData.size();
            spi.delay_usecs   = 0;
            spi.speed_hz      = m_speed;
            spi.bits_per_word = m_bitsPerWord;
            spi.cs_change = 0;

            int retVal = ioctl(m_spifd, SPI_IOC_MESSAGE(1), &spi);
            if (retVal < 0)
                qWarning() << "Problem transmitting SPI data: ioctl failed";
        }

#ifdef CLOCK_MONOTONIC
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
#else
        clock_gettime(CLOCK_REALTIME, &ts_end);
#endif
        int uSecDiff = (difftime(ts_end.tv_sec, ts_start.tv_sec) * 1000000) + ((ts_end.tv_nsec - ts_start.tv_nsec) / 1000);
        //qDebug() << "[SPI] ioctl took" << microseconds << "uSecDiff";
        usleep(m_estimatedWireTime - uSecDiff);
    }
}

void SPIOutThread::writeData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_pluginData = data;
    if (m_dataSize != data.size())
    {
        // Data size has changed! I need to estimate the
        // time that the SPI writes will take on the wire.
        // The estimation is very unprecise and it is based
        // on Simon Newton's measurements on a Raspberry Pi
        // where 512 bytes at 1Mhz takes about 70ms to be sent

        double byteWriteTimeuS = (double)(70000.0 / ((double)m_speed / 1000000.0)) / 512.0; // time taken to write a byte
        m_estimatedWireTime = byteWriteTimeuS * (double)data.size();
        m_dataSize = data.size();
        qDebug() << "[SPI out thread] estimated sleep time:" << m_estimatedWireTime;
    }
}

