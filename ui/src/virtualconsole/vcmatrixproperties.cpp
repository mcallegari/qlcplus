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
#include "vcmatrixproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "assignhotkey.h"
#include "inputpatch.h"
#include "rgbscript.h"

static bool compareControlsID(const VCMatrixControl *ctl1, const VCMatrixControl *ctl2)
{
    if (ctl1->m_id < ctl2->m_id)
        return true;
    return false;
}

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

    /* Custom controls */
    foreach(const VCMatrixControl *control, m_matrix->customControls())
    {
        m_controls.append(new VCMatrixControl(*control));
        if (control->m_id > m_lastAssignedID)
            m_lastAssignedID = control->m_id;
    }
    qSort(m_controls.begin(), m_controls.end(), compareControlsID);

    m_controlsTree->setSelectionMode(QAbstractItemView::SingleSelection);

    updateTree();

    connect(m_controlsTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotTreeSelectionChanged()));

    connect(m_addStartColorKnobButton, SIGNAL(clicked()),
            this, SLOT(slotAddStartColorKnobClicked()));
    connect(m_addEndColorKnobButton, SIGNAL(clicked()),
            this, SLOT(slotAddEndColorKnobClicked()));
    connect(m_addStartColorButton, SIGNAL(clicked()),
            this, SLOT(slotAddStartColorClicked()));
    connect(m_addEndColorButton, SIGNAL(clicked()),
            this, SLOT(slotAddEndColorClicked()));
    connect(m_addEndColorResetButton, SIGNAL(clicked()),
            this, SLOT(slotAddEndColorResetClicked()));
    connect(m_addPresetButton, SIGNAL(clicked()),
            this, SLOT(slotAddAnimationClicked()));
    connect(m_addTextButton, SIGNAL(clicked()),
            this, SLOT(slotAddTextClicked()));

    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveClicked()));

    connect(m_adControlInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectControlInputToggled(bool)));
    connect(m_chooseControlInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseControlInputClicked()));

    connect(m_attachKey, SIGNAL(clicked()), this, SLOT(slotAttachKey()));
    connect(m_detachKey, SIGNAL(clicked()), this, SLOT(slotDetachKey()));
}

VCMatrixProperties::~VCMatrixProperties()
{
    foreach (VCMatrixControl* control, m_controls)
    {
        delete control;
    }
}

/*********************************************************************
 * RGB Matrix attachment
 *********************************************************************/

void VCMatrixProperties::slotAttachFunction()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(false);
    fs.setFilter(Function::RGBMatrix);
    fs.disableFilters(Function::Scene | Function::Chaser | Function::EFX | Function::Show |
                      Function::Script | Function::Collection | Function::Audio);
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
    if (m_sliderInputSource != NULL)
        delete m_sliderInputSource;
    m_sliderInputSource = new QLCInputSource(universe, (m_matrix->page() << 16) | channel);
    updateSliderInputSource();
}

void VCMatrixProperties::slotChooseSliderInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_sliderInputSource != NULL)
            delete m_sliderInputSource;
        m_sliderInputSource = new QLCInputSource(sic.universe(), sic.channel());
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

