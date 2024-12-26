/*
  Q Light Controller Plus
  gpioreaderthread.h

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

#ifndef GPIOREADERTHREAD_H
#define GPIOREADERTHREAD_H

#include <QThread>
#include <QMutexLocker>

#include "gpioplugin.h"

class ReadThread : public QThread
{
    Q_OBJECT

public:
    ReadThread(GPIOPlugin *plugin, QObject *parent = 0);

    /** Destructor */
    virtual ~ReadThread();

    void stop();
    void pause(bool paused);

protected:
    void run();

signals:
    void valueChanged(quint32 channel, uchar value);

private:
    GPIOPlugin *m_plugin;
    bool m_running;
    bool m_paused;
    QMutex m_mutex;
};

#endif
