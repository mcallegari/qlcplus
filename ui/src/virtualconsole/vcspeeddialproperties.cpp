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
#include "inputselectionwidget.h"
#include "vcspeeddialfunction.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "vcspeeddial.h"
#include "vcspeeddialpreset.h"
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
    , m_copyItem(NULL)
{
    Q_ASSERT(dial != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    // IDs 0-15 are reserved for speed dial base controls
    m_lastAssignedID = 15;

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
    connect(m_absolutePrecisionCb, SIGNAL(toggled(bool)),
            this, SLOT(slotAbsolutePrecisionCbChecked(bool)));
    if (m_dial->absoluteValueMin() % (1000) || m_dial->absoluteValueMax() % (1000))
    {
        m_absolutePrecisionCb->setChecked(true);
        m_absoluteMinSpin->setValue(m_dial->absoluteValueMin());
        m_absoluteMaxSpin->setValue(m_dial->absoluteValueMax());
    }
    else
    {
        m_absolutePrecisionCb->setChecked(false);
        m_absoluteMinSpin->setValue(m_dial->absoluteValueMin() / 1000);
        m_absoluteMaxSpin->setValue(m_dial->absoluteValueMax() / 1000);
    }
    m_absoluteInputWidget = new InputSelectionWidget(m_doc, this);
    m_absoluteInputWidget->setInputSource(m_dial->inputSource(VCSpeedDial::absoluteInputSourceId));
    m_absoluteInputWidget->setWidgetPage(m_dial->page());
    m_absoluteInputWidget->setKeyInputVisibility(false);
    m_absoluteInputWidget->show();
    m_absoluteInputLayout->addWidget(m_absoluteInputWidget);

    /* Tap input */
    m_tapInputWidget = new InputSelectionWidget(m_doc, this);
    m_tapInputWidget->setInputSource(m_dial->inputSource(VCSpeedDial::tapInputSourceId));
    m_tapInputWidget->setWidgetPage(m_dial->page());
    m_tapInputWidget->setKeySequence(dial->tapKeySequence());
    m_tapInputWidget->show();
    m_tapInputLayout->addWidget(m_tapInputWidget);

    /* Apply input */
    m_applyInputWidget = new InputSelectionWidget(m_doc, this);
    m_applyInputWidget->setInputSource(m_dial->inputSource(VCSpeedDial::applyInputSourceId));
    m_applyInputWidget->setWidgetPage(m_dial->page());
    m_applyInputWidget->setKeySequence(dial->applyKeySequence());
    m_applyInputWidget->show();
    m_applyInputLayout->addWidget(m_applyInputWidget);

    // Mult/Div options
    m_resetFactorOnDialChangeCb->setChecked(m_dial->resetFactorOnDialChange());

    /* Mult input */
    m_multInputWidget = new InputSelectionWidget(m_doc, this);
    m_multInputWidget->setTitle(tr("Multiply by 2 Input"));
    m_multInputWidget->setInputSource(m_dial->inputSource(VCSpeedDial::multInputSourceId));
    m_multInputWidget->setWidgetPage(m_dial->page());
    m_multInputWidget->setKeySequence(dial->multKeySequence());
    m_multInputWidget->show();
    m_multInputLayout->addWidget(m_multInputWidget);

    /* Div input */
    m_divInputWidget = new InputSelectionWidget(m_doc, this);
    m_divInputWidget->setTitle(tr("Divide by 2 Input"));
    m_divInputWidget->setInputSource(m_dial->inputSource(VCSpeedDial::divInputSourceId));
    m_divInputWidget->setWidgetPage(m_dial->page());
    m_divInputWidget->setKeySequence(dial->divKeySequence());
    m_divInputWidget->show();
    m_divInputLayout->addWidget(m_divInputWidget);

    /* MultDiv Reset input */
    m_multDivResetInputWidget = new InputSelectionWidget(m_doc, this);
    m_multDivResetInputWidget->setTitle(tr("Factor Reset Input"));
    m_multDivResetInputWidget->setInputSource(m_dial->inputSource(VCSpeedDial::multDivResetInputSourceId));
    m_multDivResetInputWidget->setWidgetPage(m_dial->page());
    m_multDivResetInputWidget->setKeySequence(dial->multDivResetKeySequence());
    m_multDivResetInputWidget->show();
    m_multDivResetInputLayout->addWidget(m_multDivResetInputWidget);

    /* Visibility */
    quint32 dialMask = m_dial->visibilityMask();
    if (dialMask & SpeedDial::PlusMinus) m_pmCheck->setChecked(true);
    if (dialMask & SpeedDial::Dial) m_dialCheck->setChecked(true);
    if (dialMask & SpeedDial::Tap) m_tapCheck->setChecked(true);
    if (dialMask & SpeedDial::Hours) m_hoursCheck->setChecked(true);
    if (dialMask & SpeedDial::Minutes) m_minCheck->setChecked(true);
    if (dialMask & SpeedDial::Seconds) m_secCheck->setChecked(true);
    if (dialMask & SpeedDial::Milliseconds) m_msCheck->setChecked(true);
    if (dialMask & VCSpeedDial::MultDiv) m_mdCheck->setChecked(true);
    if (dialMask & VCSpeedDial::Apply) m_applyCheck->setChecked(true);

    /* Presets */
    foreach (VCSpeedDialPreset const* preset, m_dial->presets())
    {
        m_presets.append(new VCSpeedDialPreset(*preset));
        if (preset->m_id > m_lastAssignedID)
            m_lastAssignedID = preset->m_id;
    }
    m_presetsTree->setSelectionMode(QAbstractItemView::SingleSelection);
    updateTree();

    m_presetInputWidget = new InputSelectionWidget(m_doc, this);
    m_presetInputWidget->setCustomFeedbackVisibility(true);
    m_presetInputWidget->setWidgetPage(m_dial->page());
    m_presetInputWidget->show();
    m_presetInputLayout->addWidget(m_presetInputWidget);

    connect(m_presetInputWidget, SIGNAL(inputValueChanged(quint32,quint32)),
            this, SLOT(slotInputValueChanged(quint32,quint32)));
    connect(m_presetInputWidget, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(slotKeySequenceChanged(QKeySequence)));

    connect(m_presetsTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotTreeSelectionChanged()));

    connect(m_addPresetButton, SIGNAL(clicked()),
            this, SLOT(slotAddPresetClicked()));
    connect(m_removePresetButton, SIGNAL(clicked()),
            this, SLOT(slotRemovePresetClicked()));
    connect(m_presetNameEdit, SIGNAL(textEdited(QString const&)),
            this, SLOT(slotPresetNameEdited(QString const&)));

    connect(m_speedDialWidget, SIGNAL(valueChanged(int)),
            this, SLOT(slotSpeedDialWidgetValueChanged(int)));

    connect(m_addButton, SIGNAL(clicked()),
            this, SLOT(slotAddClicked()));
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveClicked()));

    connect(m_copyFactorsButton, SIGNAL(clicked()),
            this, SLOT(slotCopyFactorsClicked()));
    connect(m_pasteFactorsButton, SIGNAL(clicked()),
            this, SLOT(slotPasteFactorsClicked()));
}

