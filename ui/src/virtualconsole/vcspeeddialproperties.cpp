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

#include <QDebug>

#include "vcspeeddialproperties.h"
#include "vcspeeddialfunction.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "assignhotkey.h"
#include "vcspeeddial.h"
#include "inputpatch.h"
#include "speeddial.h"
#include "apputil.h"
#include "doc.h"

#define PROP_ID  Qt::UserRole

#define COL_NAME     0
#define COL_FADEIN   1
#define COL_FADEOUT  2
#define COL_DURATION 3


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
    foreach (const VCSpeedDialFunction &speeddialfunction, m_dial->functions())
        createFunctionItem(speeddialfunction);

    /* Forbid editing the function name */
    m_tree->setItemDelegateForColumn(COL_NAME, new NoEditDelegate(this));
    /* Combobox for editing the multipliers */
    const QStringList &multiplierNames = VCSpeedDialFunction::speedMultiplierNames();
    m_tree->setItemDelegateForColumn(COL_FADEIN, new ComboBoxDelegate(multiplierNames, this));
    m_tree->setItemDelegateForColumn(COL_FADEOUT, new ComboBoxDelegate(multiplierNames, this));
    m_tree->setItemDelegateForColumn(COL_DURATION, new ComboBoxDelegate(multiplierNames, this));

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

    ushort dialMask = m_dial->visibilityMask();
    if (dialMask & SpeedDial::PlusMinus) m_pmCheck->setChecked(true);
    if (dialMask & SpeedDial::Dial) m_dialCheck->setChecked(true);
    if (dialMask & SpeedDial::Tap) m_tapCheck->setChecked(true);
    if (dialMask & SpeedDial::Hours) m_hoursCheck->setChecked(true);
    if (dialMask & SpeedDial::Minutes) m_minCheck->setChecked(true);
    if (dialMask & SpeedDial::Seconds) m_secCheck->setChecked(true);
    if (dialMask & SpeedDial::Milliseconds) m_msCheck->setChecked(true);
    if (dialMask & SpeedDial::Infinite) m_infiniteCheck->setChecked(true);

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

    /* Input sources */
    m_dial->setAbsoluteValueRange(m_absoluteMinSpin->value() * 1000,
                                  m_absoluteMaxSpin->value() * 1000);
    m_dial->setInputSource(m_absoluteInputSource, VCSpeedDial::absoluteInputSourceId);
    m_dial->setInputSource(m_tapInputSource, VCSpeedDial::tapInputSourceId);

    m_dial->setKeySequence(m_tapKeySequence);

    ushort dialMask = 0;
    if (m_pmCheck->isChecked()) dialMask |= SpeedDial::PlusMinus;
    if (m_dialCheck->isChecked()) dialMask |= SpeedDial::Dial;
    if (m_tapCheck->isChecked()) dialMask |= SpeedDial::Tap;
    if (m_hoursCheck->isChecked()) dialMask |= SpeedDial::Hours;
    if (m_minCheck->isChecked()) dialMask |= SpeedDial::Minutes;
    if (m_secCheck->isChecked()) dialMask |= SpeedDial::Seconds;
    if (m_msCheck->isChecked()) dialMask |= SpeedDial::Milliseconds;
    if (m_infiniteCheck->isChecked()) dialMask |= SpeedDial::Infinite;

    m_dial->setVisibilityMask(dialMask);

    QDialog::accept();
}

/****************************************************************************
 * Functions page
 ****************************************************************************/

void VCSpeedDialProperties::slotAddClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    QList <quint32> ids;
    foreach (const VCSpeedDialFunction &speeddialfunction, functions())
        ids.append(speeddialfunction.functionId);
    fs.setDisabledFunctions(ids);
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

QList <VCSpeedDialFunction> VCSpeedDialProperties::functions() const
{
    QList <VCSpeedDialFunction> list;
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);

        QVariant id = item->data(COL_NAME, PROP_ID);
        if (id.isValid() == true)
        {
            VCSpeedDialFunction speeddialfunction(id.toUInt());
            speeddialfunction.fadeInMultiplier = static_cast<VCSpeedDialFunction::SpeedMultiplier>(item->data(COL_FADEIN, PROP_ID).toUInt());
            speeddialfunction.fadeOutMultiplier = static_cast<VCSpeedDialFunction::SpeedMultiplier>(item->data(COL_FADEOUT, PROP_ID).toUInt());
            speeddialfunction.durationMultiplier = static_cast<VCSpeedDialFunction::SpeedMultiplier>(item->data(COL_DURATION, PROP_ID).toUInt());
            list.append(speeddialfunction);
        }
    }

    return list;
}

void VCSpeedDialProperties::createFunctionItem(const VCSpeedDialFunction &speeddialfunction)
{
    Function* function = m_doc->function(speeddialfunction.functionId);
    if (function != NULL)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(COL_NAME, function->name());
        item->setData(COL_NAME, PROP_ID, speeddialfunction.functionId);

        const QStringList &multiplierNames = VCSpeedDialFunction::speedMultiplierNames();

        item->setText(COL_FADEIN, multiplierNames[speeddialfunction.fadeInMultiplier]);
        item->setData(COL_FADEIN, PROP_ID, speeddialfunction.fadeInMultiplier);
        item->setText(COL_FADEOUT, multiplierNames[speeddialfunction.fadeOutMultiplier]);
        item->setData(COL_FADEOUT, PROP_ID, speeddialfunction.fadeOutMultiplier);
        item->setText(COL_DURATION, multiplierNames[speeddialfunction.durationMultiplier]);
        item->setData(COL_DURATION, PROP_ID, speeddialfunction.durationMultiplier);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
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
    if (m_absoluteInputSource != NULL)
        delete m_absoluteInputSource;
    m_absoluteInputSource = new QLCInputSource(universe, (m_dial->page() << 16) | channel);
    updateInputSources();
}

void VCSpeedDialProperties::slotTapInputValueChanged(quint32 universe, quint32 channel)
{
    if (m_tapInputSource != NULL)
        delete m_tapInputSource;
    m_tapInputSource = new QLCInputSource(universe, (m_dial->page() << 16) | channel);
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
        if (m_absoluteInputSource != NULL)
            delete m_absoluteInputSource;
        m_absoluteInputSource = new QLCInputSource(sic.universe(), sic.channel());
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
        if (m_tapInputSource != NULL)
            delete m_tapInputSource;
        m_tapInputSource = new QLCInputSource(sic.universe(), sic.channel());
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
