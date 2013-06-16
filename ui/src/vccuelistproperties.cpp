/*
  Q Light Controller
  vccuelistproperties.cpp

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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QHeaderView>
#include <QDebug>

#include "vccuelistproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "assignhotkey.h"
#include "vccuelist.h"
#include "qlcmacros.h"
#include "inputmap.h"
#include "doc.h"

#define KColumnNumber 0
#define KColumnName   1
#define KColumnID     2

VCCueListProperties::VCCueListProperties(VCCueList* cueList, Doc* doc)
    : QDialog(cueList)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(cueList != NULL);
    m_cueList = cueList;

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    /************************************************************************
     * Cues page
     ************************************************************************/

    /* Name */
    m_nameEdit->setText(cueList->caption());
    m_nameEdit->setSelection(0, cueList->caption().length());

    /* Chaser */
    m_chaserId = cueList->chaser();
    updateChaserName();

    /* Connections */
    connect(m_chaserAttachButton, SIGNAL(clicked()), this, SLOT(slotChaserAttachClicked()));
    connect(m_chaserDetachButton, SIGNAL(clicked()), this, SLOT(slotChaserDetachClicked()));

    /************************************************************************
     * Next Cue page
     ************************************************************************/

    /* Connections */
    connect(m_nextAttachButton, SIGNAL(clicked()),
            this, SLOT(slotNextAttachClicked()));
    connect(m_nextDetachButton, SIGNAL(clicked()),
            this, SLOT(slotNextDetachClicked()));
    connect(m_nextAutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotNextAutoDetectInputToggled(bool)));
    connect(m_nextChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotNextChooseInputClicked()));

    /* Key binding */
    m_nextKeySequence = QKeySequence(cueList->nextKeySequence());
    m_nextKeyEdit->setText(m_nextKeySequence.toString(QKeySequence::NativeText));

    /* External input */
    m_nextInputSource = cueList->inputSource(VCCueList::nextInputSourceId);
    updateNextInputSource();

    /************************************************************************
     * Previous Cue page
     ************************************************************************/

    /* Connections */
    connect(m_previousAttachButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousAttachClicked()));
    connect(m_previousDetachButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousDetachClicked()));
    connect(m_previousAutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotPreviousAutoDetectInputToggled(bool)));
    connect(m_previousChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousChooseInputClicked()));

    /* Key binding */
    m_previousKeySequence = QKeySequence(cueList->previousKeySequence());
    m_previousKeyEdit->setText(m_previousKeySequence.toString(QKeySequence::NativeText));

    /* External input */
    m_previousInputSource = cueList->inputSource(VCCueList::previousInputSourceId);
    updatePreviousInputSource();

    /************************************************************************
     * Stop Cue List page
     ************************************************************************/

    /* Connections */
    connect(m_stopAttachButton, SIGNAL(clicked()),
            this, SLOT(slotStopAttachClicked()));
    connect(m_stopDetachButton, SIGNAL(clicked()),
            this, SLOT(slotStopDetachClicked()));
    connect(m_stopAutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotStopAutoDetectInputToggled(bool)));
    connect(m_stopChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotStopChooseInputClicked()));

    /* Key binding */
    m_stopKeySequence = QKeySequence(cueList->stopKeySequence());
    m_stopKeyEdit->setText(m_stopKeySequence.toString(QKeySequence::NativeText));

    /* External input */
    m_stopInputSource = cueList->inputSource(VCCueList::stopInputSourceId);
    updateStopInputSource();

    /************************************************************************
     * Crossfade Cue List page
     ************************************************************************/

    /* Connections */
    connect(m_cf1AutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotCF1AutoDetectInputToggled(bool)));
    connect(m_cf1ChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotCF1ChooseInputClicked()));
    connect(m_cf2AutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotCF2AutoDetectInputToggled(bool)));
    connect(m_cf2ChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotCF2ChooseInputClicked()));

    /* External input */
    m_cf1InputSource = cueList->inputSource(VCCueList::cf1InputSourceId);
    m_cf2InputSource = cueList->inputSource(VCCueList::cf2InputSourceId);
    updateCrossfadeInputSource();
}

VCCueListProperties::~VCCueListProperties()
{
}

