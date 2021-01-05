/*
  Q Light Controller
  groupsconsole.h

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

#ifndef GROUPSCONSOLE_H
#define GROUPSCONSOLE_H

#include <QWidget>

#include "consolechannel.h"
#include "scene.h"

class QSlider;
class QSpinBox;

/** @addtogroup ui UI
 * @{
 */

class GroupsConsole : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(GroupsConsole)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    GroupsConsole(QWidget* parent, Doc* doc, QList <quint32> ids, QList<uchar> levels);
    ~GroupsConsole();

    QList<ConsoleChannel*>groups();

private:
    Doc* m_doc;
    /** List of active ChannelsGroup IDs */
    QList <quint32> m_ids;
    QList <uchar> m_levels;
    QList<ConsoleChannel*> m_groups;

    /** init the widget view with groups sliders */
    void init();

signals:
    /** Emitted when the value of a channels group object changes (continuous) */
    void groupValueChanged(quint32 group, uchar value);
};

/** @} */

#endif
