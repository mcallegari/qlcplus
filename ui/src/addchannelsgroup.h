/*
  Q Light Controller
  channelselection.h

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

#ifndef CHANNELSELECTION_H
#define CHANNELSELECTION_H

#include <QDialog>

#include "ui_addchannelsgroup.h"

class InputSelectionWidget;
class ChannelsGroup;
class Doc;

/** @addtogroup ui_fixtures
 * @{
 */

class AddChannelsGroup : public QDialog, public Ui_AddChannelsGroup
{
    Q_OBJECT
    Q_DISABLE_COPY(AddChannelsGroup)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Constructor
     *
     * @param parent The parent widget for the dialog
     * @param doc The QLC engine instance
     */
    AddChannelsGroup(QWidget* parent, Doc* doc, ChannelsGroup *group);
    ~AddChannelsGroup();

    /** @reimp */
    void accept();

private:
    Doc *m_doc;
    ChannelsGroup *m_chansGroup;
    InputSelectionWidget *m_inputSelWidget;

protected:
    int m_checkedChannels;
    bool m_isUpdating;

protected slots:
    void slotItemChecked(QTreeWidgetItem *item, int col);
};

/** @} */

#endif
