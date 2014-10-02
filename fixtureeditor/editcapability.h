/*
  Q Light Controller - Fixture Definition Editor
  editcapability.h

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

#ifndef EDITCAPABILITY_H
#define EDITCAPABILITY_H

#include <QWidget>
#include "ui_editcapability.h"
#include "qlcchannel.h"

class QWidget;
class QLCCapability;

/** @addtogroup fixtureeditor Fixture Editor
 * @{
 */

class EditCapability : public QDialog, public Ui_EditCapability
{
    Q_OBJECT

public:
    EditCapability(QWidget* parent, const QLCCapability* capability = NULL,
                   QLCChannel::Group group = QLCChannel::NoGroup, uchar min = 0);
    ~EditCapability();

    /*********************************************************************
     * Capability
     *********************************************************************/
public:
    QLCCapability* capability() const {
        return m_capability;
    }

protected:
    QLCCapability* m_capability;

    /*********************************************************************
     * UI slots
     *********************************************************************/
public slots:
    void slotMinSpinChanged(int value);
    void slotMaxSpinChanged(int value);
    void slotDescriptionEdited(const QString& text);
    void slotPictureButtonPressed();
    void slotColor1ButtonPressed();
    void slotColor2ButtonPressed();

};

/** @} */

#endif
