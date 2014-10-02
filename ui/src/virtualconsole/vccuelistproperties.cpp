/*
  Q Light Controller
  vccuelistproperties.cpp

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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QHeaderView>
#include <QDebug>

#include "vccuelistproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "assignhotkey.h"
#include "inputpatch.h"
#include "vccuelist.h"
#include "qlcmacros.h"
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
    m_chaserId = cueList->chaserID();
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
     * Playback Cue List page
     ************************************************************************/

    /* Connections */
    connect(m_playbackAttachButton, SIGNAL(clicked()),
            this, SLOT(slotPlaybackAttachClicked()));
    connect(m_playbackDetachButton, SIGNAL(clicked()),
            this, SLOT(slotPlaybackDetachClicked()));
    connect(m_playbackAutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotPlaybackAutoDetectInputToggled(bool)));
    connect(m_playbackChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotPlaybackChooseInputClicked()));

    /* Key binding */
    m_playbackKeySequence = QKeySequence(cueList->playbackKeySequence());
    m_playbackKeyEdit->setText(m_playbackKeySequence.toString(QKeySequence::NativeText));

    /* External input */
    m_playbackInputSource = cueList->inputSource(VCCueList::playbackInputSourceId);
    updatePlaybackInputSource();

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
    m_cueList->setPlaybackKeySequence(m_playbackKeySequence);

    /* Input sources */
    m_cueList->setInputSource(m_nextInputSource, VCCueList::nextInputSourceId);
    m_cueList->setInputSource(m_previousInputSource, VCCueList::previousInputSourceId);
    m_cueList->setInputSource(m_playbackInputSource, VCCueList::playbackInputSourceId);
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
    if (m_playbackAutoDetectInputButton->isChecked() == true)
        m_playbackAutoDetectInputButton->toggle();
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
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_nextInputSource != NULL)
           delete m_nextInputSource;
        m_nextInputSource = new QLCInputSource(sic.universe(), sic.channel());
        updateNextInputSource();
    }
}

void VCCueListProperties::slotNextAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotNextInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotNextInputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotNextInputValueChanged(quint32 uni, quint32 ch)
{
    if (m_nextInputSource != NULL)
       delete m_nextInputSource;
    m_nextInputSource = new QLCInputSource(uni, (m_cueList->page() << 16) | ch);
    updateNextInputSource();
}

void VCCueListProperties::updateNextInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_nextInputSource, uniName, chName) == true)
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
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_previousInputSource != NULL)
           delete m_previousInputSource;
        m_previousInputSource = new QLCInputSource(sic.universe(), sic.channel());
        updatePreviousInputSource();
    }
}

void VCCueListProperties::slotPreviousAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotPreviousInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotPreviousInputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotPreviousInputValueChanged(quint32 uni, quint32 ch)
{
    if (m_previousInputSource != NULL)
       delete m_previousInputSource;
    m_previousInputSource = new QLCInputSource(uni, (m_cueList->page() << 16) | ch);
    updatePreviousInputSource();
}

void VCCueListProperties::updatePreviousInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_previousInputSource, uniName, chName) == true)
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
 * Cue List Playback
 ****************************************************************************/

void VCCueListProperties::slotPlaybackAttachClicked()
{
    AssignHotKey ahk(this, m_playbackKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_playbackKeySequence = QKeySequence(ahk.keySequence());
        m_playbackKeyEdit->setText(m_playbackKeySequence.toString(QKeySequence::NativeText));
    }
}

void VCCueListProperties::slotPlaybackDetachClicked()
{
    m_playbackKeySequence = QKeySequence();
    m_playbackKeyEdit->setText(m_playbackKeySequence.toString(QKeySequence::NativeText));
}

void VCCueListProperties::slotPlaybackChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_playbackInputSource != NULL)
           delete m_playbackInputSource;
        m_playbackInputSource = new QLCInputSource(sic.universe(), sic.channel());
        updatePlaybackInputSource();
    }
}

void VCCueListProperties::slotPlaybackAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotPlaybackInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotPlaybackInputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotPlaybackInputValueChanged(quint32 uni, quint32 ch)
{
    if (m_playbackInputSource != NULL)
       delete m_playbackInputSource;
    m_playbackInputSource = new QLCInputSource(uni, (m_cueList->page() << 16) | ch);
    updatePlaybackInputSource();
}

void VCCueListProperties::updatePlaybackInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_playbackInputSource, uniName, chName) == true)
    {
        /* Display the gathered information */
        m_playbackInputUniverseEdit->setText(uniName);
        m_playbackInputChannelEdit->setText(chName);
    }
    else
    {
        m_playbackInputUniverseEdit->setText(KInputNone);
        m_playbackInputChannelEdit->setText(KInputNone);
    }
}

/************************************************************************
 * Crossfade Cue List
 ************************************************************************/

void VCCueListProperties::slotCF1ChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_cf1InputSource != NULL)
           delete m_cf1InputSource;
        m_cf1InputSource = new QLCInputSource(sic.universe(), sic.channel());
        updateCrossfadeInputSource();
    }
}

void VCCueListProperties::slotCF1AutoDetectInputToggled(bool checked)
{
    m_cf2AutoDetectInputButton->setChecked(false);

    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotCF1InputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotCF1InputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotCF1InputValueChanged(quint32 uni, quint32 ch)
{
    if (m_cf1InputSource != NULL)
       delete m_cf1InputSource;
    m_cf1InputSource = new QLCInputSource(uni, (m_cueList->page() << 16) | ch);
    updateCrossfadeInputSource();
}

void VCCueListProperties::slotCF2ChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_cf2InputSource != NULL)
           delete m_cf2InputSource;
        m_cf2InputSource = new QLCInputSource(sic.universe(), sic.channel());
        updateCrossfadeInputSource();
    }
}

void VCCueListProperties::slotCF2AutoDetectInputToggled(bool checked)
{
    m_cf1AutoDetectInputButton->setChecked(false);

    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotCF2InputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotCF2InputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotCF2InputValueChanged(quint32 uni, quint32 ch)
{
    if (m_cf2InputSource != NULL)
       delete m_cf2InputSource;
    m_cf2InputSource = new QLCInputSource(uni, (m_cueList->page() << 16) | ch);
    updateCrossfadeInputSource();
}

void VCCueListProperties::updateCrossfadeInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_cf1InputSource, uniName, chName) == true)
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

    if (m_doc->inputOutputMap()->inputSourceNames(m_cf2InputSource, uniName, chName) == true)
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