void VCCueListProperties::accept()
{
    /* Name */
    m_cueList->setCaption(m_nameEdit->text());

    /* Chaser */
    m_cueList->setChaser(m_chaserId);

    /* Key sequences */
    m_cueList->setNextKeySequence(m_nextKeySequence);
    m_cueList->setPreviousKeySequence(m_previousKeySequence);
    m_cueList->setStopKeySequence(m_stopKeySequence);

    /* Input sources */
    m_cueList->setInputSource(m_nextInputSource, VCCueList::nextInputSourceId);
    m_cueList->setInputSource(m_previousInputSource, VCCueList::previousInputSourceId);
    m_cueList->setInputSource(m_stopInputSource, VCCueList::stopInputSourceId);
    m_cueList->setInputSource(m_cf1InputSource, VCCueList::cf1InputSourceId);
    m_cueList->setInputSource(m_cf2InputSource, VCCueList::cf2InputSourceId);

    QDialog::accept();
}

void VCCueListProperties::slotTabChanged()
{
    // Disengage auto-detect buttons
    if (m_nextAutoDetectInputButton->isChecked() == true)
        m_nextAutoDetectInputButton->toggle();
    if (m_previousAutoDetectInputButton->isChecked() == true)
        m_previousAutoDetectInputButton->toggle();
    if (m_stopAutoDetectInputButton->isChecked() == true)
        m_stopAutoDetectInputButton->toggle();
    if (m_cf1AutoDetectInputButton->isChecked() == true)
        m_cf1AutoDetectInputButton->toggle();
    if (m_cf2AutoDetectInputButton->isChecked() == true)
        m_cf2AutoDetectInputButton->toggle();
}

/****************************************************************************
 * Cues
 ****************************************************************************/

void VCCueListProperties::slotChaserAttachClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(false);
    fs.setFilter(Function::Chaser, true);
    if (fs.exec() == QDialog::Accepted && fs.selection().size() > 0)
    {
        m_chaserId = fs.selection().first();
        updateChaserName();
    }
}

void VCCueListProperties::slotChaserDetachClicked()
{
    m_chaserId = Function::invalidId();
    updateChaserName();
}

void VCCueListProperties::updateChaserName()
{
    Function* function = m_doc->function(m_chaserId);
    if (function == NULL)
        m_chaserEdit->setText(tr("No function"));
    else
        m_chaserEdit->setText(function->name());
}

/****************************************************************************
 * Next Cue
 ****************************************************************************/

void VCCueListProperties::slotNextAttachClicked()
{
    AssignHotKey ahk(this, m_nextKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_nextKeySequence = QKeySequence(ahk.keySequence());
        m_nextKeyEdit->setText(m_nextKeySequence.toString(QKeySequence::NativeText));
    }
}

void VCCueListProperties::slotNextDetachClicked()
{
    m_nextKeySequence = QKeySequence();
    m_nextKeyEdit->setText(m_nextKeySequence.toString(QKeySequence::NativeText));
}

void VCCueListProperties::slotNextChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_nextInputSource = QLCInputSource(sic.universe(), sic.channel());
        updateNextInputSource();
    }
}

void VCCueListProperties::slotNextAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotNextInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotNextInputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotNextInputValueChanged(quint32 uni, quint32 ch)
{
    m_nextInputSource = QLCInputSource(uni, ch);
    updateNextInputSource();
}

void VCCueListProperties::updateNextInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputMap()->inputSourceNames(m_nextInputSource, uniName, chName) == true)
    {
        /* Display the gathered information */
        m_nextInputUniverseEdit->setText(uniName);
        m_nextInputChannelEdit->setText(chName);
    }
    else
    {
        m_nextInputUniverseEdit->setText(KInputNone);
        m_nextInputChannelEdit->setText(KInputNone);
    }
}

/****************************************************************************
 * Previous Cue
 ****************************************************************************/

void VCCueListProperties::slotPreviousAttachClicked()
{
    AssignHotKey ahk(this, m_previousKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_previousKeySequence = QKeySequence(ahk.keySequence());
        m_previousKeyEdit->setText(m_previousKeySequence.toString(QKeySequence::NativeText));
    }
}

