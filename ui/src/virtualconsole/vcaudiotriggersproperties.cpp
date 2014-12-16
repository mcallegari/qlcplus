/*
  Q Light Controller Plus
  audiotriggersconfiguration.cpp

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

#include <QTreeWidgetItem>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>


#include "vcaudiotriggersproperties.h"
#include "selectinputchannel.h"
#include "channelsselection.h"
#include "functionselection.h"
#include "vcwidgetselection.h"
#include "vcaudiotriggers.h"
#include "assignhotkey.h"
#include "inputpatch.h"
#include "qlcmacros.h"
#include "audiobar.h"
#include "chaser.h"

#define KColumnName             0
#define KColumnType             1
#define KColumnAssign           2
#define KColumnInfo             3
#define KColumnMinThreshold     4
#define KColumnMaxThreshold     5
#define KColumnDivisor          6

AudioTriggersConfiguration::AudioTriggersConfiguration(VCAudioTriggers *triggers, Doc *doc,
                                                       int bandsNumber, int maxFrequency)
    : QDialog(triggers)
    , m_doc(doc)
    , m_maxFrequency(maxFrequency)
{
    setupUi(this);

    m_triggers = triggers;

    m_nameEdit->setText(m_triggers->caption());

    m_barsNumSpin->setFixedWidth(70);
    m_barsNumSpin->setFixedHeight(30);
    m_barsNumSpin->setValue(bandsNumber);

    connect(m_barsNumSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateTree()));

    /* Key sequence */
    m_keySequence = QKeySequence(triggers->keySequence());
    m_keyEdit->setText(m_keySequence.toString(QKeySequence::NativeText));

    /* External input */
    m_inputSource = triggers->inputSource();
    updateInputSource();

    connect(m_attachKey, SIGNAL(clicked()), this, SLOT(slotAttachKey()));
    connect(m_detachKey, SIGNAL(clicked()), this, SLOT(slotDetachKey()));

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseInputClicked()));

    m_tree->setAlternatingRowColors(true);
    m_tree->setRootIsDecorated(false);
    m_tree->setSelectionMode(QAbstractItemView::NoSelection);
    m_tree->setAllColumnsShowFocus(true);

    updateTree();
}

AudioTriggersConfiguration::~AudioTriggersConfiguration()
{
}

void AudioTriggersConfiguration::accept()
{
    m_triggers->setCaption(m_nameEdit->text());
    m_triggers->setKeySequence(m_keySequence);
    m_triggers->setInputSource(m_inputSource);

    /* Close dialog */
    QDialog::accept();
}

