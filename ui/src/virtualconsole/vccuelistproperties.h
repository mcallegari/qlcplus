/*
  Q Light Controller
  vccuelistproperties.h

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

#ifndef VCCUELISTPROPERTIES_H
#define VCCUELISTPROPERTIES_H

#include <QKeySequence>
#include <QDialog>
#include <QList>

#include "ui_vccuelistproperties.h"

class InputSelectionWidget;
class VCCueList;
class Doc;

/** @addtogroup ui_vc_props
 * @{
 */

class VCCueListProperties : public QDialog, public Ui_VCCueListProperties
{
    Q_OBJECT
    Q_DISABLE_COPY(VCCueListProperties)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    VCCueListProperties(VCCueList* cueList, Doc* doc);
    ~VCCueListProperties();

public slots:
    void accept();
    void slotTabChanged();

protected:
    VCCueList* m_cueList;
    Doc* m_doc;
    InputSelectionWidget *m_playInputWidget;
    InputSelectionWidget *m_stopInputWidget;
    InputSelectionWidget *m_nextInputWidget;
    InputSelectionWidget *m_prevInputWidget;

    InputSelectionWidget *m_crossfadeInputWidget;

    /************************************************************************
     * Cues
     ************************************************************************/
protected slots:
    void slotChaserAttachClicked();
    void slotChaserDetachClicked();
    void slotPlaybackLayoutChanged();

private:
    void updateChaserName();
    quint32 m_chaserId;
};

/** @} */

#endif
