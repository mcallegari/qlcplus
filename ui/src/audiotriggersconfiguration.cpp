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

#define KColumnName             0
#define KColumnType             1
#define KColumnAssign           2
#define KColumnMinThreshold     3
#define KColumnMaxThreshold     4

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
    m_tree->setSelectionMode(QAbstractItemView::NoSelection);

    updateTree();
}

AudioTriggersConfiguration::~AudioTriggersConfiguration()
{
}

void AudioTriggersConfiguration::accept()
{
}

void AudioTriggersConfiguration::addTypeCombo(QString name, int idx)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(m_tree);
    item->setText(KColumnName, name);
    QComboBox *combo = new QComboBox();
    combo->addItem(QIcon(":/uncheck.png"), tr("None"), idx);
    combo->addItem(QIcon(":/intensity.png"), tr("DMX"), idx);
    combo->addItem(QIcon(":/function.png"), tr("Function"), idx);
    combo->addItem(QIcon(":/virtualconsole.png"), tr("VC Widget"), idx);
    m_tree->setItemWidget(item, KColumnType, combo);
    connect(combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTypeComboChanged(int)));
}

void AudioTriggersConfiguration::updateTree()
{
    m_tree->clear();

    // add volume item
    addTypeCombo(tr("Volume Bar"), 1000);

    double freqIncr = (double)m_capture->maxFrequency() / m_barsNumSpin->value();
    double freqCount = 0.0;

    for (int i = 0; i < m_barsNumSpin->value(); i++)
    {
        addTypeCombo(tr("#%1 (%2Hz - %3Hz)").arg(i + 1).arg((int)freqCount).arg((int)(freqCount + freqIncr)), i);
        freqCount += freqIncr;
    }
}

void AudioTriggersConfiguration::slotTypeComboChanged(int comboIndex)
{
    QComboBox *combo = (QComboBox *)sender();
    int index = combo->itemData(comboIndex).toInt();
    QTreeWidgetItem *item = NULL;
    if (index == 1000)
    {
        item = m_tree->topLevelItem(0);
    }
    else
    {
        item = m_tree->topLevelItem(index + 1);
        m_factory->setSpectrumBarType(comboIndex, index);
    }

    //QWidget *widget = m_tree->itemWidget(item, KColumnAssign);
    m_tree->removeItemWidget(item, KColumnAssign);
    m_tree->removeItemWidget(item, KColumnMinThreshold);
    m_tree->removeItemWidget(item, KColumnMaxThreshold);

    if (comboIndex == AudioBar::DMXBar)
    {
        QToolButton *btn = new QToolButton();
        btn->setIcon(QIcon(":/attach.png"));
        btn->setMinimumHeight(32);
        btn->setProperty("index", index);
        m_tree->setItemWidget(item, KColumnAssign, btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(slotDmxSelectionClicked()));
    }
    else if (comboIndex == AudioBar::FunctionBar)
    {
        QSpinBox *minspin = new QSpinBox();
        minspin->setMinimum(5);
        minspin->setMaximum(95);
        minspin->setSingleStep(1);
        minspin->setPrefix("%");
        m_tree->setItemWidget(item, KColumnMinThreshold, minspin);

        QSpinBox *maxspin = new QSpinBox();
        maxspin->setMinimum(5);
        maxspin->setMaximum(95);
        maxspin->setSingleStep(1);
        maxspin->setPrefix("%");
        m_tree->setItemWidget(item, KColumnMaxThreshold, maxspin);
    }
    else if (comboIndex == AudioBar::VCWidgetBar)
    {
        QSpinBox *minspin = new QSpinBox();
        minspin->setMinimum(5);
        minspin->setMaximum(95);
        minspin->setSingleStep(1);
        minspin->setSuffix("%");
        m_tree->setItemWidget(item, KColumnMinThreshold, minspin);

        QSpinBox *maxspin = new QSpinBox();
        maxspin->setMinimum(5);
        maxspin->setMaximum(95);
        maxspin->setSingleStep(1);
        maxspin->setSuffix("%");
        m_tree->setItemWidget(item, KColumnMaxThreshold, maxspin);
    }
}

void AudioTriggersConfiguration::slotDmxSelectionClicked()
{
    ChannelsSelection cfg(m_doc, this);
    if (cfg.exec() == QDialog::Rejected)
        return; // User pressed cancel
}
