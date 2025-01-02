/*
  Q Light Controller Plus
  vcmatrixproperties.cpp

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

#include <QColorDialog>
#include <QInputDialog>
#include <QTreeWidget>

#include "vcmatrixpresetselection.h"
#include "inputselectionwidget.h"
#include "vcmatrixproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "inputpatch.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
 #include "rgbscript.h"
#else
 #include "rgbscriptv4.h"
#endif

VCMatrixProperties::VCMatrixProperties(VCMatrix* matrix, Doc* doc)
    : QDialog(matrix)
    , m_doc(doc)
{
    Q_ASSERT(matrix != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_lastAssignedID = 0;

    /* Matrix text and function */
    m_matrix = matrix;
    m_nameEdit->setText(m_matrix->caption());
    slotSetFunction(m_matrix->function());

    if (m_matrix->instantChanges())
        m_instantCheck->setChecked(true);

    /* Matrix connections */
    connect(m_attachFunction, SIGNAL(clicked()), this, SLOT(slotAttachFunction()));
    connect(m_detachFunction, SIGNAL(clicked()), this, SLOT(slotSetFunction()));

    /* Slider external input */
    m_sliderInputSource = m_matrix->inputSource();
    updateSliderInputSource();

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectSliderInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseSliderInputClicked()));

    /* Visibility */
    quint32 visibilityMask = m_matrix->visibilityMask();
    if (visibilityMask & VCMatrix::ShowSlider) m_sliderCheck->setChecked(true);
    if (visibilityMask & VCMatrix::ShowLabel) m_labelCheck->setChecked(true);
    if (visibilityMask & VCMatrix::ShowColor1Button) m_mtxColor1ButtonCheck->setChecked(true);
    if (visibilityMask & VCMatrix::ShowColor2Button) m_mtxColor2ButtonCheck->setChecked(true);
    if (visibilityMask & VCMatrix::ShowColor3Button) m_mtxColor3ButtonCheck->setChecked(true);
    if (visibilityMask & VCMatrix::ShowColor4Button) m_mtxColor4ButtonCheck->setChecked(true);
    if (visibilityMask & VCMatrix::ShowColor5Button) m_mtxColor5ButtonCheck->setChecked(true);
    if (visibilityMask & VCMatrix::ShowPresetCombo) m_presetComboCheck->setChecked(true);

    /* Custom controls */
    foreach (const VCMatrixControl *control, m_matrix->customControls())
    {
        m_controls.append(new VCMatrixControl(*control));
        if (control->m_id > m_lastAssignedID)
            m_lastAssignedID = control->m_id;
    }

    m_controlsTree->setSelectionMode(QAbstractItemView::SingleSelection);

    updateTree();
    slotColorComboActivated();

    connect(m_controlsTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotTreeSelectionChanged()));

    connect(m_colorsCombo, SIGNAL(activated(int)),
            this, SLOT(slotColorComboActivated()));
    connect(m_addColorButton, SIGNAL(clicked()),
            this, SLOT(slotAddColorClicked()));
    connect(m_addColorKnobsButton, SIGNAL(clicked()),
            this, SLOT(slotAddColorKnobsClicked()));
    connect(m_addColorResetButton, SIGNAL(clicked()),
            this, SLOT(slotAddColorResetClicked()));

    connect(m_addPresetButton, SIGNAL(clicked()),
            this, SLOT(slotAddAnimationClicked()));
    connect(m_addTextButton, SIGNAL(clicked()),
            this, SLOT(slotAddTextClicked()));

    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveClicked()));

    m_presetInputWidget = new InputSelectionWidget(m_doc, this);
    m_presetInputWidget->setCustomFeedbackVisibility(true);
    m_presetInputWidget->setWidgetPage(m_matrix->page());
    m_presetInputWidget->show();
    m_presetInputLayout->addWidget(m_presetInputWidget);

    connect(m_presetInputWidget, SIGNAL(inputValueChanged(quint32,quint32)),
            this, SLOT(slotInputValueChanged(quint32,quint32)));
    connect(m_presetInputWidget, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(slotKeySequenceChanged(QKeySequence)));
}