void VCCueListProperties::slotPreviousDetachClicked()
{
    m_previousKeySequence = QKeySequence();
    m_previousKeyEdit->setText(m_previousKeySequence.toString(QKeySequence::NativeText));
}

void VCCueListProperties::slotPreviousChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_previousInputSource = QLCInputSource(sic.universe(), sic.channel());
        updatePreviousInputSource();
    }
}

void VCCueListProperties::slotPreviousAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotPreviousInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotPreviousInputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotPreviousInputValueChanged(quint32 uni, quint32 ch)
{
    m_previousInputSource = QLCInputSource(uni, ch);
    updatePreviousInputSource();
}

void VCCueListProperties::updatePreviousInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputMap()->inputSourceNames(m_previousInputSource, uniName, chName) == true)
    {
        /* Display the gathered information */
        m_previousInputUniverseEdit->setText(uniName);
        m_previousInputChannelEdit->setText(chName);
    }
    else
    {
        m_previousInputUniverseEdit->setText(KInputNone);
        m_previousInputChannelEdit->setText(KInputNone);
    }
}

/****************************************************************************
 * Stop Cue List
 ****************************************************************************/

void VCCueListProperties::slotStopAttachClicked()
{
    AssignHotKey ahk(this, m_stopKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_stopKeySequence = QKeySequence(ahk.keySequence());
        m_stopKeyEdit->setText(m_stopKeySequence.toString(QKeySequence::NativeText));
    }
}

void VCCueListProperties::slotStopDetachClicked()
{
    m_stopKeySequence = QKeySequence();
    m_stopKeyEdit->setText(m_stopKeySequence.toString(QKeySequence::NativeText));
}

void VCCueListProperties::slotStopChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_stopInputSource = QLCInputSource(sic.universe(), sic.channel());
        updateStopInputSource();
    }
}

void VCCueListProperties::slotStopAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotStopInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotStopInputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotStopInputValueChanged(quint32 uni, quint32 ch)
{
    m_stopInputSource = QLCInputSource(uni, ch);
    updateStopInputSource();
}

void VCCueListProperties::updateStopInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputMap()->inputSourceNames(m_stopInputSource, uniName, chName) == true)
    {
        /* Display the gathered information */
        m_stopInputUniverseEdit->setText(uniName);
        m_stopInputChannelEdit->setText(chName);
    }
    else
    {
        m_stopInputUniverseEdit->setText(KInputNone);
        m_stopInputChannelEdit->setText(KInputNone);
    }
}

/************************************************************************
 * Crossfade Cue List
 ************************************************************************/

void VCCueListProperties::slotCF1ChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_cf1InputSource = QLCInputSource(sic.universe(), sic.channel());
        updateCrossfadeInputSource();
    }
}

void VCCueListProperties::slotCF1AutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotCF1InputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotCF1InputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotCF1InputValueChanged(quint32 uni, quint32 ch)
{
    m_cf1InputSource = QLCInputSource(uni, ch);
    updateCrossfadeInputSource();
}

void VCCueListProperties::slotCF2ChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_cf2InputSource = QLCInputSource(sic.universe(), sic.channel());
        updateCrossfadeInputSource();
    }
}

void VCCueListProperties::slotCF2AutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotCF2InputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotCF2InputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotCF2InputValueChanged(quint32 uni, quint32 ch)
{
    m_cf2InputSource = QLCInputSource(uni, ch);
    updateCrossfadeInputSource();
}

void VCCueListProperties::updateCrossfadeInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputMap()->inputSourceNames(m_cf1InputSource, uniName, chName) == true)
    {
        /* Display the gathered information */
        m_cf1InputUniverseEdit->setText(uniName);
        m_cf1InputChannelEdit->setText(chName);
    }
    else
    {
        m_cf1InputUniverseEdit->setText(KInputNone);
        m_cf1InputChannelEdit->setText(KInputNone);
    }

    if (m_doc->inputMap()->inputSourceNames(m_cf2InputSource, uniName, chName) == true)
    {
        /* Display the gathered information */
        m_cf2InputUniverseEdit->setText(uniName);
        m_cf2InputChannelEdit->setText(chName);
    }
    else
    {
        m_cf2InputUniverseEdit->setText(KInputNone);
        m_cf2InputChannelEdit->setText(KInputNone);
    }
}
