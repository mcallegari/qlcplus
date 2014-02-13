/*
  Q Light Controller Plus
  channelsselection.h

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

#ifndef CHANNELSCONFIGURATION_H
#define CHANNELSCONFIGURATION_H

#include <QDialog>

#include "ui_channelsselection.h"
#include "scenevalue.h"

class Doc;

/** @addtogroup ui UI
 * @{
 */

class ChannelsSelection : public QDialog, public Ui_ChannelsSelection
{
    Q_OBJECT
    Q_DISABLE_COPY(ChannelsSelection)
    
public:
    enum ChannelSelectionType
    {
        NormalMode,
        ExcludeChannelsMode
    };

    ChannelsSelection(Doc* doc, QWidget *parent = 0, ChannelSelectionType mode = NormalMode);
    ~ChannelsSelection();

    void setChannelsList(QList<SceneValue> list);
    QList<SceneValue> channelsList();

private:
    Doc* m_doc;
    ChannelSelectionType m_mode;
    QList<SceneValue> m_channelsList;

protected:
    bool m_isUpdating;
    void updateFixturesTree();

protected slots:

    void slotItemChecked(QTreeWidgetItem *item, int col);
    void slotItemExpanded();

    /** Callback for OK button clicks */
    void accept();
};

/** @} */

#endif // CHANNELSCONFIGURATION_H
