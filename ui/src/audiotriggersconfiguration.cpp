/*
  Q Light Controller Plus
  audiotriggersconfiguration.cpp

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QTreeWidgetItem>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>


#include "audiotriggersconfiguration.h"
#include "channelsselection.h"
#include "functionselection.h"
#include "vcwidgetselection.h"
#include "qlcmacros.h"
#include "chaser.h"

#define KColumnName             0
#define KColumnType             1
#define KColumnAssign           2
#define KColumnInfo             3
#define KColumnMinThreshold     4
#define KColumnMaxThreshold     5

AudioTriggersConfiguration::AudioTriggersConfiguration(QWidget *parent, Doc *doc, AudioCapture *capture)
    : QDialog(parent)
    , m_doc(doc)
    , m_capture(capture)
{
    setupUi(this);

    Q_ASSERT(capture != NULL);

    m_factory = (AudioTriggerFactory *)parent;

    m_barsNumSpin->setFixedWidth(70);
    m_barsNumSpin->setFixedHeight(30);
    m_barsNumSpin->setValue(m_capture->bandsNumber());

    connect(m_barsNumSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateTree()));

    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
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
    /* Close dialog */
    QDialog::accept();
}

void AudioTriggersConfiguration::updateTreeItem(QTreeWidgetItem *item, int idx)
{
    if (item == NULL)
        return;

    AudioBar *bar = m_factory->getSpectrumBar(idx);
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
            item->setText(KColumnInfo, tr("No function"));
    }
    else if (bar->m_type == AudioBar::VCWidgetBar)
    {
        QToolButton *btn = new QToolButton();
        btn->setIcon(QIcon(":/attach.png"));
        btn->setProperty("index", idx);
        m_tree->setItemWidget(item, KColumnAssign, btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(slotWidgetSelectionClicked()));
        if (bar->m_widget != NULL)
        {
            item->setText(KColumnInfo, bar->m_widget->caption());
            item->setIcon(KColumnInfo, VCWidget::typeToIcon(bar->m_widget->type()));
        }
        else
            item->setText(KColumnInfo, tr("No widget"));
    }
    else
        item->setText(KColumnInfo, tr("Not assigned"));

    if (bar->m_type == AudioBar::FunctionBar || bar->m_type == AudioBar::VCWidgetBar)
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
}

void AudioTriggersConfiguration::updateTree()
{
    if (m_barsNumSpin->value() < m_barsNumSpin->minimum() ||
        m_barsNumSpin->value() > m_barsNumSpin->maximum())
            return;

    m_tree->clear();
    m_factory->setSpectrumBarsNumber(m_barsNumSpin->value());

    // add volume item
    QTreeWidgetItem *volItem = new QTreeWidgetItem(m_tree);
    volItem->setText(KColumnName, tr("Volume Bar"));
    updateTreeItem(volItem, 1000);

    double freqIncr = (double)m_capture->maxFrequency() / m_barsNumSpin->value();
    double freqCount = 0.0;

    for (int i = 0; i < m_barsNumSpin->value(); i++)
    {
        QTreeWidgetItem *barItem = new QTreeWidgetItem(m_tree);
        barItem->setText(KColumnName, tr("#%1 (%2Hz - %3Hz)").arg(i + 1).arg((int)freqCount).arg((int)(freqCount + freqIncr)));
        updateTreeItem(barItem, i);
        freqCount += freqIncr;
    }
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

    m_factory->setSpectrumBarType(index, comboIndex);

    updateTreeItem(item, index);
}

void AudioTriggersConfiguration::slotDmxSelectionClicked()
{
    QToolButton *btn = (QToolButton *)sender();
    QVariant prop = btn->property("index");
    if (prop.isValid())
    {
        AudioBar *bar = m_factory->getSpectrumBar(prop.toInt());
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
        if (fs.exec() == QDialog::Rejected)
            return; // User pressed cancel
        AudioBar *bar = m_factory->getSpectrumBar(prop.toInt());
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
        VCWidgetSelection ws(filters, this);
        if (ws.exec() == QDialog::Rejected)
            return; // User pressed cancel
        AudioBar *bar = m_factory->getSpectrumBar(prop.toInt());
        if (bar != NULL)
            bar->attachWidget(ws.getSelectedWidget());

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
        AudioBar *bar = m_factory->getSpectrumBar(prop.toInt());
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
        AudioBar *bar = m_factory->getSpectrumBar(prop.toInt());
        uchar scaledVal = SCALE(float(val), 0.0, 100.0, 0.0, 255.0);
        if (bar != NULL)
            bar->setMaxThreshold(scaledVal);
    }
}
