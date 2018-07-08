/*
  Q Light Controller
  hpmtest.cpp

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

#include <QListWidget>
#include <QLayout>

#include "hotplugmonitor.h"
#include "hpmtest.h"

HPMTest::HPMTest(QWidget* parent)
    : QWidget(parent)
{
    new QHBoxLayout(this);
    m_list = new QListWidget(this);
    layout()->addWidget(m_list);

    HotPlugMonitor::connectListener(this);
}

HPMTest::~HPMTest()
{
}

void HPMTest::slotDeviceAdded(uint vid, uint pid)
{
    m_list->addItem(QString("%1: VID %2, PID %3").arg("Added")
                                                 .arg(QString::number(vid, 16))
                                                 .arg(QString::number(pid, 16)));
}

void HPMTest::slotDeviceRemoved(uint vid, uint pid)
{
    m_list->addItem(QString("%1: VID %2, PID %3").arg("Removed")
                                                 .arg(QString::number(vid, 16))
                                                 .arg(QString::number(pid, 16)));
}