void AudioTriggersConfiguration::updateTreeItem(QTreeWidgetItem *item, int idx)
{
    if (item == NULL)
        return;

    AudioBar *bar = m_triggers->getSpectrumBar(idx);
    bar->setName(item->text(KColumnName));

    bar->debugInfo();
    QComboBox *currCombo = (QComboBox *)m_tree->itemWidget(item, KColumnType);
    if (currCombo != NULL)
    {
        disconnect(currCombo, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(slotTypeComboChanged(int)));
        m_tree->removeItemWidget(item, KColumnType);
    }
    m_tree->removeItemWidget(item, KColumnAssign);
    m_tree->removeItemWidget(item, KColumnMinThreshold);
    m_tree->removeItemWidget(item, KColumnMaxThreshold);
    m_tree->removeItemWidget(item, KColumnDivisor);

    QComboBox *combo = new QComboBox();
    combo->addItem(QIcon(":/uncheck.png"), tr("None"), idx);
    combo->addItem(QIcon(":/intensity.png"), tr("DMX"), idx);
    combo->addItem(QIcon(":/function.png"), tr("Function"), idx);
    combo->addItem(QIcon(":/virtualconsole.png"), tr("VC Widget"), idx);
    combo->setCurrentIndex(bar->m_type);
    m_tree->setItemWidget(item, KColumnType, combo);
    connect(combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTypeComboChanged(int)));

    if (bar->m_type == AudioBar::DMXBar)
    {
        QToolButton *btn = new QToolButton();
        btn->setIcon(QIcon(":/attach.png"));
        btn->setProperty("index", idx);
        m_tree->setItemWidget(item, KColumnAssign, btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(slotDmxSelectionClicked()));
        item->setText(KColumnInfo, tr("%1 channels").arg(bar->m_dmxChannels.count()));
    }
    else if (bar->m_type == AudioBar::FunctionBar)
    {
        QToolButton *btn = new QToolButton();
        btn->setIcon(QIcon(":/attach.png"));
        btn->setProperty("index", idx);
        m_tree->setItemWidget(item, KColumnAssign, btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(slotFunctionSelectionClicked()));
        if (bar->m_function != NULL)
        {
            item->setText(KColumnInfo, bar->m_function->name());
            switch (bar->m_function->type())
            {
            case Function::Scene: item->setIcon(KColumnInfo, QIcon(":/scene.png")); break;
            case Function::Chaser:
                if (qobject_cast<const Chaser*>(bar->m_function)->isSequence() == true)
                    item->setIcon(KColumnInfo, QIcon(":/sequence.png"));
                else
                    item->setIcon(KColumnInfo, QIcon(":/chaser.png"));
            break;
            case Function::EFX:item->setIcon(KColumnInfo, QIcon(":/efx.png")); break;
            case Function::Collection:item->setIcon(KColumnInfo, QIcon(":/collection.png")); break;
            case Function::RGBMatrix:item->setIcon(KColumnInfo, QIcon(":/rgbmatrix.png")); break;
            case Function::Script:item->setIcon(KColumnInfo, QIcon(":/script.png")); break;
            case Function::Show:item->setIcon(KColumnInfo, QIcon(":/show.png")); break;
            case Function::Audio:item->setIcon(KColumnInfo, QIcon(":/audio.png")); break;
            default: item->setIcon(KColumnInfo, QIcon(":/function.png")); break;
            }
        }
        else
        {
            item->setText(KColumnInfo, tr("No function"));
            item->setIcon(KColumnInfo, QIcon());
        }
    }
    else if (bar->m_type == AudioBar::VCWidgetBar)
    {
        QToolButton *btn = new QToolButton();
        btn->setIcon(QIcon(":/attach.png"));
        btn->setProperty("index", idx);
        m_tree->setItemWidget(item, KColumnAssign, btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(slotWidgetSelectionClicked()));
        if (bar->widget() != NULL)
        {
            item->setText(KColumnInfo, bar->widget()->caption());
            item->setIcon(KColumnInfo, VCWidget::typeToIcon(bar->widget()->type()));
        }
        else
        {
            item->setText(KColumnInfo, tr("No widget"));
            item->setIcon(KColumnInfo, QIcon());
        }
    }
    else
    {
        item->setText(KColumnInfo, tr("Not assigned"));
        item->setIcon(KColumnInfo, QIcon());
    }

    if (bar->m_type == AudioBar::FunctionBar 
        || (bar->m_type == AudioBar::VCWidgetBar && ((bar->widget() == NULL) || bar->widget()->type() != VCWidget::SliderWidget)))
    {
        QSpinBox *minspin = new QSpinBox();
        minspin->setMinimum(5);
        minspin->setMaximum(95);
        minspin->setSingleStep(1);
        minspin->setSuffix("%");
        minspin->setValue(SCALE(float(bar->m_minThreshold), 0.0, 255.0, 0.0, 100.0));
        minspin->setProperty("index", idx);
        connect(minspin, SIGNAL(valueChanged(int)), this, SLOT(slotMinThresholdChanged(int)));
        m_tree->setItemWidget(item, KColumnMinThreshold, minspin);

        QSpinBox *maxspin = new QSpinBox();
        maxspin->setMinimum(5);
        maxspin->setMaximum(95);
        maxspin->setSingleStep(1);
        maxspin->setSuffix("%");
        maxspin->setValue(SCALE(float(bar->m_maxThreshold), 0.0, 255.0, 0.0, 100.0));
        maxspin->setProperty("index", idx);
        connect(maxspin, SIGNAL(valueChanged(int)), this, SLOT(slotMaxThresholdChanged(int)));
        m_tree->setItemWidget(item, KColumnMaxThreshold, maxspin);
    }

    if (bar->m_type == AudioBar::VCWidgetBar
        && bar->widget() != NULL 
        && (bar->widget()->type() == VCWidget::SpeedDialWidget || bar->widget()->type() == VCWidget::CueListWidget))
    {
        QSpinBox *divisor = new QSpinBox();
        divisor->setMinimum(1);
        divisor->setMaximum(64);
        divisor->setSingleStep(1);
        divisor->setValue(bar->m_divisor);
        divisor->setProperty("index", idx);
        connect(divisor, SIGNAL(valueChanged(int)), this, SLOT(slotDivisorChanged(int)));
        m_tree->setItemWidget(item, KColumnDivisor, divisor);
    }
}

