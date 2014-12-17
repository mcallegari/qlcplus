/*
  Q Light Controller Plus
  audiotriggersconfiguration.h

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

#ifndef AUDIOTRIGGERSCONFIGURATION_H
#define AUDIOTRIGGERSCONFIGURATION_H

#include <QDialog>

#include "ui_vcaudiotriggersproperties.h"
#include "doc.h"

class VCAudioTriggers;

/** @addtogroup ui_vc_props
 * @{
 */

class AudioTriggersConfiguration : public QDialog, public Ui_AudioTriggersConfiguration
{
    Q_OBJECT
    
public:
    explicit AudioTriggersConfiguration(VCAudioTriggers *triggers, Doc *doc,
                                        int bandsNumber, int maxFrequency);
    ~AudioTriggersConfiguration();

    /** @reimp */
    void accept();
    
private slots:
    void updateTreeItem(QTreeWidgetItem *item, int idx);
    void updateTree();
    void slotTypeComboChanged(int comboIndex);
    void slotDmxSelectionClicked();
    void slotFunctionSelectionClicked();
    void slotWidgetSelectionClicked();
    void slotMinThresholdChanged(int val);
    void slotMaxThresholdChanged(int val);
    void slotDivisorChanged(int val);

    /*************************************************************************
     * External Input
     *************************************************************************/
protected slots:
    void slotAttachKey();
    void slotDetachKey();

    void slotAutoDetectInputToggled(bool checked);
    void slotInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseInputClicked();

protected:
    void updateInputSource();

private:
    Doc *m_doc;
    VCAudioTriggers *m_triggers;
    int m_maxFrequency;
    QKeySequence m_keySequence;
    QLCInputSource *m_inputSource;
};

/** @} */

#endif // AUDIOTRIGGERSCONFIGURATION_H
