/*
  Q Light Controller
  vcspeeddialproperties.cpp

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

#include "vcspeeddialproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "assignhotkey.h"
#include "vcspeeddial.h"
#include "inputpatch.h"
#include "doc.h"

#define COL_NAME 0
#define COL_TYPE 1
#define PROP_ID  Qt::UserRole

VCSpeedDialProperties::VCSpeedDialProperties(VCSpeedDial* dial, Doc* doc)
    : QDialog(dial)
    , m_dial(dial)
    , m_doc(doc)
{
    Q_ASSERT(dial != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    /* Name */
    m_nameEdit->setText(m_dial->caption());

    /* Functions */
    foreach (quint32 id, m_dial->functions())
        createFunctionItem(id);

    /* Speed types */
    if (dial->speedTypes() & VCSpeedDial::FadeIn)
        m_fadeInCheck->setChecked(true);
    if (dial->speedTypes() & VCSpeedDial::FadeOut)
        m_fadeOutCheck->setChecked(true);
    if (dial->speedTypes() & VCSpeedDial::Duration)
        m_durationCheck->setChecked(true);

    m_fadeInCombo->addItem("1/16");
    m_fadeInCombo->addItem("1/8");
    m_fadeInCombo->addItem("1/4");
    m_fadeInCombo->addItem("1/2");
    m_fadeInCombo->addItem("1");
    m_fadeInCombo->addItem("2");
    m_fadeInCombo->addItem("4");
    m_fadeInCombo->addItem("8");
    m_fadeInCombo->addItem("16");
    m_fadeInCombo->setCurrentIndex(dial->fadeInMultiplier());
    m_fadeOutCombo->addItem("1/16");
    m_fadeOutCombo->addItem("1/8");
    m_fadeOutCombo->addItem("1/4");
    m_fadeOutCombo->addItem("1/2");
    m_fadeOutCombo->addItem("1");
    m_fadeOutCombo->addItem("2");
    m_fadeOutCombo->addItem("4");
    m_fadeOutCombo->addItem("8");
    m_fadeOutCombo->addItem("16");
    m_fadeOutCombo->setCurrentIndex(dial->fadeOutMultiplier());
    m_durationCombo->addItem("1/16");
    m_durationCombo->addItem("1/8");
    m_durationCombo->addItem("1/4");
    m_durationCombo->addItem("1/2");
    m_durationCombo->addItem("1");
    m_durationCombo->addItem("2");
    m_durationCombo->addItem("4");
    m_durationCombo->addItem("8");
    m_durationCombo->addItem("16");
    m_durationCombo->setCurrentIndex(dial->durationMultiplier());

    /* Absolute input */
    m_absoluteMinSpin->setValue(m_dial->absoluteValueMin() / 1000);
    m_absoluteMaxSpin->setValue(m_dial->absoluteValueMax() / 1000);
    m_absoluteInputSource = m_dial->inputSource(VCSpeedDial::absoluteInputSourceId);

    /* Tap input */
    m_tapInputSource = m_dial->inputSource(VCSpeedDial::tapInputSourceId);

    /* Key sequence */
    m_tapKeySequence = QKeySequence(dial->keySequence());
    m_keyEdit->setText(m_tapKeySequence.toString(QKeySequence::NativeText));

    updateInputSources();

    connect(m_autoDetectAbsoluteInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectAbsoluteInputSourceToggled(bool)));
    connect(m_autoDetectTapInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectTapInputSourceToggled(bool)));
    connect(m_attachKey, SIGNAL(clicked()), this, SLOT(slotAttachKey()));
    connect(m_detachKey, SIGNAL(clicked()), this, SLOT(slotDetachKey()));
}

VCSpeedDialProperties::~VCSpeedDialProperties()
{
}

