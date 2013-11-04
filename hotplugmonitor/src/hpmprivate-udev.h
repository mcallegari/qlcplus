/*
  Q Light Controller
  hpmprivate-udev.h

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

#ifndef HPMPRIVATE_UDEV_H
#define HPMPRIVATE_UDEV_H

#include <QThread>

class HotPlugMonitor;

class HPMPrivate : public QThread
{
    Q_OBJECT

public:
    HPMPrivate(HotPlugMonitor* parent);
    ~HPMPrivate();

public slots:
    void stop();

private:
    void run();

private:
    HotPlugMonitor* m_hpm;
    bool m_run;
};

#endif