void VCMatrixProperties::updateTree()
{
    m_controlsTree->blockSignals(true);
    m_controlsTree->clear();
    foreach(VCMatrixControl *control, m_controls)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_controlsTree);
        item->setData(0, Qt::UserRole, control->m_id);

        switch(control->m_type)
        {
            case VCMatrixControl::StartColor:
            case VCMatrixControl::StartColorKnob:
                item->setIcon(0, QIcon(":/color.png"));
                item->setText(0, tr("Start Color"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::EndColor:
            case VCMatrixControl::EndColorKnob:
                item->setIcon(0, QIcon(":/color.png"));
                item->setText(0, tr("End Color"));
                item->setText(1, control->m_color.name());
                item->setBackground(1, QBrush(control->m_color));
            break;
            case VCMatrixControl::ResetEndColor:
                item->setIcon(0, QIcon(":/fileclose.png"));
                item->setText(0, tr("End Color Reset"));
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
                    while(it.hasNext())
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
        foreach(VCMatrixControl *control, m_controls)
        {
            if (control->m_id == ctlID)
                return control;
        }
    }
    return NULL;
}

/*********************************************************************
 * Custom controls
 *********************************************************************/

void VCMatrixProperties::addControl(VCMatrixControl *control)
{
    m_controls.append(control);
}

void VCMatrixProperties::removeControl(quint8 id)
{
    for(int i = 0; i < m_controls.count(); i++)
    {
        if (m_controls.at(i)->m_id == id)
        {
            m_controls.removeAt(i);
            return;
        }
    }
}

void VCMatrixProperties::slotAddStartColorKnobClicked()
{
    QColor col(Qt::red);
    while ((col = QColorDialog::getColor()).isValid())
    {
        if (col == Qt::red || col == Qt::green || col == Qt::blue)
        {
            VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
            newControl->m_type = VCMatrixControl::StartColorKnob;
            newControl->m_color = col;
            addControl(newControl);
            updateTree();
            return;
        }
    }
}

void VCMatrixProperties::slotAddEndColorKnobClicked()
{
    QColor col(Qt::red);
    while ((col = QColorDialog::getColor()).isValid())
    {
        if (col == Qt::red || col == Qt::green || col == Qt::blue)
        {
            VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
            newControl->m_type = VCMatrixControl::EndColorKnob;
            newControl->m_color = col;
            addControl(newControl);
            updateTree();
            return;
        }
    }
}

void VCMatrixProperties::slotAddStartColorClicked()
{
    QColor col = QColorDialog::getColor();
    if (col.isValid() == true)
    {
        VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
        newControl->m_type = VCMatrixControl::StartColor;
        newControl->m_color = col;
        addControl(newControl);
        updateTree();
    }
}

void VCMatrixProperties::slotAddEndColorClicked()
{
    QColor col = QColorDialog::getColor();
    if (col.isValid() == true)
    {
        VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
        newControl->m_type = VCMatrixControl::EndColor;
        newControl->m_color = col;
        addControl(newControl);
        updateTree();
    }
}

void VCMatrixProperties::slotAddEndColorResetClicked()
{
    VCMatrixControl *newControl = new VCMatrixControl(++m_lastAssignedID);
    newControl->m_type = VCMatrixControl::ResetEndColor;
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
    removeControl(ctlID);
    updateTree();
}

void VCMatrixProperties::updateControlInputSource(QLCInputSource *source)
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(source, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_controlInputUniverseEdit->setText(uniName);
    m_controlInputChannelEdit->setText(chName);
}

void VCMatrixProperties::slotTreeSelectionChanged()
{
    VCMatrixControl *control = getSelectedControl();

    if (control != NULL)
    {
        updateControlInputSource(control->m_inputSource);
        m_keyEdit->setText(control->m_keySequence.toString(QKeySequence::NativeText));
    }
}

void VCMatrixProperties::slotAutoDetectControlInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotControlInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotControlInputValueChanged(quint32,quint32)));
    }
}

void VCMatrixProperties::slotControlInputValueChanged(quint32 universe, quint32 channel)
{
    VCMatrixControl *control = getSelectedControl();

    if (control != NULL)
    {
        if (control->m_inputSource != NULL)
            delete control->m_inputSource;
        control->m_inputSource = new QLCInputSource(universe, (m_matrix->page() << 16) | channel);
        updateControlInputSource(control->m_inputSource);
    }
}

void VCMatrixProperties::slotChooseControlInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        VCMatrixControl *control = getSelectedControl();

        if (control != NULL)
        {
            if (control->m_inputSource != NULL)
                delete control->m_inputSource;
            control->m_inputSource = new QLCInputSource(sic.universe(), sic.channel());
            updateControlInputSource(control->m_inputSource);
        }
    }
}

void VCMatrixProperties::slotAttachKey()
{
    VCMatrixControl *control = getSelectedControl();

    if (control != NULL)
    {
        AssignHotKey ahk(this, control->m_keySequence);
        if (ahk.exec() == QDialog::Accepted)
        {
            control->m_keySequence = QKeySequence(ahk.keySequence());
            m_keyEdit->setText(control->m_keySequence.toString(QKeySequence::NativeText));
        }
    }
}

void VCMatrixProperties::slotDetachKey()
{
    VCMatrixControl *control = getSelectedControl();

    if (control != NULL)
    {
        control->m_keySequence = QKeySequence();
        m_keyEdit->setText(control->m_keySequence.toString(QKeySequence::NativeText));
    }
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

    m_matrix->resetCustomControls();
    for (int i = 0; i < m_controls.count(); i++)
        m_matrix->addCustomControl(*m_controls.at(i));

    /* Close dialog */
    QDialog::accept();
}
