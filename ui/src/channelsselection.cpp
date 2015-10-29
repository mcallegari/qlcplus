/*
  Q Light Controller Plus
  channelsselection.cpp

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
#include <QPushButton>
#include <QComboBox>
#include <QDebug>

#include "channelmodifiereditor.h"
#include "channelsselection.h"
#include "channelmodifier.h"
#include "qlcfixturedef.h"
#include "universe.h"
#include "doc.h"

#define KColumnName         0
#define KColumnType         1
#define KColumnSelection    2
#define KColumnBehaviour    3
#define KColumnModifier     4
#define KColumnChIdx        5
#define KColumnID           6

ChannelsSelection::ChannelsSelection(Doc *doc, QWidget *parent, ChannelSelectionType mode)
    : QDialog(parent)
    , m_doc(doc)
    , m_mode(mode)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    QStringList hdrLabels;
    hdrLabels << tr("Name") << tr("Type");

    if (mode == NormalMode)
    {
        hdrLabels << tr("Selected");
    }
    else if (mode == ConfigurationMode)
    {
        setWindowTitle(tr("Channel properties configuration"));
        setWindowIcon(QIcon(":/fade.png"));
        hdrLabels << tr("Can fade") << tr("Behaviour") << tr("Modifier");
    }

    m_channelsTree->setHeaderLabels(hdrLabels);

    updateFixturesTree();

    connect(m_channelsTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChecked(QTreeWidgetItem*, int)));
    connect(m_channelsTree, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotItemExpanded()));
    connect(m_channelsTree, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(slotItemExpanded()));
}

ChannelsSelection::~ChannelsSelection()
{
}

void ChannelsSelection::setChannelsList(QList<SceneValue> list)
{
    if (list.count() > 0)
    {
        m_channelsList = list;
        updateFixturesTree();
    }
}

QList<SceneValue> ChannelsSelection::channelsList()
{
    return m_channelsList;
}

void ChannelsSelection::updateFixturesTree()
{
    m_channelsTree->clear();
    m_channelsTree->setIconSize(QSize(24, 24));
    m_channelsTree->setAllColumnsShowFocus(true);
    InputOutputMap *ioMap = m_doc->inputOutputMap();

    foreach(Fixture *fxi, m_doc->fixtures())
    {
        QTreeWidgetItem *topItem = NULL;
        quint32 uni = fxi->universe();
        for (int i = 0; i < m_channelsTree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = m_channelsTree->topLevelItem(i);
            quint32 tUni = tItem->text(KColumnID).toUInt();
            if (tUni == uni)
            {
                topItem = tItem;
                break;
            }
        }
        // Haven't found this universe node ? Create it.
        if (topItem == NULL)
        {
            topItem = new QTreeWidgetItem(m_channelsTree);
            topItem->setText(KColumnName, tr("Universe %1").arg(uni + 1));
            topItem->setText(KColumnID, QString::number(uni));
            topItem->setExpanded(true);
        }

        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);
        fItem->setText(KColumnName, fxi->name());
        fItem->setIcon(KColumnName, fxi->getIconFromType(fxi->type()));
        fItem->setText(KColumnID, QString::number(fxi->id()));

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel* channel = fxi->channel(c);
            QTreeWidgetItem *item = new QTreeWidgetItem(fItem);
            item->setText(KColumnName, QString("%1:%2").arg(c + 1)
                          .arg(channel->name()));
            item->setIcon(KColumnName, channel->getIcon());
            if (channel->group() == QLCChannel::Intensity &&
                channel->colour() != QLCChannel::NoColour)
                item->setText(KColumnType, QLCChannel::colourToString(channel->colour()));
            else
                item->setText(KColumnType, QLCChannel::groupToString(channel->group()));

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if (m_mode == ConfigurationMode)
            {
                if (fxi->channelCanFade(c))
                    item->setCheckState(KColumnSelection, Qt::Checked);
                else
                    item->setCheckState(KColumnSelection, Qt::Unchecked);

                QComboBox *combo = new QComboBox();
                combo->addItem("HTP", false);
                combo->addItem("LTP", false);
                combo->setProperty("treeItem", qVariantFromValue((void *)item));
                m_channelsTree->setItemWidget(item, KColumnBehaviour, combo);
                quint32 absChannelIdx = fxi->address() + c;
                QList<Universe *> universes = ioMap->claimUniverses();
                if (fxi->universe() < (quint32)universes.count())
                {
                    int caps = universes.at(fxi->universe())->channelCapabilities(absChannelIdx);
                    if (caps & Universe::LTP)
                    {
                        combo->setCurrentIndex(1);
                        // set the other behaviour as true
                        combo->setItemData(0, true, Qt::UserRole);
                    }
                    else
                    {
                        combo->setCurrentIndex(0);
                        // set the other behaviour as true
                        combo->setItemData(1, true, Qt::UserRole);
                    }
                }
                QPushButton *button = new QPushButton();
                ChannelModifier *mod = fxi->channelModifier(c);
                if (mod == NULL)
                    button->setText("...");
                else
                    button->setText(mod->name());
                button->setProperty("treeItem", qVariantFromValue((void *)item));
                m_channelsTree->setItemWidget(item, KColumnModifier, button);
                ioMap->releaseUniverses(false);
                connect(combo, SIGNAL(currentIndexChanged(int)),
                        this, SLOT(slotComboChanged(int)));
                connect(button, SIGNAL(clicked()),
                        this, SLOT(slotModifierButtonClicked()));
            }
            else
            {
                SceneValue scv(fxi->id(), c);
                if (m_channelsList.contains(scv))
                    item->setCheckState(KColumnSelection, Qt::Checked);
                else
                    item->setCheckState(KColumnSelection, Qt::Unchecked);
            }
            item->setText(KColumnID, QString::number(fxi->id()));
            item->setText(KColumnChIdx, QString::number(c));
        }
    }
    m_channelsTree->resizeColumnToContents(KColumnName);
    m_channelsTree->resizeColumnToContents(KColumnType);
    m_channelsTree->resizeColumnToContents(KColumnSelection);
}

QList<QTreeWidgetItem *> ChannelsSelection::getSameChannels(QTreeWidgetItem *item)
{
    QList<QTreeWidgetItem *> sameChannelsList;
    Fixture *fixture = m_doc->fixture(item->text(KColumnID).toUInt());
    if (fixture == NULL)
        return sameChannelsList;

    const QLCFixtureDef *def = fixture->fixtureDef();
    if (def == NULL)
        return sameChannelsList;

    QString manufacturer = def->manufacturer();
    QString model = def->model();
    int chIdx = item->text(KColumnChIdx).toInt();

    qDebug() << "Manuf:" << manufacturer << ", model:" << model << ", ch:" << chIdx;

    for (int t = 0; t < m_channelsTree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_channelsTree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fixItem = uniItem->child(f);
            quint32 fxID = fixItem->text(KColumnID).toUInt();
            Fixture *fxi = m_doc->fixture(fxID);
            if (fxi != NULL)
            {
                const QLCFixtureDef *tmpDef = fxi->fixtureDef();
                if (tmpDef != NULL)
                {
                    QString tmpManuf = tmpDef->manufacturer();
                    QString tmpModel = tmpDef->model();
                    if (tmpManuf == manufacturer && tmpModel == model)
                    {
                        QTreeWidgetItem* chItem = fixItem->child(chIdx);
                        if (chItem != NULL)
                            sameChannelsList.append(chItem);
                    }
                }
            }
        }
    }

    return sameChannelsList;
}

void ChannelsSelection::slotItemChecked(QTreeWidgetItem *item, int col)
{
    if (m_applyAllCheck->isChecked() == false || col != KColumnSelection ||
        item->text(KColumnID).isEmpty())
        return;

    m_channelsTree->blockSignals(true);

    Qt::CheckState enable = item->checkState(KColumnSelection);

    foreach(QTreeWidgetItem *chItem, getSameChannels(item))
        chItem->setCheckState(KColumnSelection, enable);

    m_channelsTree->blockSignals(false);
}

void ChannelsSelection::slotItemExpanded()
{
    m_channelsTree->resizeColumnToContents(KColumnName);
    m_channelsTree->resizeColumnToContents(KColumnType);
    m_channelsTree->resizeColumnToContents(KColumnSelection);
    if (m_mode == ConfigurationMode)
        m_channelsTree->resizeColumnToContents(KColumnBehaviour);
}

void ChannelsSelection::slotComboChanged(int idx)
{
    Q_UNUSED(idx)
    QComboBox *combo = (QComboBox *)sender();
    if (combo != NULL)
    {
        combo->setStyleSheet("QWidget {color:red}");
        if (m_applyAllCheck->isChecked() == true)
        {
            QVariant var = combo->property("treeItem");
            QTreeWidgetItem *item = (QTreeWidgetItem *) var.value<void *>();

            foreach(QTreeWidgetItem *chItem, getSameChannels(item))
            {
                QComboBox *chCombo = (QComboBox *)m_channelsTree->itemWidget(chItem, KColumnBehaviour);
                if (chCombo != NULL)
                {
                    chCombo->blockSignals(true);
                    chCombo->setCurrentIndex(idx);
                    chCombo->setStyleSheet("QWidget {color:red}");
                    chCombo->blockSignals(false);
                }
            }
        }
    }
}

void ChannelsSelection::slotModifierButtonClicked()
{
    QPushButton *button = (QPushButton *)sender();
    if (button == NULL)
        return;

    ChannelModifierEditor cme(m_doc, button->text(), this);
    if (cme.exec() == QDialog::Rejected)
        return; // User pressed cancel

    QString displayName = "...";
    ChannelModifier *modif = cme.selectedModifier();
    if (modif != NULL)
        displayName = modif->name();

    button->setText(displayName);
    if (m_applyAllCheck->isChecked() == true)
    {
        QVariant var = button->property("treeItem");
        QTreeWidgetItem *item = (QTreeWidgetItem *) var.value<void *>();

        foreach(QTreeWidgetItem *chItem, getSameChannels(item))
        {
            QPushButton *chButton = (QPushButton *)m_channelsTree->itemWidget(chItem, KColumnModifier);
            if (chButton != NULL)
                chButton->setText(displayName);
        }
    }
}

void ChannelsSelection::accept()
{
    QList<int> excludeList;
    QList<int> forcedHTPList;
    QList<int> forcedLTPList;
    m_channelsList.clear();

    for (int t = 0; t < m_channelsTree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_channelsTree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fixItem = uniItem->child(f);
            quint32 fxID = fixItem->text(KColumnID).toUInt();
            Fixture *fxi = m_doc->fixture(fxID);
            if (fxi != NULL)
            {
                excludeList.clear();
                forcedHTPList.clear();
                forcedLTPList.clear();
                for (int c = 0; c < fixItem->childCount(); c++)
                {
                    QTreeWidgetItem *chanItem = fixItem->child(c);
                    if (m_mode == ConfigurationMode)
                    {
                        if (chanItem->checkState(KColumnSelection) == Qt::Unchecked)
                            excludeList.append(c);

                        QComboBox *combo = (QComboBox *)m_channelsTree->itemWidget(chanItem, KColumnBehaviour);
                        if (combo != NULL)
                        {
                            int selIdx = combo->currentIndex();
                            if (combo->itemData(selIdx).toBool() == true)
                            {
                                if (selIdx == 0)
                                    forcedHTPList.append(c);
                                else
                                    forcedLTPList.append(c);
                            }
                        }
                        QPushButton *button = (QPushButton *)m_channelsTree->itemWidget(chanItem, KColumnModifier);
                        if (button != NULL)
                        {
                            ChannelModifier *mod = m_doc->modifiersCache()->modifier(button->text());
                            fxi->setChannelModifier((quint32)c, mod);
                        }
                    }
                    else
                    {
                        if (chanItem->checkState(KColumnSelection) == Qt::Checked)
                            m_channelsList.append(SceneValue(fxID, c));
                    }
                }
                if (m_mode == ConfigurationMode)
                {
                    fxi->setExcludeFadeChannels(excludeList);
                    m_doc->updateFixtureChannelCapabilities(fxi->id(), forcedHTPList, forcedLTPList);
                }
            }
        }
    }

    /* Close dialog */
    QDialog::accept();
}
