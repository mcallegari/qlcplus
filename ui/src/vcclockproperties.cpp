/*
  Q Light Controller Plus
  vcclockproperties.cpp

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

#include <QTimeEdit>

#include "vcclockproperties.h"
#include "functionselection.h"

#define KColumnName     0
#define KColumnTime     1

VCClockProperties::VCClockProperties(VCClock *clock, Doc *doc)
    : QDialog(clock)
    , m_clock(clock)
    , m_doc(doc)
{
    Q_ASSERT(clock != NULL);

    setupUi(this);

    switch(m_clock->clockType())
    {
        case VCClock::Stopwatch:
            m_stopWatchRadio->setChecked(true);
        break;
        case VCClock::Countdown:
        {
            m_countdownRadio->setChecked(true);
            m_hoursSpin->setValue(m_clock->getHours());
            m_minutesSpin->setValue(m_clock->getMinutes());
            m_secondsSpin->setValue(m_clock->getSeconds());
        }
        break;
        case VCClock::Clock:
        default:
            m_clockRadio->setChecked(true);
        break;
    }

    foreach(VCClockSchedule sch, m_clock->schedules())
        addScheduleItem(sch);

    connect(m_addScheduleBtn, SIGNAL(clicked()),
            this, SLOT(slotAddSchedule()));
    connect(m_removeScheduleBtn, SIGNAL(clicked()),
            this, SLOT(slotRemoveSchedule()));
}

VCClockProperties::~VCClockProperties()
{

}

void VCClockProperties::addScheduleItem(VCClockSchedule schedule)
{
    quint32 fid = schedule.function();
    if (fid == Function::invalidId())
        return;

    Function *func = m_doc->function(fid);
    if (func != NULL)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_scheduleTree);
        item->setText(KColumnName, func->name());
        item->setIcon(KColumnName, Function::typeToIcon(func->type()));
        item->setData(KColumnName, Qt::UserRole, func->id());
        QTimeEdit *timeEdit = new QTimeEdit();
        timeEdit->setDisplayFormat("HH:mm:ss");
        timeEdit->setTime(schedule.time().time());
        m_scheduleTree->setItemWidget(item, KColumnTime, timeEdit);
    }
    m_scheduleTree->resizeColumnToContents(KColumnName);
}

void VCClockProperties::accept()
{
    if (m_clockRadio->isChecked())
        m_clock->setClockType(VCClock::Clock);
    else if (m_stopWatchRadio->isChecked())
        m_clock->setClockType(VCClock::Stopwatch);
    else if (m_countdownRadio->isChecked())
    {
        m_clock->setClockType(VCClock::Countdown);
        m_clock->setCountdown(m_hoursSpin->value(), m_minutesSpin->value(), m_secondsSpin->value());
    }

    m_clock->removeAllSchedule();
    for (int i = 0; i < m_scheduleTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_scheduleTree->topLevelItem(i);
        VCClockSchedule sch;
        sch.setFunction(item->data(KColumnName, Qt::UserRole).toUInt());
        QTimeEdit *timeEdit = (QTimeEdit *)m_scheduleTree->itemWidget(item, KColumnTime);
        if (timeEdit != NULL)
        {
            QDateTime dt;
            dt.setTime(timeEdit->time());
            sch.setTime(dt);
        }
        m_clock->addSchedule(sch);
    }

    QDialog::accept();
}

void VCClockProperties::slotAddSchedule()
{
    FunctionSelection fs(this, m_doc);

    if (fs.exec() == QDialog::Accepted)
    {
        /* Append selected functions */
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
        {
            VCClockSchedule sch;
            sch.setFunction(it.next());
            sch.setTime(QDateTime());
            addScheduleItem(sch);
        }
    }
}

void VCClockProperties::slotRemoveSchedule()
{
    QListIterator <QTreeWidgetItem *> it(m_scheduleTree->selectedItems());

    while (it.hasNext() == true)
    {
        int index = m_scheduleTree->indexOfTopLevelItem(it.next());
        m_scheduleTree->takeTopLevelItem(index);
    }
}