VCSpeedDialProperties::~VCSpeedDialProperties()
{
    foreach (VCSpeedDialPreset* preset, m_presets)
    {
        delete preset;
    }
}

void VCSpeedDialProperties::accept()
{
    /* Name */
    m_dial->setCaption(m_nameEdit->text());

    /* Functions */
    m_dial->setFunctions(functions());

    /* Input sources */
    if (m_absolutePrecisionCb->isChecked())
        m_dial->setAbsoluteValueRange(m_absoluteMinSpin->value(),
                                      m_absoluteMaxSpin->value());
    else
        m_dial->setAbsoluteValueRange(m_absoluteMinSpin->value() * 1000,
                                      m_absoluteMaxSpin->value() * 1000);
    m_dial->setInputSource(m_absoluteInputWidget->inputSource(), VCSpeedDial::absoluteInputSourceId);

    m_dial->setInputSource(m_tapInputWidget->inputSource(), VCSpeedDial::tapInputSourceId);
    m_dial->setTapKeySequence(m_tapInputWidget->keySequence());

    m_dial->setInputSource(m_applyInputWidget->inputSource(), VCSpeedDial::applyInputSourceId);
    m_dial->setApplyKeySequence(m_applyInputWidget->keySequence());

    // Mult & Div
    m_dial->setResetFactorOnDialChange(m_resetFactorOnDialChangeCb->isChecked());
    m_dial->setInputSource(m_multInputWidget->inputSource(), VCSpeedDial::multInputSourceId);
    m_dial->setMultKeySequence(m_multInputWidget->keySequence());
    m_dial->setInputSource(m_divInputWidget->inputSource(), VCSpeedDial::divInputSourceId);
    m_dial->setDivKeySequence(m_divInputWidget->keySequence());
    m_dial->setInputSource(m_multDivResetInputWidget->inputSource(), VCSpeedDial::multDivResetInputSourceId);
    m_dial->setMultDivResetKeySequence(m_multDivResetInputWidget->keySequence());

    /* Visibility */
    quint32 dialMask = 0;
    if (m_pmCheck->isChecked()) dialMask |= SpeedDial::PlusMinus;
    if (m_dialCheck->isChecked()) dialMask |= SpeedDial::Dial;
    if (m_tapCheck->isChecked()) dialMask |= SpeedDial::Tap;
    if (m_hoursCheck->isChecked()) dialMask |= SpeedDial::Hours;
    if (m_minCheck->isChecked()) dialMask |= SpeedDial::Minutes;
    if (m_secCheck->isChecked()) dialMask |= SpeedDial::Seconds;
    if (m_msCheck->isChecked()) dialMask |= SpeedDial::Milliseconds;
    if (m_mdCheck->isChecked()) dialMask |= VCSpeedDial::MultDiv;
    if (m_applyCheck->isChecked()) dialMask |= VCSpeedDial::Apply;
    m_dial->setVisibilityMask(dialMask);

    /* Presets */
    m_dial->resetPresets();
    for (int i = 0; i < m_presets.count(); i++)
        m_dial->addPreset(*m_presets.at(i));

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

void VCSpeedDialProperties::slotCopyFactorsClicked()
{
    QList <QTreeWidgetItem*> allSelected = m_tree->selectedItems();
    if (allSelected.isEmpty())
        return;

    m_copyItem = allSelected.first();
    m_pasteFactorsButton->setEnabled(true);
}

void VCSpeedDialProperties::slotPasteFactorsClicked()
{
    if (m_copyItem == NULL)
        return;

    // is only called after the paste button has been enabled and copiedFactorValues is filled with data
    const QStringList &multiplierNames = VCSpeedDialFunction::speedMultiplierNames();

    VCSpeedDialFunction::SpeedMultiplier fadeInMultiplier = static_cast<VCSpeedDialFunction::SpeedMultiplier>(m_copyItem->data(COL_FADEIN, PROP_ID).toUInt());
    VCSpeedDialFunction::SpeedMultiplier fadeOutMultiplier = static_cast<VCSpeedDialFunction::SpeedMultiplier>(m_copyItem->data(COL_FADEOUT, PROP_ID).toUInt());
    VCSpeedDialFunction::SpeedMultiplier durationMultiplier = static_cast<VCSpeedDialFunction::SpeedMultiplier>(m_copyItem->data(COL_DURATION, PROP_ID).toUInt());

    foreach (QTreeWidgetItem* item, m_tree->selectedItems())
    {
        Q_ASSERT(item != NULL);

        QVariant id = item->data(COL_NAME, PROP_ID);
        if (id.isValid() == true)
        {
            item->setText(COL_FADEIN, multiplierNames[fadeInMultiplier]);
            item->setData(COL_FADEIN, PROP_ID, fadeInMultiplier);
            item->setText(COL_FADEOUT, multiplierNames[fadeOutMultiplier]);
            item->setData(COL_FADEOUT, PROP_ID, fadeOutMultiplier);
            item->setText(COL_DURATION, multiplierNames[durationMultiplier]);
            item->setData(COL_DURATION, PROP_ID, durationMultiplier);
        }
    }
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

void VCSpeedDialProperties::slotAbsolutePrecisionCbChecked(bool checked)
{
    if (checked)
    {
        m_absoluteMinSpin->setSuffix("ms");
        m_absoluteMinSpin->setMaximum(600 * 1000);
        m_absoluteMinSpin->setValue(m_absoluteMinSpin->value() * 1000);
        m_absoluteMaxSpin->setSuffix("ms");
        m_absoluteMaxSpin->setMaximum(600 * 1000);
        m_absoluteMaxSpin->setValue(m_absoluteMaxSpin->value() * 1000);
    }
    else
    {
        m_absoluteMinSpin->setSuffix("s");
        m_absoluteMinSpin->setValue(m_absoluteMinSpin->value() / 1000);
        m_absoluteMinSpin->setMaximum(600);
        m_absoluteMaxSpin->setSuffix("s");
        m_absoluteMaxSpin->setValue(m_absoluteMaxSpin->value() / 1000);
        m_absoluteMaxSpin->setMaximum(600);
    }
}

/*********************************************************************
 * Presets
 *********************************************************************/

void VCSpeedDialProperties::updateTree()
{
    m_presetsTree->blockSignals(true);
    m_presetsTree->clear();
    foreach (VCSpeedDialPreset* preset, m_presets)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_presetsTree);
        item->setData(0, Qt::UserRole, preset->m_id);
        item->setText(0, preset->m_name);
        item->setText(1, Function::speedToString(preset->m_value));
    }
    m_presetsTree->resizeColumnToContents(0);
    m_presetsTree->blockSignals(false);
}

