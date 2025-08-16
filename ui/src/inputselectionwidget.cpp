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
#include <qmath.h>

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

    connect(m_syncColour, SIGNAL(clicked(bool)),
            this, SLOT(slotSyncStatusChanged()));
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

void InputSelectionWidget::updateFeedback()
{
    if (m_inputSource.isNull())
        return;

    InputPatch *ip = m_doc->inputOutputMap()->inputPatch(m_inputSource->universe());
    if (ip != NULL && ip->profile() != NULL)
    {
        QLCInputProfile *m_profile = ip->profile();

        if (m_profile->hasColorTable())
        {
            QMapIterator <uchar, QPair<QString, QColor>> it(m_profile->colorTable());

            int currentDistanceWithRed = 255 * 255 * 3;
            int currentDistanceWithGreen = 255 * 255 * 3;
            int currentDistanceWithYellow = 255 * 255 * 3;

            uchar redValue = 0;
            uchar greenValue = 0;
            uchar yellowValue = 0;
            while (it.hasNext() == true)
            {
                it.next();
                QPair<QString, QColor> lc = it.value();
                QColor colorValue = lc.second;
                int distanceWithRed = getColorDistance(colorValue, QColor(Qt::red));
                int distanceWithGreen = getColorDistance(colorValue, QColor(Qt::green));
                int distanceWithYellow = getColorDistance(colorValue, QColor(Qt::yellow));
                if (distanceWithRed < currentDistanceWithRed)
                {
                    currentDistanceWithRed = distanceWithRed;
                    redValue = it.key();
                }
                if (distanceWithGreen < currentDistanceWithGreen)
                {
                    currentDistanceWithGreen = distanceWithGreen;
                    greenValue = it.key();
                }
                if (distanceWithYellow < currentDistanceWithYellow)
                {
                    currentDistanceWithYellow = distanceWithYellow;
                    yellowValue = it.key();
                }
            }
            m_inputSource->setFeedbackValue(QLCInputFeedback::LowerValue, redValue);
            m_inputSource->setFeedbackValue(QLCInputFeedback::UpperValue, greenValue);
            m_inputSource->setFeedbackValue(QLCInputFeedback::MonitorValue, yellowValue);
        }

        QLCInputChannel *m_channel = m_profile->channel(m_inputSource->channel());
        if (isSyncColor() && m_channel != NULL)
        {
            if (m_channel->lowerValue() > 0)
            {
                if (m_channel->lowerValue() == UCHAR_MAX)
                    m_inputSource->setFeedbackValue(QLCInputFeedback::LowerValue, UCHAR_MAX);
                else
                    m_inputSource->setFeedbackValue(QLCInputFeedback::LowerValue, (m_channel->lowerValue() + 1) / 2 * 2);
            }

            if (m_channel->upperValue() > 0)
            {
                if (m_channel->upperValue() == UCHAR_MAX)
                    m_inputSource->setFeedbackValue(QLCInputFeedback::UpperValue, UCHAR_MAX);
                else
                    m_inputSource->setFeedbackValue(QLCInputFeedback::UpperValue, (m_channel->upperValue() + 1) / 2 * 2);
            }
        }
    }
}

int InputSelectionWidget::getColorDistance(const QColor &color1, const QColor &color2)
{
    return qSqrt(qPow(color1.red() - color2.red(), 2) +
                 qPow(color1.green() - color2.green(), 2) +
                 qPow(color1.blue() - color2.blue(), 2));
}

void InputSelectionWidget::slotSyncStatusChanged()
{
    updateFeedback();
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
    cfDialog.setSyncStatus(m_syncColour->isChecked());
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
    updateFeedback();
}

bool InputSelectionWidget::isSyncColor()
{
    return m_syncColour->isChecked();
}

void InputSelectionWidget::setSyncStatus(bool enable)
{
    m_syncColour->setChecked(enable);
}
