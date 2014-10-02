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
#include "qlcinputsource.h"

class MasterTimer;
class VCCueList;
class OutputMap;
class InputMap;
class Function;
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

    /************************************************************************
     * Cues
     ************************************************************************/
protected slots:
    void slotChaserAttachClicked();
    void slotChaserDetachClicked();

private:
    void updateChaserName();
    quint32 m_chaserId;

    /************************************************************************
     * Next Cue
     ************************************************************************/
protected slots:
    void slotNextAttachClicked();
    void slotNextDetachClicked();
    void slotNextChooseInputClicked();
    void slotNextAutoDetectInputToggled(bool checked);
    void slotNextInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updateNextInputSource();

protected:
    QKeySequence m_nextKeySequence;
    QLCInputSource *m_nextInputSource;

    /************************************************************************
     * Previous Cue
     ************************************************************************/
protected slots:
    void slotPreviousAttachClicked();
    void slotPreviousDetachClicked();
    void slotPreviousChooseInputClicked();
    void slotPreviousAutoDetectInputToggled(bool checked);
    void slotPreviousInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updatePreviousInputSource();

protected:
    QKeySequence m_previousKeySequence;
    QLCInputSource *m_previousInputSource;

    /************************************************************************
     * Cue List Playback
     ************************************************************************/
protected slots:
    void slotPlaybackAttachClicked();
    void slotPlaybackDetachClicked();
    void slotPlaybackChooseInputClicked();
    void slotPlaybackAutoDetectInputToggled(bool checked);
    void slotPlaybackInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updatePlaybackInputSource();

protected:
    QKeySequence m_playbackKeySequence;
    QLCInputSource *m_playbackInputSource;

    /************************************************************************
     * Crossfade Cue List
     ************************************************************************/
protected slots:
    void slotCF1ChooseInputClicked();
    void slotCF1AutoDetectInputToggled(bool checked);
    void slotCF1InputValueChanged(quint32 uni, quint32 ch);
    void slotCF2ChooseInputClicked();
    void slotCF2AutoDetectInputToggled(bool checked);
    void slotCF2InputValueChanged(quint32 uni, quint32 ch);

protected:
    void updateCrossfadeInputSource();

protected:
    QLCInputSource *m_cf1InputSource;
    QLCInputSource *m_cf2InputSource;
};

/** @} */

#endif
