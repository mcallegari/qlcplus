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

#include "inputselectionwidget.h"
#include "selectinputchannel.h"
#include "qlcinputchannel.h"
#include "assignhotkey.h"
#include "inputpatch.h"
#include "doc.h"

#include <QDebug>

InputSelectionWidget::InputSelectionWidget(Doc *doc, QWidget *parent)
    : QWidget(parent)
    , m_doc(doc)
    , m_widgetPage(0)
    , m_emitOdd(false)
    , m_signalsReceived(0)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_customFbButton->setVisible(false);
    m_feedbackGroup->setVisible(false);
    m_lowerSpin->setEnabled(false);
    m_upperSpin->setEnabled(false);

    connect(m_attachKey, SIGNAL(clicked()), this, SLOT(slotAttachKey()));
    connect(m_detachKey, SIGNAL(clicked()), this, SLOT(slotDetachKey()));

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseInputClicked()));

    connect(m_customFbButton, SIGNAL(toggled(bool)),
            this, SLOT(slotCustomFeedbackToggled(bool)));
    connect(m_lowerSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotLowerSpinValueChanged(int)));
    connect(m_upperSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpperSpinValueChanged(int)));
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

void InputSelectionWidget::slotCustomFeedbackToggled(bool checked)
{
    m_feedbackGroup->setVisible(checked);
}

void InputSelectionWidget::slotLowerSpinValueChanged(int value)
{
    m_inputSource->setRange(uchar(value), uchar(m_upperSpin->value()));
}

void InputSelectionWidget::slotUpperSpinValueChanged(int value)
{
    m_inputSource->setRange(uchar(m_lowerSpin->value()), uchar(value));
}

void InputSelectionWidget::updateInputSource()
{
    QString uniName;
    QString chName;

    if (!m_inputSource || m_doc->inputOutputMap()->inputSourceNames(m_inputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
        m_lowerSpin->setEnabled(false);
        m_upperSpin->setEnabled(false);
        m_customFbButton->setChecked(false);
        m_feedbackGroup->setVisible(false);
    }
    else
    {
        m_lowerSpin->blockSignals(true);
        m_upperSpin->blockSignals(true);

        uchar min = 0, max = UCHAR_MAX;

        InputPatch *ip = m_doc->inputOutputMap()->inputPatch(m_inputSource->universe());
        if (ip != NULL && ip->profile() != NULL)
        {
            QLCInputChannel *ich = ip->profile()->channel(m_inputSource->channel());
            if (ich != NULL && ich->type() == QLCInputChannel::Button)
            {
                min = ich->lowerValue();
                max = ich->upperValue();
            }
        }
        m_lowerSpin->setValue((m_inputSource->lowerValue() != 0) ? m_inputSource->lowerValue() : min);
        m_upperSpin->setValue((m_inputSource->upperValue() != UCHAR_MAX) ? m_inputSource->upperValue() : max);
        if (m_lowerSpin->value() != 0 || m_upperSpin->value() != UCHAR_MAX)
        {
            m_customFbButton->setChecked(true);
        }
        else
        {
            m_customFbButton->setChecked(false);
            m_feedbackGroup->setVisible(false);
        }
        m_lowerSpin->blockSignals(false);
        m_upperSpin->blockSignals(false);
        m_lowerSpin->setEnabled(true);
        m_upperSpin->setEnabled(true);
    }

    m_inputUniverseEdit->setText(uniName);
    m_inputChannelEdit->setText(chName);
}
