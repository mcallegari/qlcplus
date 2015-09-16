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
#include "speeddialwidget.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "assignhotkey.h"
#include "vcspeeddial.h"
#include "vcspeeddialpreset.h"
#include "inputpatch.h"
#include "speeddial.h"
#include "apputil.h"
#include "doc.h"

#define PROP_ID  Qt::UserRole

#define COL_NAME     0
#define COL_FADEIN   1
#define COL_FADEOUT  2
#define COL_DURATION 3

#define MS_DIV    10

VCSpeedDialProperties::VCSpeedDialProperties(VCSpeedDial* dial, Doc* doc)
    : QDialog(dial)
    , m_dial(dial)
    , m_doc(doc)
    , m_speedDialWidget(NULL)
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
    if (m_dial->absoluteValueMin() % (1000 / MS_DIV) || m_dial->absoluteValueMax() % (1000 / MS_DIV))
    {
        m_absolutePrecisionCb->setChecked(true);
        m_absoluteMinSpin->setValue(m_dial->absoluteValueMin() / MS_DIV);
        m_absoluteMaxSpin->setValue(m_dial->absoluteValueMax() / MS_DIV);
    }
    else
    {
        m_absolutePrecisionCb->setChecked(false);
        m_absoluteMinSpin->setValue(m_dial->absoluteValueMin() / 1000);
        m_absoluteMaxSpin->setValue(m_dial->absoluteValueMax() / 1000);
    }
    m_absoluteInputSource = m_dial->inputSource(VCSpeedDial::absoluteInputSourceId);

    /* Tap input */
    m_tapInputSource = m_dial->inputSource(VCSpeedDial::tapInputSourceId);

    /* Key sequence */
    m_tapKeySequence = QKeySequence(dial->keySequence());
    m_keyEdit->setText(m_tapKeySequence.toString(QKeySequence::NativeText));

    updateInputSources();

    /* Visibility */
    ushort dialMask = m_dial->visibilityMask();
    if (dialMask & SpeedDial::PlusMinus) m_pmCheck->setChecked(true);
    if (dialMask & SpeedDial::MultDiv) m_mdCheck->setChecked(true);
    if (dialMask & SpeedDial::Dial) m_dialCheck->setChecked(true);
    if (dialMask & SpeedDial::Tap) m_tapCheck->setChecked(true);
    if (dialMask & SpeedDial::Apply) m_applyCheck->setChecked(true);
    if (dialMask & SpeedDial::Hours) m_hoursCheck->setChecked(true);
    if (dialMask & SpeedDial::Minutes) m_minCheck->setChecked(true);
    if (dialMask & SpeedDial::Seconds) m_secCheck->setChecked(true);
    if (dialMask & SpeedDial::Milliseconds) m_msCheck->setChecked(true);

    connect(m_attachKey, SIGNAL(clicked()), this, SLOT(slotAttachKey()));
    connect(m_detachKey, SIGNAL(clicked()), this, SLOT(slotDetachKey()));

    /* Presets */
    foreach(VCSpeedDialPreset const* preset, m_dial->presets())
    {
        m_presets.append(new VCSpeedDialPreset(*preset));
        if (preset->m_id > m_lastAssignedID)
            m_lastAssignedID = preset->m_id;
    }
    m_presetsTree->setSelectionMode(QAbstractItemView::SingleSelection);
    updateTree();

    connect(m_presetsTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotTreeSelectionChanged()));

    connect(m_addPresetButton, SIGNAL(clicked()),
            this, SLOT(slotAddPresetClicked()));
    connect(m_removePresetButton, SIGNAL(clicked()),
            this, SLOT(slotRemovePresetClicked()));
    connect(m_showPresetNameCb, SIGNAL(clicked()),
            this, SLOT(slotShowPresetNameClicked()));
    connect(m_presetNameEdit, SIGNAL(textEdited(QString const&)),
            this, SLOT(slotPresetNameEdited(QString const&)));
    connect(m_speedDialWidgetButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSpeedDialWidgetToggle(bool)));

    connect(m_adPresetInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectPresetInputToggled(bool)));
    connect(m_choosePresetInputButton, SIGNAL(clicked()),
            this, SLOT(slotChoosePresetInputClicked()));

    connect(m_attachPresetKey, SIGNAL(clicked()), this, SLOT(slotAttachPresetKey()));
    connect(m_detachPresetKey, SIGNAL(clicked()), this, SLOT(slotDetachPresetKey()));
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
        m_dial->setAbsoluteValueRange(m_absoluteMinSpin->value() * MS_DIV,
                                      m_absoluteMaxSpin->value() * MS_DIV);
    else
        m_dial->setAbsoluteValueRange(m_absoluteMinSpin->value() * 1000,
                                      m_absoluteMaxSpin->value() * 1000);
    m_dial->setInputSource(m_absoluteInputSource, VCSpeedDial::absoluteInputSourceId);
    m_dial->setInputSource(m_tapInputSource, VCSpeedDial::tapInputSourceId);

    m_dial->setKeySequence(m_tapKeySequence);

    /* Visibility */
    ushort dialMask = 0;
    if (m_pmCheck->isChecked()) dialMask |= SpeedDial::PlusMinus;
    if (m_mdCheck->isChecked()) dialMask |= SpeedDial::MultDiv;
    if (m_dialCheck->isChecked()) dialMask |= SpeedDial::Dial;
    if (m_tapCheck->isChecked()) dialMask |= SpeedDial::Tap;
    if (m_applyCheck->isChecked()) dialMask |= SpeedDial::Apply;
    if (m_hoursCheck->isChecked()) dialMask |= SpeedDial::Hours;
    if (m_minCheck->isChecked()) dialMask |= SpeedDial::Minutes;
    if (m_secCheck->isChecked()) dialMask |= SpeedDial::Seconds;
    if (m_msCheck->isChecked()) dialMask |= SpeedDial::Milliseconds;
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
    m_absoluteInputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(universe, (m_dial->page() << 16) | channel));
    updateInputSources();
}

