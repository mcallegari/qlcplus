/*
  Q Light Controller Plus
  inputselectionwidget.cpp

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

#include <QDebug>

#include "customfeedbackdialog.h"
#include "inputselectionwidget.h"
#include "selectinputchannel.h"
#include "qlcinputchannel.h"
#include "assignhotkey.h"
#include "inputpatch.h"
#include "doc.h"

InputSelectionWidget::InputSelectionWidget(Doc *doc, QWidget *parent)
    : QWidget(parent)
    , m_doc(doc)
    , m_widgetPage(0)
    , m_emitOdd(false)
    , m_supportMonitoring(false)
    , m_signalsReceived(0)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_customFbButton->setVisible(false);

    connect(m_attachKey, SIGNAL(clicked()), this, SLOT(slotAttachKey()));
    connect(m_detachKey, SIGNAL(clicked()), this, SLOT(slotDetachKey()));

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseInputClicked()));

    connect(m_customFbButton, SIGNAL(clicked(bool)),
            this, SLOT(slotCustomFeedbackClicked()));
}

InputSelectionWidget::~InputSelectionWidget()
{
}

void InputSelectionWidget::setKeyInputVisibility(bool visible)
{
    m_keyInputGroup->setVisible(visible);
}

void InputSelectionWidget::setCustomFeedbackVisibility(bool visible)
{
    m_customFbButton->setVisible(visible);
}

void InputSelectionWidget::setMonitoringSupport(bool enable)
{
    m_supportMonitoring = enable;
}

void InputSelectionWidget::setTitle(QString title)
{
    m_extInputGroup->setTitle(title);
}

void InputSelectionWidget::setWidgetPage(int page)
{
    m_widgetPage = page;
}

bool InputSelectionWidget::isAutoDetecting()
{
   return m_autoDetectInputButton->isChecked();
}

void InputSelectionWidget::stopAutoDetection()
{
    if (m_autoDetectInputButton->isChecked())
        m_autoDetectInputButton->toggle();
}

void InputSelectionWidget::emitOddValues(bool enable)
{
    m_emitOdd = enable;
}

void InputSelectionWidget::setKeySequence(const QKeySequence &keySequence)
{
    m_keySequence = QKeySequence(keySequence);
    m_keyEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
}

QKeySequence InputSelectionWidget::keySequence() const
{
    return m_keySequence;
}

void InputSelectionWidget::setInputSource(const QSharedPointer<QLCInputSource> &source)
{
    m_inputSource = source;
    updateInputSource();
}

QSharedPointer<QLCInputSource> InputSelectionWidget::inputSource() const
{
    return m_inputSource;
}

void InputSelectionWidget::slotAttachKey()
{
    AssignHotKey ahk(this, m_keySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        setKeySequence(QKeySequence(ahk.keySequence()));
        emit keySequenceChanged(m_keySequence);
    }
}

void InputSelectionWidget::slotDetachKey()
{
    setKeySequence(QKeySequence());
    emit keySequenceChanged(m_keySequence);
}

void InputSelectionWidget::slotAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(),
                SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(),
                   SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
    emit autoDetectToggled(checked);
}

void InputSelectionWidget::slotInputValueChanged(quint32 universe, quint32 channel)
{
    if (m_emitOdd == true && m_signalsReceived % 2)
    {
        emit inputValueChanged(universe, (m_widgetPage << 16) | channel);
        m_signalsReceived++;
        return;
    }

    m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(universe, (m_widgetPage << 16) | channel));
    updateInputSource();
    m_signalsReceived++;

    if (m_emitOdd == false)
        emit inputValueChanged(universe, (m_widgetPage << 16) | channel);
}

void InputSelectionWidget::slotChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(sic.universe(), (m_widgetPage << 16) | sic.channel()));
        updateInputSource();
        emit inputValueChanged(sic.universe(), (m_widgetPage << 16) | sic.channel());
    }
}

void InputSelectionWidget::slotCustomFeedbackClicked()
{
    CustomFeedbackDialog cfDialog(m_doc, m_inputSource, this);
    cfDialog.setMonitoringVisibility(m_supportMonitoring);
    cfDialog.exec();
}

void InputSelectionWidget::updateInputSource()
{
    QString uniName;
    QString chName;

    if (!m_inputSource || m_doc->inputOutputMap()->inputSourceNames(m_inputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_inputUniverseEdit->setText(uniName);
    m_inputChannelEdit->setText(chName);
}