void AudioTriggersConfiguration::updateTree()
{
    if (m_barsNumSpin->value() < m_barsNumSpin->minimum() ||
        m_barsNumSpin->value() > m_barsNumSpin->maximum())
            return;

    m_tree->clear();
    m_triggers->setSpectrumBarsNumber(m_barsNumSpin->value());

    // add volume item
    QTreeWidgetItem *volItem = new QTreeWidgetItem(m_tree);
    volItem->setText(KColumnName, tr("Volume Bar"));
    updateTreeItem(volItem, 1000);

    double freqIncr = (double)m_maxFrequency / m_barsNumSpin->value();
    double freqCount = 0.0;

    for (int i = 0; i < m_barsNumSpin->value(); i++)
    {
        QTreeWidgetItem *barItem = new QTreeWidgetItem(m_tree);
        barItem->setText(KColumnName, tr("#%1 (%2Hz - %3Hz)").arg(i + 1).arg((int)freqCount).arg((int)(freqCount + freqIncr)));
        updateTreeItem(barItem, i);
        freqCount += freqIncr;
    }

    m_tree->resizeColumnToContents(KColumnName);
    m_tree->resizeColumnToContents(KColumnType);
    m_tree->resizeColumnToContents(KColumnAssign);
    m_tree->resizeColumnToContents(KColumnInfo);
    m_tree->resizeColumnToContents(KColumnMinThreshold);
    m_tree->resizeColumnToContents(KColumnMaxThreshold);
    m_tree->resizeColumnToContents(KColumnDivisor);

}

void AudioTriggersConfiguration::slotTypeComboChanged(int comboIndex)
{
    QComboBox *combo = (QComboBox *)sender();
    int index = combo->itemData(comboIndex).toInt();
    QTreeWidgetItem *item = NULL;
    if (index == 1000)
        item = m_tree->topLevelItem(0);
    else
        item = m_tree->topLevelItem(index + 1);

    m_triggers->setSpectrumBarType(index, comboIndex);

    updateTreeItem(item, index);
}

void AudioTriggersConfiguration::slotDmxSelectionClicked()
{
    QToolButton *btn = (QToolButton *)sender();
    QVariant prop = btn->property("index");
    if (prop.isValid())
    {
        AudioBar *bar = m_triggers->getSpectrumBar(prop.toInt());
        ChannelsSelection cfg(m_doc, this);
        if (bar != NULL)
            cfg.setChannelsList(bar->m_dmxChannels);
        if (cfg.exec() == QDialog::Rejected)
            return; // User pressed cancel

        QList<SceneValue> dmxList = cfg.channelsList();
        if (bar != NULL)
            bar->attachDmxChannels(m_doc, dmxList);
        QTreeWidgetItem *item = NULL;
        if (prop.toInt() == 1000)
            item = m_tree->topLevelItem(0);
        else
            item = m_tree->topLevelItem(prop.toInt() + 1);
        updateTreeItem(item, prop.toInt());
    }
}