void VCSpeedDialProperties::slotTapInputValueChanged(quint32 universe, quint32 channel)
{
    m_tapInputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(universe, (m_dial->page() << 16) | channel));
    updateInputSources();
}

void VCSpeedDialProperties::slotAbsolutePrecisionCbChecked(bool checked)
{
    if (checked)
    {
        m_absoluteMinSpin->setSuffix("0ms");
        m_absoluteMinSpin->setMaximum(600 * (1000 / MS_DIV));
        m_absoluteMinSpin->setValue(m_absoluteMinSpin->value() * (1000 / MS_DIV));
        m_absoluteMaxSpin->setSuffix("0ms");
        m_absoluteMaxSpin->setMaximum(600 * (1000 / MS_DIV));
        m_absoluteMaxSpin->setValue(m_absoluteMaxSpin->value() * 1000 / MS_DIV);
    }
    else
    {
        m_absoluteMinSpin->setSuffix("s");
        m_absoluteMinSpin->setValue(m_absoluteMinSpin->value() / (1000 / MS_DIV));
        m_absoluteMinSpin->setMaximum(600);
        m_absoluteMaxSpin->setSuffix("s");
        m_absoluteMaxSpin->setValue(m_absoluteMaxSpin->value() / (1000 / MS_DIV));
        m_absoluteMaxSpin->setMaximum(600);
    }
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
        m_absoluteInputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(sic.universe(), sic.channel()));
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
        m_tapInputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(sic.universe(), sic.channel()));
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

/*********************************************************************
 * Presets
 *********************************************************************/

