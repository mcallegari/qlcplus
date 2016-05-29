/*
  Q Light Controller Plus
  spioutthread.h

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

#ifndef SPIOUTTHREAD_H
#define SPIOUTTHREAD_H

#include <QThread>

class SPIOutThread : public QThread
{
public:
    SPIOutThread();

    void runThread(int fd, int speed);
    void stopThread();
    void setSpeed(int speed);

    void run();

    void writeData(const QByteArray& data);

protected:
    /** File handle for /dev/spidev0.0 */
    int m_spifd;
    int m_bitsPerWord;
    int m_speed;

    bool m_isRunning;

    /** Copy of data received from the SPI plugin */
    QByteArray m_pluginData;

    /** Last size of data sent to the SPI bus */
    int m_dataSize;

    /** Roughly estimated time that SPI writes will take on the wire (in uS) */
    quint32 m_estimatedWireTime;

    /** Mutex used to synchronize data between the SPI plugin
     *  and the output thread */
    QMutex m_mutex;
};

#endif // SPIOUTTHREAD_H