void VCSpeedDialProperties::updateTreeItem(VCSpeedDialPreset const& preset)
{
    m_presetsTree->blockSignals(true);
    m_presetsTree->resizeColumnToContents(0);
    for (int i = 0; i < m_presetsTree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* treeItem = m_presetsTree->topLevelItem(i);
        if (treeItem->data(0, Qt::UserRole).toUInt() == preset.m_id)
        {
            treeItem->setText(0, preset.m_name);
            treeItem->setText(1, Function::speedToString(preset.m_value));
            m_presetsTree->blockSignals(false);
            return;
        }
    }
    Q_ASSERT(false);
}

VCSpeedDialPreset* VCSpeedDialProperties::getSelectedPreset()
{
    if (m_presetsTree->selectedItems().isEmpty())
        return NULL;

    QTreeWidgetItem* item = m_presetsTree->selectedItems().first();
    if (item != NULL)
    {
        quint8 presetID = item->data(0, Qt::UserRole).toUInt();
        foreach (VCSpeedDialPreset* preset, m_presets)
        {
            if (preset->m_id == presetID)
                return preset;
        }
    }

    Q_ASSERT(false);
    return NULL;
}

void VCSpeedDialProperties::addPreset(VCSpeedDialPreset *preset)
{
    m_presets.append(preset);
}