VCMatrixProperties::~VCMatrixProperties()
{
    foreach (VCMatrixControl* control, m_controls)
        delete control;

    delete m_presetInputWidget;
}

/*********************************************************************
 * RGB Matrix attachment
 *********************************************************************/

void VCMatrixProperties::slotAttachFunction()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(false);
    fs.setFilter(Function::RGBMatrixType);
    fs.disableFilters(Function::SceneType | Function::ChaserType | Function::EFXType |
                      Function::ShowType | Function::ScriptType | Function::CollectionType |
                      Function::AudioType | Function::VideoType);

    if (fs.exec() == QDialog::Accepted && fs.selection().size() > 0)
        slotSetFunction(fs.selection().first());
}

void VCMatrixProperties::slotSetFunction(quint32 fid)
{
    m_function = fid;
    Function* func = m_doc->function(m_function);

    if (func == NULL)
    {
        m_functionEdit->setText(tr("No function"));
    }
    else
    {
        m_functionEdit->setText(func->name());
        if (m_nameEdit->text().simplified().contains(QString::number(m_matrix->id())))
            m_nameEdit->setText(func->name());
    }
}

/*********************************************************************
 * Slider External input
 *********************************************************************/

void VCMatrixProperties::slotAutoDetectSliderInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotSliderInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotSliderInputValueChanged(quint32,quint32)));
    }
}

void VCMatrixProperties::slotSliderInputValueChanged(quint32 universe, quint32 channel)
{
    m_sliderInputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(universe, (m_matrix->page() << 16) | channel));
    updateSliderInputSource();
}

void VCMatrixProperties::slotChooseSliderInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_sliderInputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(sic.universe(), sic.channel()));
        updateSliderInputSource();
    }
}

void VCMatrixProperties::updateSliderInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_sliderInputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_inputUniverseEdit->setText(uniName);
    m_inputChannelEdit->setText(chName);
}

/*********************************************************************
 * Custom controls
 *********************************************************************/

