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

#include "vcmatrixproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "inputpatch.h"

VCMatrixProperties::VCMatrixProperties(VCMatrix* matrix, Doc* doc)
    : QDialog(matrix)
    , m_doc(doc)
{
    Q_ASSERT(matrix != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    /* Matrix text and function */
    m_matrix = matrix;
    m_nameEdit->setText(m_matrix->caption());
    slotSetFunction(m_matrix->function());

    if (m_matrix->instantChanges())
        m_instantCheck->setChecked(true);

    /* Matrix connections */
    connect(m_attachFunction, SIGNAL(clicked()), this, SLOT(slotAttachFunction()));
    connect(m_detachFunction, SIGNAL(clicked()), this, SLOT(slotSetFunction()));

    /********************************************************************
     * External input
     ********************************************************************/
    m_inputSource = m_matrix->inputSource();
    updateInputSource();

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseInputClicked()));
}

VCMatrixProperties::~VCMatrixProperties()
{
}

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

void VCMatrixProperties::slotAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
}

void VCMatrixProperties::slotInputValueChanged(quint32 universe, quint32 channel)
{
    if (m_inputSource != NULL)
        delete m_inputSource;
    m_inputSource = new QLCInputSource(universe, (m_matrix->page() << 16) | channel);
    updateInputSource();
}

void VCMatrixProperties::slotChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_inputSource != NULL)
            delete m_inputSource;
        m_inputSource = new QLCInputSource(sic.universe(), sic.channel());
        updateInputSource();
    }
}

void VCMatrixProperties::updateInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_inputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_inputUniverseEdit->setText(uniName);
    m_inputChannelEdit->setText(chName);
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
    m_matrix->setInputSource(m_inputSource);

    /* Close dialog */
    QDialog::accept();
}


