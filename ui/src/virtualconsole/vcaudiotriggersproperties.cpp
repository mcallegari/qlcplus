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

#include <QLabel>
#include <QTreeWidgetItem>
#include <QToolButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <qmath.h>

#include "vcaudiotriggersproperties.h"
#include "inputselectionwidget.h"
#include "channelsselection.h"
#include "functionselection.h"
#include "vcwidgetselection.h"
#include "vcaudiotriggers.h"
#include "audiocapture.h"
#include "spectrumgrid.h"
#include "qlcmacros.h"
#include "audiobar.h"

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
    , m_gridCombo(NULL)
    , m_gammaSpin(NULL)
    , m_initialGridMode(triggers->spectrumGridMode())
{
    setupUi(this);

    m_triggers = triggers;

    m_nameEdit->setText(m_triggers->caption());

    QLabel *gridLabel = new QLabel(tr("Spectrum grid:"), this);
    m_gridCombo = new QComboBox(this);
    m_gridCombo->addItem(tr("Uniform log"), int(SpectrumGridMode::LogUniform));
    m_gridCombo->addItem(tr("Semi-log (1-2-5)"), int(SpectrumGridMode::SemiLogPreferred));
    const int gridIndex = m_gridCombo->findData(int(m_initialGridMode));
    m_gridCombo->setCurrentIndex(gridIndex >= 0 ? gridIndex : 0);
    gridLayout->addWidget(gridLabel, 1, 2, Qt::AlignRight);
    gridLayout->addWidget(m_gridCombo, 1, 3);

    QLabel *gammaLabel = new QLabel(tr("Low band emphasis:"), this);
    m_gammaSpin = new QDoubleSpinBox(this);
    m_gammaSpin->setRange(1.0, 4.0);
    m_gammaSpin->setSingleStep(0.1);
    m_gammaSpin->setDecimals(1);
    m_gammaSpin->setValue(triggers->spectrumLowBandGamma());
    gridLayout->addWidget(gammaLabel, 2, 2, Qt::AlignRight);
    gridLayout->addWidget(m_gammaSpin, 2, 3);

    m_barsNumSpin->setFixedWidth(70);
    m_barsNumSpin->setFixedHeight(30);
    m_barsNumSpin->setValue(bandsNumber);

    connect(m_barsNumSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateTree()));
    connect(m_gridCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotGridModeChanged(int)));
    connect(m_gammaSpin, SIGNAL(valueChanged(double)),
            this, SLOT(slotGammaChanged(double)));

    /* External input */
    m_inputSelWidget = new InputSelectionWidget(m_doc, this);
    m_inputSelWidget->setCustomFeedbackVisibility(true);
    m_inputSelWidget->setKeySequence(m_triggers->keySequence());
    m_inputSelWidget->setInputSource(m_triggers->inputSource());
    m_inputSelWidget->setWidgetPage(m_triggers->page());
    m_inputSelWidget->show();
    m_extControlLayout->addWidget(m_inputSelWidget);

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
    m_triggers->setKeySequence(m_inputSelWidget->keySequence());
    m_triggers->setInputSource(m_inputSelWidget->inputSource());

    const SpectrumGridMode mode = SpectrumGridMode(m_gridCombo->currentData().toInt());
    m_triggers->setSpectrumGridMode(mode);
    m_triggers->setSpectrumLowBandGamma(m_gammaSpin->value());

    QDialog::accept();
}

void AudioTriggersConfiguration::slotGammaChanged(double value)
{
    m_triggers->setSpectrumLowBandGamma(value);
    updateTree();
}

void AudioTriggersConfiguration::slotGridModeChanged(int index)
{
    Q_UNUSED(index)
    m_triggers->setSpectrumGridMode(SpectrumGridMode(m_gridCombo->currentData().toInt()));
    updateTree();
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

    if (bar->m_type == AudioBar::BarType::DMXBar)
    {
        QToolButton *btn = new QToolButton();
        btn->setIcon(QIcon(":/attach.png"));
        btn->setProperty("index", idx);
        m_tree->setItemWidget(item, KColumnAssign, btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(slotDmxSelectionClicked()));
        item->setText(KColumnInfo, tr("%1 channels").arg(bar->m_dmxChannels.count()));
    }
    else if (bar->m_type == AudioBar::BarType::FunctionBar)
    {
        QToolButton *btn = new QToolButton();
        btn->setIcon(QIcon(":/attach.png"));
        btn->setProperty("index", idx);
        m_tree->setItemWidget(item, KColumnAssign, btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(slotFunctionSelectionClicked()));
        if (bar->m_function != NULL)
        {
            item->setText(KColumnInfo, bar->m_function->name());
            item->setIcon(KColumnInfo, bar->m_function->getIcon());
        }
        else
        {
            item->setText(KColumnInfo, tr("No function"));
            item->setIcon(KColumnInfo, QIcon());
        }
    }
    else if (bar->m_type == AudioBar::BarType::VCWidgetBar)
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

    if (bar->m_type == AudioBar::BarType::FunctionBar
        || (bar->m_type == AudioBar::BarType::VCWidgetBar && ((bar->widget() == NULL) || bar->widget()->type() != VCWidget::SliderWidget)))
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

    if (bar->m_type == AudioBar::BarType::VCWidgetBar
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

    const int bandsNumber = m_barsNumSpin->value();
    const double minFreq = qMax(1.0, double(AudioCapture::minFrequency()));
    const double maxFreq = double(m_maxFrequency);
    const SpectrumGridMode gridMode = SpectrumGridMode(m_gridCombo->currentData().toInt());
    const double gamma = m_gammaSpin ? m_gammaSpin->value() : 1.0;
    const QVector<double> edges = computeSpectrumBandEdges(bandsNumber, minFreq, maxFreq, gridMode, gamma);

    for (int i = 0; i < bandsNumber; i++)
    {
        int bandStartHz = int(qCeil(edges[i]));
        int bandEndHz = (i == bandsNumber - 1) ? int(maxFreq) : (qCeil(edges[i + 1]) - 1);
        if (bandEndHz <= bandStartHz)
            bandEndHz = bandStartHz;

        QTreeWidgetItem *barItem = new QTreeWidgetItem(m_tree);
        barItem->setText(KColumnName, tr("#%1 (%2Hz - %3Hz)").arg(i + 1).arg(bandStartHz).arg(bandEndHz));
        updateTreeItem(barItem, i);
    }

    m_tree->header()->resizeSections(QHeaderView::ResizeToContents);
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
