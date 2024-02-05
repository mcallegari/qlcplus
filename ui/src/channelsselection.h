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

/**
 * The ChannelsSelection class is used mainly for 2 purposes: channels selection and
 * channels configuration.
 * In both cases, the channels list is displayed as a tree view, where the items levels
 * are nested as universe -> fixture -> channel.
 * Depending on the selected mode during the class construction, different columns
 * are presented to the user:
 * Channels selection: Name, Type, Selected
 * Channels configuration: Name, Type, Can fade, Behaviour, Modifier
 *
 * In channels selection mode, to retrieve the selected channels list, just use the
 * "channelsList" method before destroying the class.
 * In channels configuration mode, changes will be applied on the "accept" event.
 *
 * In both modes, users are presented with a "Apply changes to fixtures of the same type"
 * check box, which will apply a change to a channel to all the channels of the same
 * type and fixture in all the universes.
 */

class ChannelsSelection : public QDialog, public Ui_ChannelsSelection
{
    Q_OBJECT
    Q_DISABLE_COPY(ChannelsSelection)

public:
    enum ChannelSelectionType
    {
        NormalMode,
        ConfigurationMode
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
    void updateFixturesTree();
    QList<QTreeWidgetItem *> getSameChannels(QTreeWidgetItem *item);

protected slots:

    void slotItemChecked(QTreeWidgetItem *item, int col);
    void slotItemExpanded();

    /** Slot called when user picks a new behaviour from a channel
     *  combo box */
    void slotComboChanged(int idx);

    /** Slot called when user clicks on a channel modifier button */
    void slotModifierButtonClicked();

    /** Callback for OK button clicks */
    void accept();
};

/** @} */

#endif // CHANNELSCONFIGURATION_H
