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

#include <gpiod.hpp>

#include <filesystem>
#include <system_error>

#include "gpioreaderthread.h"
#include "gpioplugin.h"

#define HYSTERESIS_THRESHOLD  3

ReadThread::ReadThread(GPIOPlugin *plugin, QObject *parent)
    : QThread(parent)
    , m_plugin(plugin)
    , m_running(false)
    , m_paused(false)
{
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

void ReadThread::run()
{
    qDebug() << "[GPIO] Reader thread created";
    m_running = true;

    try
    {
        ::gpiod::chip gChip{std::filesystem::path(m_plugin->chipName())};

        while (m_running == true)
        {
            if (m_paused == true)
            {
                usleep(50000);
                continue;
            }

            QMutexLocker locker(&m_mutex);

            foreach (GPIOLineInfo *gpio, m_plugin->gpioList())
            {
                if (gpio->m_direction != GPIOPlugin::InputDirection)
                    continue;

                ::gpiod::line_settings settings;
                settings.set_direction(::gpiod::line::direction::INPUT);

                auto request = gChip.prepare_request()
                    .set_consumer("get_value")
                    .add_line_settings(gpio->m_line, settings)
                    .do_request();

                int newVal = request.get_value(gpio->m_line) == ::gpiod::line::value::ACTIVE ? 1 : 0;
                request.release();

                if (newVal != gpio->m_value)
                {
                    gpio->m_count++;
                    if (gpio->m_count > HYSTERESIS_THRESHOLD)
                    {
                        qDebug() << "Value read: GPIO:" << gpio->m_line << "val:" <<  newVal;
                        gpio->m_value = newVal ? UCHAR_MAX : 0;
                        gpio->m_count = 0;
                        emit valueChanged(gpio->m_line, gpio->m_value);
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
    catch (const std::system_error &e)
    {
        qWarning() << "GPIO reader thread failed to open chip:" << e.what();
    }
}
