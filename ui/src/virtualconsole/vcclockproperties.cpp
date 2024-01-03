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
#include "inputselectionwidget.h"

#define KColumnName     0
#define KColumnTime     1

VCClockProperties::VCClockProperties(VCClock *clock, Doc *doc)
    : QDialog(clock)
    , m_clock(clock)
    , m_doc(doc)
{
    Q_ASSERT(clock != NULL);

    setupUi(this);

    m_playInputWidget = new InputSelectionWidget(m_doc, this);
    m_playInputWidget->setTitle(tr("Play/Pause control"));
    m_playInputWidget->setCustomFeedbackVisibility(true);
    m_playInputWidget->setKeySequence(m_clock->playKeySequence());
    m_playInputWidget->setInputSource(m_clock->inputSource(VCClock::playInputSourceId));
    m_playInputWidget->setWidgetPage(m_clock->page());
    m_playInputWidget->show();
    m_externalInputLayout->addWidget(m_playInputWidget);

    m_resetInputWidget = new InputSelectionWidget(m_doc, this);
    m_resetInputWidget->setTitle(tr("Reset control"));
    m_resetInputWidget->setCustomFeedbackVisibility(true);
    m_resetInputWidget->setKeySequence(m_clock->resetKeySequence());
    m_resetInputWidget->setInputSource(m_clock->inputSource(VCClock::resetInputSourceId));
    m_resetInputWidget->setWidgetPage(m_clock->page());
    m_resetInputWidget->show();
    m_externalInputLayout->addWidget(m_resetInputWidget);

    m_noControlLabel->hide();

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
        {
            m_clockRadio->setChecked(true);
            m_playInputWidget->hide();
            m_resetInputWidget->hide();
            m_noControlLabel->show();
        }
        break;
        default:
            m_clockRadio->setChecked(true);
        break;
    }

    foreach (VCClockSchedule sch, m_clock->schedules())
        addScheduleItem(sch);

    connect(m_clockRadio, SIGNAL(clicked()),
            this, SLOT(slotTypeSelectChanged()));
    connect(m_countdownRadio, SIGNAL(clicked()),
            this, SLOT(slotTypeSelectChanged()));
    connect(m_stopWatchRadio, SIGNAL(clicked()),
            this, SLOT(slotTypeSelectChanged()));
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
        item->setIcon(KColumnName, func->getIcon());
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

    /* Key sequences */
    m_clock->setPlayKeySequence(m_playInputWidget->keySequence());
    m_clock->setResetKeySequence(m_resetInputWidget->keySequence());

    /* Input sources */
    m_clock->setInputSource(m_playInputWidget->inputSource(), VCClock::playInputSourceId);
    m_clock->setInputSource(m_resetInputWidget->inputSource(), VCClock::resetInputSourceId);

    QDialog::accept();
}

void VCClockProperties::slotTypeSelectChanged()
{
    if (m_clockRadio->isChecked())
    {
        m_resetInputWidget->hide();
        m_playInputWidget->hide();
        m_noControlLabel->show();
    }
    else
    {
        m_resetInputWidget->show();
        m_playInputWidget->show();
        m_noControlLabel->hide();
    }
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