void VCSpeedDialProperties::accept()
{
    /* Name */
    m_dial->setCaption(m_nameEdit->text());

    /* Functions */
    m_dial->setFunctions(functions());

    /* Speed types */
    VCSpeedDial::SpeedTypes types = 0;
    if (m_fadeInCheck->isChecked() == true)
        types |= VCSpeedDial::FadeIn;
    if (m_fadeOutCheck->isChecked() == true)
        types |= VCSpeedDial::FadeOut;
    if (m_durationCheck->isChecked() == true)
        types |= VCSpeedDial::Duration;
    m_dial->setSpeedTypes(types);

    m_dial->setFadeInMultiplier(VCSpeedDial::SpeedMultiplier(m_fadeInCombo->currentIndex()));
    m_dial->setFadeOutMultiplier(VCSpeedDial::SpeedMultiplier(m_fadeOutCombo->currentIndex()));
    m_dial->setDurationMultiplier(VCSpeedDial::SpeedMultiplier(m_durationCombo->currentIndex()));

    /* Input sources */
    m_dial->setAbsoluteValueRange(m_absoluteMinSpin->value() * 1000,
                                  m_absoluteMaxSpin->value() * 1000);
    m_dial->setInputSource(m_absoluteInputSource, VCSpeedDial::absoluteInputSourceId);
    m_dial->setInputSource(m_tapInputSource, VCSpeedDial::tapInputSourceId);

    m_dial->setKeySequence(m_tapKeySequence);

    QDialog::accept();
}

/****************************************************************************
 * Functions page
 ****************************************************************************/

void VCSpeedDialProperties::slotAddClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setDisabledFunctions(functions().toList());
    if (fs.exec() == QDialog::Accepted)
    {
        foreach (quint32 id, fs.selection())
            createFunctionItem(id);
    }
}

void VCSpeedDialProperties::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        delete it.next();
}

QSet <quint32> VCSpeedDialProperties::functions() const
{
    QSet <quint32> set;
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);

        QVariant var = item->data(COL_NAME, PROP_ID);
        if (var.isValid() == true)
            set << var.toUInt();
    }

    return set;
}

void VCSpeedDialProperties::createFunctionItem(quint32 id)
{
    Function* function = m_doc->function(id);
    if (function != NULL)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(COL_NAME, function->name());
        item->setText(COL_TYPE, function->typeString());
        item->setData(COL_NAME, PROP_ID, id);
    }
}

/****************************************************************************
 * Input page
 ****************************************************************************/

void VCSpeedDialProperties::updateInputSources()
{
    QString uniName;
    QString chName;

    // Absolute
    if (m_doc->inputOutputMap()->inputSourceNames(m_absoluteInputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }
    m_absoluteInputUniverseEdit->setText(uniName);
    m_absoluteInputChannelEdit->setText(chName);

    // Tap
    if (m_doc->inputOutputMap()->inputSourceNames(m_tapInputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }
    m_tapInputUniverseEdit->setText(uniName);
    m_tapInputChannelEdit->setText(chName);
}

void VCSpeedDialProperties::slotAbsoluteInputValueChanged(quint32 universe, quint32 channel)
{
    qDebug() << "Signal received !";
    m_absoluteInputSource = QLCInputSource(universe, (m_dial->page() << 16) | channel);
    updateInputSources();
}

void VCSpeedDialProperties::slotTapInputValueChanged(quint32 universe, quint32 channel)
{
    m_tapInputSource = QLCInputSource(universe, (m_dial->page() << 16) | channel);
    updateInputSources();
}

void VCSpeedDialProperties::slotAutoDetectAbsoluteInputSourceToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotAbsoluteInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotAbsoluteInputValueChanged(quint32,quint32)));
    }
}

void VCSpeedDialProperties::slotChooseAbsoluteInputSourceClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_absoluteInputSource = QLCInputSource(sic.universe(), sic.channel());
        updateInputSources();
    }
}

void VCSpeedDialProperties::slotAutoDetectTapInputSourceToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotTapInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotTapInputValueChanged(quint32,quint32)));
    }
}

void VCSpeedDialProperties::slotChooseTapInputSourceClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_tapInputSource = QLCInputSource(sic.universe(), sic.channel());
        updateInputSources();
    }
}

void VCSpeedDialProperties::slotAttachKey()
{
    AssignHotKey ahk(this, m_tapKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_tapKeySequence = QKeySequence(ahk.keySequence());
        m_keyEdit->setText(m_tapKeySequence.toString(QKeySequence::NativeText));
    }
}

void VCSpeedDialProperties::slotDetachKey()
{
    m_tapKeySequence = QKeySequence();
    m_keyEdit->setText(m_tapKeySequence.toString(QKeySequence::NativeText));
}
