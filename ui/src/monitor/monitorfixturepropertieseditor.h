/*
  Q Light Controller Plus
  monitorfixturepropertieseditor.h

  Copyright (C) Massimo Callegari

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

#ifndef MONITORFIXTUREPROPERTIESEDITOR_H
#define MONITORFIXTUREPROPERTIESEDITOR_H

#include <QWidget>

#include "ui_monitorfixturepropertieseditor.h"

class MonitorGraphicsView;
class MonitorFixtureItem;
class MonitorProperties;

/** \addtogroup ui_mon DMX Monitor
 * @{
 */

class MonitorFixturePropertiesEditor : public QWidget, public Ui_MonitorFixturePropertiesEditor
{
    Q_OBJECT

public:
    MonitorFixturePropertiesEditor(MonitorFixtureItem *fxItem, MonitorGraphicsView *gfxView,
                                   MonitorProperties *props, QWidget *parent = 0);
    ~MonitorFixturePropertiesEditor();

protected slots:
    /** Slot called when the user changes
     *  the position of a fixture item */
    void slotSetPosition();

    /** Slot called when the user changes
     *  a fixture item rotation */
    void slotRotationChanged(int value);

    /** Slot called when the user wants to set a
     *  gel color to apply to a fixture */
    void slotGelColorClicked();

    /** Slot called when the user resets
     *  the current gel color */
    void slotGelResetClicked();

private:
    MonitorFixtureItem *m_fxItem;
    MonitorGraphicsView *m_gfxView;
    MonitorProperties *m_props;
};

/** @} */

#endif // MONITORFIXTUREPROPERTIESEDITOR_H