void VCMatrixProperties::updateTree()
{
    m_controlsTree->blockSignals(true);
    m_controlsTree->clear();
    foreach (VCMatrixControl *control, m_controls)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_controlsTree);
        item->setData(0, Qt::UserRole, control->m_id);

        switch(control->m_type)
        {
            case VCMatrixControl::Color1:
                item->setIcon(0, QIcon(":/color.png"));
                item->setText(0, tr("Color 1"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color1Knob:
                item->setIcon(0, QIcon(":/knob.png"));
                item->setText(0, tr("Color 1 Knob"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color1Reset:
            break;
            case VCMatrixControl::Color2:
                item->setIcon(0, QIcon(":/color.png"));
                item->setText(0, tr("Color 2"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color2Knob:
                item->setIcon(0, QIcon(":/knob.png"));
                item->setText(0, tr("Color 2 Knob"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color2Reset:
                item->setIcon(0, QIcon(":/fileclose.png"));
                item->setText(0, tr("Color 2 Reset"));
            break;
            case VCMatrixControl::Color3:
                item->setIcon(0, QIcon(":/color.png"));
                item->setText(0, tr("Color 3"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color3Knob:
                item->setIcon(0, QIcon(":/knob.png"));
                item->setText(0, tr("Color 3 Knob"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color3Reset:
                item->setIcon(0, QIcon(":/fileclose.png"));
                item->setText(0, tr("Color 3 Reset"));
            break;
            case VCMatrixControl::Color4:
                item->setIcon(0, QIcon(":/color.png"));
                item->setText(0, tr("Color 4"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color4Knob:
                item->setIcon(0, QIcon(":/knob.png"));
                item->setText(0, tr("Color 4 Knob"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color4Reset:
                item->setIcon(0, QIcon(":/fileclose.png"));
                item->setText(0, tr("Color 4 Reset"));
            break;
            case VCMatrixControl::Color5:
                item->setIcon(0, QIcon(":/color.png"));
                item->setText(0, tr("Color 5"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color5Knob:
                item->setIcon(0, QIcon(":/knob.png"));
                item->setText(0, tr("Color 5 Knob"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::Color5Reset:
                item->setIcon(0, QIcon(":/fileclose.png"));
                item->setText(0, tr("Color 5 Reset"));
            break;
            case VCMatrixControl::Animation:
            {
                item->setIcon(0, QIcon(":/script.png"));
                item->setText(0, tr("Animation"));
                QString presetName = control->m_resource;
                if (!control->m_properties.isEmpty())
                {
                    presetName += " (";
                    QHashIterator<QString, QString> it(control->m_properties);
                    while (it.hasNext())
                    {
                        it.next();
                        presetName += it.value();
                        if (it.hasNext())
                            presetName += ",";
                    }
                    presetName += ")";
                }
                item->setText(1, presetName);
            }
            break;
            case VCMatrixControl::Image:
            break;
            case VCMatrixControl::Text:
                item->setIcon(0, QIcon(":/fonts.png"));
                item->setText(0, tr("Text"));
                item->setText(1, control->m_resource);
            break;
        }
    }
    m_controlsTree->resizeColumnToContents(0);
    m_controlsTree->blockSignals(false);
}

VCMatrixControl *VCMatrixProperties::getSelectedControl()
{
    if (m_controlsTree->selectedItems().isEmpty())
        return NULL;

    QTreeWidgetItem * item = m_controlsTree->selectedItems().first();
    if (item != NULL)
    {
        quint8 ctlID = item->data(0, Qt::UserRole).toUInt();
        foreach (VCMatrixControl *control, m_controls)
        {
            if (control->m_id == ctlID)
                return control;
        }
    }

    Q_ASSERT(false);
    return NULL;
}

QList<QColor> VCMatrixProperties::rgbColorList()
{
    QList<QColor> colors;
    colors.append(QColor(Qt::red));
    colors.append(QColor(Qt::green));
    colors.append(QColor(Qt::blue));
    return colors;
}

void VCMatrixProperties::addControl(VCMatrixControl *control)
{
    m_controls.append(control);
}

void VCMatrixProperties::removeControl(quint8 id)
{
    for (int i = 0; i < m_controls.count(); i++)
    {
        if (m_controls.at(i)->m_id == id)
        {
            m_controls.removeAt(i);
            return;
        }
    }
}

void VCMatrixProperties::slotAddColorClicked()
{
    QColor col = QColorDialog::getColor();
    if (col.isValid() == true)
    {
        VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
        newControl->m_type = VCMatrixControl::ControlType(int(VCMatrixControl::Color1) + m_colorsCombo->currentIndex());
        newControl->m_color = col;
        addControl(newControl);
        updateTree();
    }
}

void VCMatrixProperties::slotAddColorKnobsClicked()
{
    foreach (QColor col, VCMatrixProperties::rgbColorList())
    {
        VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
        newControl->m_type = VCMatrixControl::ControlType(int(VCMatrixControl::Color1Knob) + m_colorsCombo->currentIndex());
        newControl->m_color = col;
        addControl(newControl);
    }
    updateTree();
}

void VCMatrixProperties::slotAddColorResetClicked()
{
    VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
    newControl->m_type = VCMatrixControl::ControlType(int(VCMatrixControl::Color1Reset) + m_colorsCombo->currentIndex());
    addControl(newControl);
    updateTree();
}

void VCMatrixProperties::slotAddAnimationClicked()
{
    VCMatrixPresetSelection ps(m_doc, this);

    if (ps.exec() == QDialog::Accepted)
    {
        VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
        newControl->m_type = VCMatrixControl::Animation;
        newControl->m_resource = ps.selectedPreset();
        newControl->m_properties = ps.customizedProperties();
        addControl(newControl);
        updateTree();
    }
}

void VCMatrixProperties::slotAddTextClicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Enter a text"),
                                      tr("Text"), QLineEdit::Normal,
                                      "Q Light Controller+", &ok);
    if (ok && !text.isEmpty())
    {
        VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
        newControl->m_type = VCMatrixControl::Text;
        newControl->m_resource = text;
        addControl(newControl);
        updateTree();
    }
}

void VCMatrixProperties::slotRemoveClicked()
{
    if (m_controlsTree->selectedItems().isEmpty())
        return;
    QTreeWidgetItem *selItem = m_controlsTree->selectedItems().first();
    quint8 ctlID = selItem->data(0, Qt::UserRole).toUInt();

    {
        // For R/G/B Knobs:
        // Remove the two others
        VCMatrixControl *control = getSelectedControl();
        if (control != NULL)
        {
            if (control->m_type == VCMatrixControl::Color1Knob
                    || control->m_type == VCMatrixControl::Color2Knob
                    || control->m_type == VCMatrixControl::Color3Knob
                    || control->m_type == VCMatrixControl::Color4Knob
                    || control->m_type == VCMatrixControl::Color5Knob)
            {
                if (control->m_color == Qt::red)
                {
                    removeControl(ctlID + 1);
                    removeControl(ctlID + 2);
                }
                else if (control->m_color == Qt::green)
                {
                    removeControl(ctlID - 1);
                    removeControl(ctlID + 1);
                }
                else if (control->m_color == Qt::blue)
                {
                    removeControl(ctlID - 2);
                    removeControl(ctlID - 1);
                }
                else
                {
                    Q_ASSERT(false);
                }
            }
        }
    }

    removeControl(ctlID);
    updateTree();
}

void VCMatrixProperties::slotInputValueChanged(quint32 universe, quint32 channel)
{
    Q_UNUSED(universe);
    Q_UNUSED(channel);

    VCMatrixControl *preset = getSelectedControl();

    if (preset != NULL) {
        preset->m_inputSource = m_presetInputWidget->inputSource();
    }
}

void VCMatrixProperties::slotKeySequenceChanged(QKeySequence key)
{
    VCMatrixControl *preset = getSelectedControl();

    if (preset != NULL)
        preset->m_keySequence = key;
}


void VCMatrixProperties::slotTreeSelectionChanged()
{
    VCMatrixControl *control = getSelectedControl();

    if (control != NULL)
    {
        m_presetInputWidget->setInputSource(control->m_inputSource);
        m_presetInputWidget->setKeySequence(control->m_keySequence.toString(QKeySequence::NativeText));
        if (control->widgetType() == VCMatrixControl::Button)
        {
            m_presetInputWidget->setCustomFeedbackVisibility(true);
            m_presetInputWidget->setKeyInputVisibility(true);
        }
        else
        {
            m_presetInputWidget->setCustomFeedbackVisibility(false);
            m_presetInputWidget->setKeyInputVisibility(false);
        }
    }
}

void VCMatrixProperties::slotColorComboActivated()
{
    if (m_colorsCombo->currentIndex() == 0)
        m_addColorResetButton->setEnabled(false);
    else
        m_addColorResetButton->setEnabled(true);
}

void VCMatrixProperties::accept()
{
    m_matrix->setCaption(m_nameEdit->text());
    m_matrix->setFunction(m_function);
    if (m_instantCheck->isChecked())
        m_matrix->setInstantChanges(true);
    else
        m_matrix->setInstantChanges(false);

    /* External input */
    m_matrix->setInputSource(m_sliderInputSource);

    /* Visibility */
    quint32 visibilityMask = 0;
    if (m_sliderCheck->isChecked()) visibilityMask |= VCMatrix::ShowSlider;
    if (m_labelCheck->isChecked()) visibilityMask |= VCMatrix::ShowLabel;
    if (m_mtxColor1ButtonCheck->isChecked()) visibilityMask |= VCMatrix::ShowColor1Button;
    if (m_mtxColor2ButtonCheck->isChecked()) visibilityMask |= VCMatrix::ShowColor2Button;
    if (m_mtxColor3ButtonCheck->isChecked()) visibilityMask |= VCMatrix::ShowColor3Button;
    if (m_mtxColor4ButtonCheck->isChecked()) visibilityMask |= VCMatrix::ShowColor4Button;
    if (m_mtxColor5ButtonCheck->isChecked()) visibilityMask |= VCMatrix::ShowColor5Button;
    if (m_presetComboCheck->isChecked()) visibilityMask |= VCMatrix::ShowPresetCombo;
    m_matrix->setVisibilityMask(visibilityMask);

    /* Controls */
    m_matrix->resetCustomControls();
    for (int i = 0; i < m_controls.count(); i++)
        m_matrix->addCustomControl(*m_controls.at(i));

    /* Close dialog */
    QDialog::accept();
}
