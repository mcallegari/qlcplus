/*
  Q Light Controller - Fixture Definition Editor
  editchannel.h

  Copyright (C) Heikki Junnila

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

#ifndef EDITCHANNEL_H
#define EDITCHANNEL_H

#include <QWidget>
#include "ui_editchannel.h"
#include "qlcchannel.h"

class QTreeWidgetItem;
class QString;

class QLCCapability;

/** @addtogroup fixtureeditor Fixture Editor
 * @{
 */

class EditChannel : public QDialog, public Ui_EditChannel
{
    Q_OBJECT
public:
    EditChannel(QWidget *parent, QLCChannel *channel = NULL);
    ~EditChannel();

protected:
    void init();
    void setupCapabilityGroup();

    /*********************************************************************
     * Channel
     *********************************************************************/
public:
    /** Get the channel that was edited. */
    QLCChannel *channel();

protected:
    QLCChannel *m_channel;

    /*********************************************************************
     * Basic channel info
     *********************************************************************/
protected slots:
    void slotNameChanged(const QString& name);
    void slotPresetActivated(int index);
    void slotGroupActivated(int index);
    void slotDefaultValueChanged(int val);
    void slotMsbRadioToggled(bool toggled);
    void slotLsbRadioToggled(bool toggled);

    /*********************************************************************
     * Capabilities
     *********************************************************************/
protected slots:
    void slotCapabilityCellChanged(int row, int column);
    void slotCapabilityCellSelected(int currentRow, int currentColumn,
                                    int previousRow, int previousColumn);
    void slotRemoveCapabilityClicked();
    void slotEditCapabilityClicked();
    void slotWizardClicked();

    void slotCapabilityPresetActivated(int index);
    void slotPictureButtonPressed();
    void slotColor1ButtonPressed();
    void slotColor2ButtonPressed();
    void slotValue1SpinChanged(double value);
    void slotValue2SpinChanged(double value);

protected:
    void refreshCapabilities();
    QLCCapability *currentCapability();
    QLCCapability *getRowCapability(int row);
    void updateCapabilityPresetGroup(bool show);
    void checkOverlapping();

protected:
    QLCCapability *m_currentCapability;
};

/** @} */

#endif
