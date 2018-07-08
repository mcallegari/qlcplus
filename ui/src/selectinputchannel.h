/*
  Q Light Controller
  selectinputchannel.h

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

#ifndef SELECTINPUTCHANNEL_H
#define SELECTINPUTCHANNEL_H

#include <QDialog>

#include "ui_selectinputchannel.h"

class QTreeWidgetItem;
class QLCInputProfile;
class QLCInputChannel;
class InputOutputMap;
class InputPatch;

/** @addtogroup ui UI
 * @{
 */

class SelectInputChannel : public QDialog, public Ui_SelectInputChannel
{
    Q_OBJECT
    Q_DISABLE_COPY(SelectInputChannel)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    SelectInputChannel(QWidget* parent, InputOutputMap* ioMap);
    ~SelectInputChannel();

protected:
    void saveSettings();
    void loadSettings();

protected slots:
    void accept();
    void slotUnpatchedClicked();

private:
    InputOutputMap* m_ioMap;

    /********************************************************************
     * Selection
     ********************************************************************/
public:
    /** Get the selected universe */
    quint32 universe() const;

    /** Get the selected channel within the selected universe */
    quint32 channel() const;

protected:
    quint32 m_universe;
    quint32 m_channel;

    /********************************************************************
     * Tree widget
     ********************************************************************/
protected:
    /** Fill the tree with available input universes & channels */
    void fillTree();

    /** Update the contents of a universe item */
    void updateUniverseItem(QTreeWidgetItem* item,
                            quint32 uni,
                            InputPatch* patch);

    /** Update the contents of a channel item */
    void updateChannelItem(QTreeWidgetItem* item,
                           quint32 universe,
                           const QLCInputChannel* channel,
                           const QLCInputProfile* profile);

protected slots:
    /** Receives changed information for items with manual input enabled */
    void slotItemChanged(QTreeWidgetItem* item, int column);

    /** Receives item double clicks */
    void slotItemDoubleClicked(QTreeWidgetItem* item, int column);
};

/** @} */

#endif
