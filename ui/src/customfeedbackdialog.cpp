/*
  Q Light Controller Plus
  customfeedbackdialog.cpp

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

#include "customfeedbackdialog.h"
#include "qlcinputchannel.h"
#include "qlcinputsource.h"
#include "doc.h"

CustomFeedbackDialog::CustomFeedbackDialog(Doc *doc, const QSharedPointer<QLCInputSource> &source, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
    , m_profile(NULL)
    , m_inputSource(source)
    , m_selectedFeedback(None)
{
    setupUi(this);

    bool enableControls = source.isNull() ? false : true;

    if (enableControls)
    {
        m_lowerSpin->setValue(m_inputSource->feedbackValue(QLCInputFeedback::LowerValue));
        m_upperSpin->setValue(m_inputSource->feedbackValue(QLCInputFeedback::UpperValue));
        m_monitorSpin->setValue(m_inputSource->feedbackValue(QLCInputFeedback::MonitorValue));
    }

    m_lowerSpin->setEnabled(enableControls);
    m_upperSpin->setEnabled(enableControls);

    m_monitorLabel->setVisible(false);
    m_monitorSpin->setVisible(false);
    m_monitorChannelCombo->setVisible(false);
    m_profileColorsTree->setVisible(false);
    m_midiChannelGroup->hide();

    if (enableControls)
    {
        InputPatch *ip = m_doc->inputOutputMap()->inputPatch(m_inputSource->universe());
        if (ip != NULL && ip->profile() != NULL)
        {
            m_profile = ip->profile();
            if (m_profile->hasColorTable())
            {
                m_lowerColor->setVisible(true);
                m_upperColor->setVisible(true);

                QMapIterator <uchar, QPair<QString, QColor>> it(m_profile->colorTable());
                while (it.hasNext() == true)
                {
                    it.next();
                    QPair<QString, QColor> lc = it.value();
                    QTreeWidgetItem *item = new QTreeWidgetItem(m_profileColorsTree);
                    item->setText(0, QString::number(it.key()));
                    item->setText(1, lc.first);

                    QLabel *colLabel = new QLabel();
                    colLabel->setStyleSheet(QString("background-color: %1").arg(lc.second.name()));

                    if (it.key() == m_inputSource->feedbackValue(QLCInputFeedback::LowerValue))
                        m_lowerColor->setStyleSheet(QString("background-color: %1").arg(lc.second.name()));

                    if (it.key() == m_inputSource->feedbackValue(QLCInputFeedback::UpperValue))
                        m_upperColor->setStyleSheet(QString("background-color: %1").arg(lc.second.name()));

                    if (it.key() == m_inputSource->feedbackValue(QLCInputFeedback::MonitorValue))
                        m_monitorColor->setStyleSheet(QString("background-color: %1").arg(lc.second.name()));

                    m_profileColorsTree->setItemWidget(item, 2, colLabel);
                }
            }
            if (m_profile->type() == QLCInputProfile::MIDI && m_profile->hasMidiChannelTable())
            {
                m_midiChannelGroup->show();
                m_lowerChannelCombo->addItem(tr("From plugin settings"));
                m_upperChannelCombo->addItem(tr("From plugin settings"));
                m_monitorChannelCombo->addItem(tr("From plugin settings"));

                QMapIterator <uchar, QString> it(m_profile->midiChannelTable());
                while (it.hasNext() == true)
                {
                    it.next();
                    m_lowerChannelCombo->addItem(it.value());
                    m_upperChannelCombo->addItem(it.value());
                    m_monitorChannelCombo->addItem(it.value());
                }

                QVariant extraParams = m_inputSource->feedbackExtraParams(QLCInputFeedback::LowerValue);
                if (extraParams.isValid())
                    m_lowerChannelCombo->setCurrentIndex(extraParams.toInt() + 1);

                extraParams = m_inputSource->feedbackExtraParams(QLCInputFeedback::UpperValue);
                if (extraParams.isValid())
                    m_upperChannelCombo->setCurrentIndex(extraParams.toInt() + 1);

                extraParams = m_inputSource->feedbackExtraParams(QLCInputFeedback::MonitorValue);
                if (extraParams.isValid())
                    m_monitorChannelCombo->setCurrentIndex(extraParams.toInt() + 1);
            }
        }
    }

    // connect signals
    connect(m_lowerColor, SIGNAL(clicked()),
            this, SLOT(slotLowerColorButtonClicked()));
    connect(m_upperColor, SIGNAL(clicked()),
            this, SLOT(slotUpperColorButtonClicked()));
    connect(m_monitorColor, SIGNAL(clicked()),
            this, SLOT(slotMonitorColorButtonClicked()));
    connect(m_profileColorsTree, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(slotColorSelected(QTreeWidgetItem *)));
}

CustomFeedbackDialog::~CustomFeedbackDialog()
{
}

void CustomFeedbackDialog::setMonitoringVisibility(bool visible)
{
    m_monitorLabel->setVisible(visible);
    m_monitorSpin->setVisible(visible);
    m_monitorColor->setVisible(visible);
    m_monitorChannelCombo->setVisible(visible);
}

void CustomFeedbackDialog::accept()
{
    if (m_inputSource.isNull())
        return;

    m_inputSource->setFeedbackValue(QLCInputFeedback::LowerValue, m_lowerSpin->value());
    m_inputSource->setFeedbackValue(QLCInputFeedback::UpperValue, m_upperSpin->value());
    if (m_monitorSpin->isVisible())
        m_inputSource->setFeedbackValue(QLCInputFeedback::MonitorValue, m_monitorSpin->value());

    if (m_midiChannelGroup->isVisible())
    {
        m_inputSource->setFeedbackExtraParams(QLCInputFeedback::LowerValue, m_lowerChannelCombo->currentIndex() - 1);
        m_inputSource->setFeedbackExtraParams(QLCInputFeedback::UpperValue, m_upperChannelCombo->currentIndex() - 1);
        if (m_monitorSpin->isVisible())
            m_inputSource->setFeedbackExtraParams(QLCInputFeedback::MonitorValue, m_monitorChannelCombo->currentIndex() - 1);
    }

    QDialog::accept();
}

void CustomFeedbackDialog::slotLowerColorButtonClicked()
{
    m_selectedFeedback = LowerValue;
    m_profileColorsTree->setVisible(true);
}

void CustomFeedbackDialog::slotUpperColorButtonClicked()
{
    m_selectedFeedback = UpperValue;
    m_profileColorsTree->setVisible(true);
}

void CustomFeedbackDialog::slotMonitorColorButtonClicked()
{
    m_selectedFeedback = MonitoringValue;
    m_profileColorsTree->setVisible(true);
}

void CustomFeedbackDialog::slotColorSelected(QTreeWidgetItem *item)
{
    QLabel *label = qobject_cast<QLabel *>(m_profileColorsTree->itemWidget(item, 2));

    if (m_selectedFeedback == LowerValue)
    {
        m_lowerSpin->setValue(item->text(0).toInt());
        m_lowerColor->setStyleSheet(label->styleSheet());
    }
    else if (m_selectedFeedback == UpperValue)
    {
        m_upperSpin->setValue(item->text(0).toInt());
        m_upperColor->setStyleSheet(label->styleSheet());
    }
    else if (m_selectedFeedback == MonitoringValue)
    {
        m_monitorSpin->setValue(item->text(0).toInt());
        m_monitorColor->setStyleSheet(label->styleSheet());
    }
    m_profileColorsTree->setVisible(false);
    m_selectedFeedback = None;
}