void VCSpeedDialProperties::updateTree()
{
    m_presetsTree->blockSignals(true);
    m_presetsTree->clear();
    foreach(VCSpeedDialPreset* preset, m_presets)
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
        foreach(VCSpeedDialPreset* preset, m_presets)
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
    for(int i = 0; i < m_presets.count(); i++)
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

void VCSpeedDialProperties::updatePresetInputSource(QSharedPointer<QLCInputSource> const& source)
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(source, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_presetInputUniverseEdit->setText(uniName);
    m_presetInputChannelEdit->setText(chName);
}

void VCSpeedDialProperties::slotTreeSelectionChanged()
{
    VCSpeedDialPreset* preset = getSelectedPreset();

    if (preset != NULL)
    {
        updatePresetInputSource(preset->m_inputSource);
        m_presetKeyEdit->setText(preset->m_keySequence.toString(QKeySequence::NativeText));
        m_showPresetNameCb->blockSignals(true);
        m_showPresetNameCb->setChecked(preset->m_showName);
        m_showPresetNameCb->blockSignals(false);
        m_presetNameEdit->setText(preset->m_name);
        if (m_speedDialWidget != NULL)
            m_speedDialWidget->setDuration(preset->m_value);
    }
}

void VCSpeedDialProperties::slotShowPresetNameClicked()
{
    VCSpeedDialPreset* preset = getSelectedPreset();

    if (preset != NULL)
    {
        preset->m_showName = m_showPresetNameCb->isChecked();
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

void VCSpeedDialProperties::slotSpeedDialWidgetToggle(bool state)
{
    if (state)
    {
        if (m_speedDialWidget == NULL)
        {
            m_speedDialWidget = new SpeedDialWidget(this);
            m_speedDialWidget->setAttribute(Qt::WA_DeleteOnClose);
            m_speedDialWidget->setWindowTitle(tr("%1 preset").arg(m_nameEdit->text()));
            m_speedDialWidget->setDurationTitle("");
            m_speedDialWidget->setFadeInEnabled(false);
            m_speedDialWidget->setFadeInVisible(false);
            m_speedDialWidget->setFadeOutEnabled(false);
            m_speedDialWidget->setFadeOutVisible(false);
            m_speedDialWidget->show();
            connect(m_speedDialWidget, SIGNAL(holdChanged(int)),
                    this, SLOT(slotSpeedDialWidgetDurationChanged(int)));
            connect(m_speedDialWidget, SIGNAL(destroyed(QObject*)),
                    this, SLOT(slotSpeedDialWidgetDestroyed(QObject*)));
        }

        VCSpeedDialPreset* preset = getSelectedPreset();
        if (preset != NULL)
        {
            m_speedDialWidget->setDuration(preset->m_value);
        }
    }
    else
    {
        if (m_speedDialWidget != NULL)
        {
            m_speedDialWidget->deleteLater();
            m_speedDialWidget = NULL;
        }
    }
}

void VCSpeedDialProperties::slotSpeedDialWidgetDestroyed(QObject*)
{
    m_speedDialWidgetButton->setChecked(false);
}

void VCSpeedDialProperties::slotSpeedDialWidgetDurationChanged(int ms)
{
    VCSpeedDialPreset* preset = getSelectedPreset();

    if (preset != NULL)
    {
        preset->m_value = ms;

        updateTreeItem(*preset);
    }
}

void VCSpeedDialProperties::slotAutoDetectPresetInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotPresetInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotPresetInputValueChanged(quint32,quint32)));
    }
}

void VCSpeedDialProperties::slotPresetInputValueChanged(quint32 universe, quint32 channel)
{
    VCSpeedDialPreset *preset = getSelectedPreset();

    if (preset != NULL)
    {
        preset->m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(universe, (m_dial->page() << 16) | channel));
        updatePresetInputSource(preset->m_inputSource);
    }
}

void VCSpeedDialProperties::slotChoosePresetInputClicked()
{
    VCSpeedDialPreset *preset = getSelectedPreset();

    if (preset != NULL)
    {
        SelectInputChannel sic(this, m_doc->inputOutputMap());
        if (sic.exec() == QDialog::Accepted)
        {
            preset->m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(sic.universe(), sic.channel()));
            updatePresetInputSource(preset->m_inputSource);
        }
    }
}

void VCSpeedDialProperties::slotAttachPresetKey()
{
    VCSpeedDialPreset *preset = getSelectedPreset();

    if (preset != NULL)
    {
        AssignHotKey ahk(this, preset->m_keySequence);
        if (ahk.exec() == QDialog::Accepted)
        {
            preset->m_keySequence = QKeySequence(ahk.keySequence());
            m_presetKeyEdit->setText(preset->m_keySequence.toString(QKeySequence::NativeText));
        }
    }
}

void VCSpeedDialProperties::slotDetachPresetKey()
{
    VCSpeedDialPreset *preset = getSelectedPreset();

    if (preset != NULL)
    {
        preset->m_keySequence = QKeySequence();
        m_presetKeyEdit->setText(preset->m_keySequence.toString(QKeySequence::NativeText));
    }
}