void VCSpeedDialProperties::removePreset(quint8 id)
{
    for (int i = 0; i < m_presets.count(); i++)
    {
        if (m_presets.at(i)->m_id == id)
        {
            m_presets.removeAt(i);
            return;
        }
    }
}

void VCSpeedDialProperties::slotAddPresetClicked()
{
    VCSpeedDialPreset *newPreset = new VCSpeedDialPreset(++m_lastAssignedID);
    newPreset->m_value = 1000;
    newPreset->m_name = Function::speedToString(1000);
    addPreset(newPreset);
    updateTree();
}

void VCSpeedDialProperties::slotRemovePresetClicked()
{
    if (m_presetsTree->selectedItems().isEmpty())
        return;
    QTreeWidgetItem *selItem = m_presetsTree->selectedItems().first();
    quint8 ctlID = selItem->data(0, Qt::UserRole).toUInt();
    removePreset(ctlID);
    updateTree();
}

void VCSpeedDialProperties::slotTreeSelectionChanged()
{
    VCSpeedDialPreset* preset = getSelectedPreset();

    if (preset != NULL)
    {
        m_presetInputWidget->setInputSource(preset->m_inputSource);
        m_presetInputWidget->setKeySequence(preset->m_keySequence.toString(QKeySequence::NativeText));
        m_presetNameEdit->setText(preset->m_name);
        m_speedDialWidget->setValue(preset->m_value);
    }
}

void VCSpeedDialProperties::slotPresetNameEdited(QString const& newName)
{
    VCSpeedDialPreset* preset = getSelectedPreset();

    if (preset != NULL)
    {
        preset->m_name = newName;

        updateTreeItem(*preset);
    }
}

void VCSpeedDialProperties::slotSpeedDialWidgetValueChanged(int ms)
{
    VCSpeedDialPreset* preset = getSelectedPreset();

    if (preset != NULL)
    {
        if (Function::stringToSpeed(preset->m_name) == uint(preset->m_value))
        {
            preset->m_name = Function::speedToString(ms);
            m_presetNameEdit->blockSignals(true);
            m_presetNameEdit->setText(preset->m_name);
            m_presetNameEdit->blockSignals(false);
        }
        preset->m_value = ms;

        updateTreeItem(*preset);
    }
}

void VCSpeedDialProperties::slotInputValueChanged(quint32 universe, quint32 channel)
{
    Q_UNUSED(universe);
    Q_UNUSED(channel);

    VCSpeedDialPreset *preset = getSelectedPreset();

    if (preset != NULL)
        preset->m_inputSource = m_presetInputWidget->inputSource();
}

void VCSpeedDialProperties::slotKeySequenceChanged(QKeySequence key)
{
    VCSpeedDialPreset *preset = getSelectedPreset();

    if (preset != NULL)
        preset->m_keySequence = key;
}

