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
#include <QTime>

#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "spioutthread.h"

SPIOutThread::SPIOutThread()
    : m_isRunning(false)
    , m_dataSize(0)
    , m_estimatedSleepTime(50000)
{

}

void SPIOutThread::runThread(int fd)
{
    if (fd < 0)
        return;

    m_spifd = fd;

    QSettings settings;
    QVariant value = settings.value("SPIPlugin/frequency");
    if (value.isValid() == true)
        m_speed = value.toUInt();
    else
        m_speed = 1000000;

    m_bitsPerWord = 8;
    int mode = SPI_MODE_0;
    int status = -1;

    status = ioctl (m_spifd, SPI_IOC_WR_MODE, &mode);
    if(status < 0)
        qWarning() << "Could not set SPIMode (WR)...ioctl fail";

    status = ioctl (m_spifd, SPI_IOC_WR_BITS_PER_WORD, &m_bitsPerWord);
    if(status < 0)
        qWarning() << "Could not set SPI bitsPerWord (WR)...ioctl fail";

    status = ioctl (m_spifd, SPI_IOC_WR_MAX_SPEED_HZ, &m_speed);
    if(status < 0)
        qWarning() << "Could not set SPI speed (WR)...ioctl fail";

    m_isRunning = true;
    start();
}

void SPIOutThread::stopThread()
{
    m_isRunning = false;
    wait();
}

void SPIOutThread::run()
{
    while(m_isRunning)
    {
        struct spi_ioc_transfer spi;
        int retVal = -1;

        QTime elapsedTime;
        elapsedTime.start();

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

            retVal = ioctl(m_spifd, SPI_IOC_MESSAGE(1), &spi);
            if(retVal < 0)
                qWarning() << "Problem transmitting SPI data: ioctl failed";
        }

        int nMilliseconds = elapsedTime.elapsed();
        //qDebug() << "[SPI] ioctl took" << nMilliseconds << "ms";
        usleep(m_estimatedSleepTime - nMilliseconds);
    }
}

void SPIOutThread::writeData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_pluginData = data;
    if (m_dataSize != data.size())
    {
        // Data size has changed ! I need to estimate the
        // time to sleep between SPI writes
        // The estimation is very unprecise and it is based
        // on Simon Newton's measurements on a Raspberry Pi
        // where 512 bytes at 1Mhz takes about 80ms to be sent

        double byteWriteTimeuS = (double)(70000 / (m_speed / 1000000)) / 512; // time taken to write a byte
        m_estimatedSleepTime = byteWriteTimeuS * (double)data.size();
        m_dataSize = data.size();
        qDebug() << "[SPI out thread] estimated sleep time:" << m_estimatedSleepTime;
    }
}

