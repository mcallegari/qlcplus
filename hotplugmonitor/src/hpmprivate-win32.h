/*
  Q Light Controller
  hpmprivate-win32.h

  Copyright (c) Heikki Junnila

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

#ifndef HPMPRIVATE_WIN32_H
#define HPMPRIVATE_WIN32_H

#include <Windows.h>
#include <QObject>
#include <qwindowdefs.h>

class HotPlugMonitor;

/****************************************************************************
 * HPMPrivate declaration
 ****************************************************************************/

class HPMPrivate: public QObject
{
    Q_OBJECT

public:
    HPMPrivate(HotPlugMonitor* parent);
    virtual ~HPMPrivate();

    void start();
    void stop();

    void setWinId(WId id);
    bool processWinEvent(MSG* message, long* result);

protected:
    static bool extractVidPid(const QString& dbccName, uint* vid, uint* pid);

private:
    HotPlugMonitor* m_hpm;
    HDEVNOTIFY m_hDeviceNotify;
};

#endif
