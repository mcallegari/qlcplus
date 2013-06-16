/*
  Q Light Controller
  vccuelistproperties.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    QLCInputSource m_nextInputSource;

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
    QLCInputSource m_previousInputSource;

    /************************************************************************
     * Stop Cue List
     ************************************************************************/
protected slots:
    void slotStopAttachClicked();
    void slotStopDetachClicked();
    void slotStopChooseInputClicked();
    void slotStopAutoDetectInputToggled(bool checked);
    void slotStopInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updateStopInputSource();

protected:
    QKeySequence m_stopKeySequence;
    QLCInputSource m_stopInputSource;

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
    QLCInputSource m_cf1InputSource;
    QLCInputSource m_cf2InputSource;

};

#endif