void AudioTriggersConfiguration::slotFunctionSelectionClicked()
{
    QToolButton *btn = (QToolButton *)sender();
    QVariant prop = btn->property("index");
    if (prop.isValid())
    {
        FunctionSelection fs(this, m_doc);
        fs.setMultiSelection(false);
        if (fs.exec() == QDialog::Rejected || fs.selection().size() == 0)
            return; // User pressed cancel or made an invalid selection
        AudioBar *bar = m_triggers->getSpectrumBar(prop.toInt());
        Function *f = m_doc->function(fs.selection().first());
        if (bar != NULL && f != NULL)
            bar->attachFunction(f);

        QTreeWidgetItem *item = NULL;
        if (prop.toInt() == 1000)
            item = m_tree->topLevelItem(0);
        else
            item = m_tree->topLevelItem(prop.toInt() + 1);
        updateTreeItem(item, prop.toInt());
    }
}

void AudioTriggersConfiguration::slotWidgetSelectionClicked()
{
    QToolButton *btn = (QToolButton *)sender();
    QVariant prop = btn->property("index");
    if (prop.isValid())
    {
        QList<int> filters;
        filters.append(VCWidget::SliderWidget);
        filters.append(VCWidget::ButtonWidget);
        filters.append(VCWidget::SpeedDialWidget);
        filters.append(VCWidget::CueListWidget);
        VCWidgetSelection ws(filters, this);
        if (ws.exec() == QDialog::Rejected || ws.getSelectedWidget() == 0)
            return; // User pressed cancel or did not select any widget
        AudioBar *bar = m_triggers->getSpectrumBar(prop.toInt());
        if (bar != NULL)
        {
            bar->attachWidget(ws.getSelectedWidget()->id());
        }

        QTreeWidgetItem *item = NULL;
        if (prop.toInt() == 1000)
            item = m_tree->topLevelItem(0);
        else
            item = m_tree->topLevelItem(prop.toInt() + 1);
        updateTreeItem(item, prop.toInt());
    }
}

void AudioTriggersConfiguration::slotMinThresholdChanged(int val)
{
    QSpinBox *spin = (QSpinBox *)sender();
    QVariant prop = spin->property("index");
    if (prop.isValid())
    {
        AudioBar *bar = m_triggers->getSpectrumBar(prop.toInt());
        uchar scaledVal = SCALE(float(val), 0.0, 100.0, 0.0, 255.0);
        if (bar != NULL)
            bar->setMinThreshold(scaledVal);
    }
}

void AudioTriggersConfiguration::slotMaxThresholdChanged(int val)
{
    QSpinBox *spin = (QSpinBox *)sender();
    QVariant prop = spin->property("index");
    if (prop.isValid())
    {
        AudioBar *bar = m_triggers->getSpectrumBar(prop.toInt());
        uchar scaledVal = SCALE(float(val), 0.0, 100.0, 0.0, 255.0);
        if (bar != NULL)
            bar->setMaxThreshold(scaledVal);
    }
}

void AudioTriggersConfiguration::slotDivisorChanged(int val)
{
    QSpinBox *spin = (QSpinBox *)sender();
    QVariant prop = spin->property("index");
    if (prop.isValid())
    {
        AudioBar *bar = m_triggers->getSpectrumBar(prop.toInt());
        if (bar != NULL)
            bar->setDivisor(val);
    }
}

void AudioTriggersConfiguration::slotAttachKey()
{
    AssignHotKey ahk(this, m_keySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_keySequence = QKeySequence(ahk.keySequence());
        m_keyEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
    }
}

void AudioTriggersConfiguration::slotDetachKey()
{
    m_keySequence = QKeySequence();
    m_keyEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
}

void AudioTriggersConfiguration::slotAutoDetectInputToggled(bool checked)
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
}

void AudioTriggersConfiguration::slotInputValueChanged(quint32 universe, quint32 channel)
{
    if (m_inputSource != NULL)
        delete m_inputSource;
    m_inputSource = new QLCInputSource(universe, (m_triggers->page() << 16) | channel);
    updateInputSource();
}

void AudioTriggersConfiguration::slotChooseInputClicked()
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

void AudioTriggersConfiguration::updateInputSource()
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
