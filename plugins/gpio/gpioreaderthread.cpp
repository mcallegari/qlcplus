/*
  Q Light Controller Plus
  gpioreaderthread.cpp

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

#include <QDebug>

#include "gpioreaderthread.h"
#include "gpioplugin.h"

#define HYSTERESIS_THRESHOLD  3

ReadThread::ReadThread(GPIOPlugin *plugin, QObject *parent)
    : QThread(parent)
    , m_plugin(plugin)
    , m_running(false)
    , m_paused(false)
{
    updateReadPINs();
    start();
}

ReadThread::~ReadThread()
{
    stop();
}

void ReadThread::stop()
{
    qDebug() << Q_FUNC_INFO;
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

void ReadThread::pause(bool paused)
{
    qDebug() << Q_FUNC_INFO << paused;
    QMutexLocker locker(&m_mutex);
    m_paused = paused;
}

void ReadThread::updateReadPINs()
{
    qDebug() << Q_FUNC_INFO;
    QMutexLocker locker(&m_mutex);

    QList<GPIOPinInfo *> gpioList = m_plugin->gpioList();

    m_readList.clear();
    foreach (GPIOPinInfo *gpio, gpioList)
    {
        if (gpio->m_usage == GPIOPlugin::InputUsage)
        {
            if (gpio->m_file != NULL)
            {
                if (gpio->m_file->isOpen() == false)
                {
                    if (!gpio->m_file->open(QIODevice::ReadOnly))
                    {
                        qDebug() << "[GPIO] Error opening GPIO for reading";
                        continue;
                    }
                }

                m_readList.append(gpio);
            }
        }
    }
}

void ReadThread::run()
{
    qDebug() << "[GPIO] Reader thread created";
    m_running = true;
    while (m_running == true)
    {
        if (m_paused == true)
        {
            usleep(50000);
            continue;
        }

        QMutexLocker locker(&m_mutex);

        foreach(GPIOPinInfo *gpio, m_readList)
        {
            if (gpio->m_file == NULL || gpio->m_file->isOpen() == false)
                continue;

            //qDebug() << "Reading file:" << gpio->m_file->fileName();
            gpio->m_file->reset();
            QByteArray dataRead = gpio->m_file->readAll().simplified();
            if (dataRead.isEmpty())
                continue;
            uchar newVal = dataRead.toUInt();
            if (newVal != gpio->m_value)
            {
                gpio->m_count++;
                if (gpio->m_count > HYSTERESIS_THRESHOLD) 
                {
                    qDebug() << "Value read: GPIO:" << gpio->m_number << "val:" <<  newVal;
                    gpio->m_value = newVal;
                    gpio->m_count = 0;
                    emit valueChanged(gpio->m_number, gpio->m_value);
                }
            } 
            else 
            {
                gpio->m_count = 0;
            }
        }
        locker.unlock();

        usleep(50000);
    }
}

