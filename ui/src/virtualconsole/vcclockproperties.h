/*
  Q Light Controller Plus
  vcclockproperties.h

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

#ifndef VCCLOCKPROPERTIES_H
#define VCCLOCKPROPERTIES_H

#include <QDialog>

#include "ui_vcclockproperties.h"
#include "vcclock.h"

class InputSelectionWidget;

/** @addtogroup ui_vc_props
 * @{
 */

class VCClockProperties : public QDialog, public Ui_VCClockProperties
{
    Q_OBJECT

public:
    VCClockProperties(VCClock *clock, Doc *doc);
    ~VCClockProperties();

private:
    void addScheduleItem(VCClockSchedule schedule);

public slots:
    void accept();

protected slots:
    void slotTypeSelectChanged();
    void slotAddSchedule();
    void slotRemoveSchedule();

private:
    VCClock *m_clock;
    Doc* m_doc;

protected:
    InputSelectionWidget *m_playInputWidget;
    InputSelectionWidget *m_resetInputWidget;
};

/** @} */

#endif // VCCLOCKPROPERTIES_H
