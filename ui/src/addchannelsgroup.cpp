/*
  Q Light Controller
  addchannelsgroup.cpp

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

#include <QPushButton>
#include <QDebug>
#include <QSettings>

#include "selectinputchannel.h"
#include "addchannelsgroup.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "channelsgroup.h"
#include "inputpatch.h"
#include "fixture.h"
#include "doc.h"

#define KColumnName  0
#define KColumnType  1
#define KColumnGroup 2
#define KColumnChIdx 3
#define KColumnID    4

#define SETTINGS_APPLYALL "addchannelsgroup/applyall"

AddChannelsGroup::AddChannelsGroup(QWidget* parent, Doc* doc, ChannelsGroup *group)
    : QDialog(parent)
    , m_doc(doc)
    , m_chansGroup(group)
    , m_checkedChannels(0)
    , m_isUpdating(false)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(m_chansGroup != NULL);

    setupUi(this);

    m_tree->header()->setSectionHidden(KColumnID, true);
    m_tree->setSelectionMode(QAbstractItemView::MultiSelection);
    m_tree->setAlternatingRowColors(true);
    m_tree->setIconSize(QSize(20, 20));

    m_groupNameEdit->setText(group->name());

    QList <SceneValue> chans = group->getChannels();
    int ch = 0;

    foreach (Fixture* fxi, m_doc->fixtures())
    {
        QTreeWidgetItem *topItem = NULL;
        quint32 uni = fxi->universe();
        for (int i = 0; i < m_tree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = m_tree->topLevelItem(i);
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
            topItem = new QTreeWidgetItem(m_tree);
            topItem->setText(KColumnName, tr("Universe %1").arg(uni + 1));
            topItem->setText(KColumnID, QString::number(uni));
            topItem->setExpanded(true);
        }

        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);
        fItem->setExpanded(true);
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
            if (chans.count() > ch &&
                chans.at(ch).fxi == fxi->id() &&
                chans.at(ch).channel == c)
            {
                item->setCheckState(KColumnGroup, Qt::Checked);
                m_checkedChannels++;
                ch++;
            }
            else
                item->setCheckState(KColumnGroup, Qt::Unchecked);
            item->setText(KColumnID, QString::number(fxi->id()));
            item->setText(KColumnChIdx, QString::number(c));
        }
    }
    m_tree->resizeColumnToContents(KColumnName);
    m_tree->resizeColumnToContents(KColumnType);
    m_tree->resizeColumnToContents(KColumnGroup);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_APPLYALL);
    if (var.isValid() == true)
    {
       m_applyAllCheck->setChecked(var.toBool());
    }

    m_inputSource = group->inputSource();
    updateInputSource();

    if (m_checkedChannels == 0)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChecked(QTreeWidgetItem*, int)));

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseInputClicked()));
}

AddChannelsGroup::~AddChannelsGroup()
{
    QSettings settings;
    settings.setValue(SETTINGS_APPLYALL, m_applyAllCheck->isChecked());
}

void AddChannelsGroup::accept()
{
    m_chansGroup->resetChannels();

    for (int t = 0; t < m_tree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_tree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fixItem = uniItem->child(f);
            quint32 fxID = fixItem->text(KColumnID).toUInt();
            Fixture *fxi = m_doc->fixture(fxID);
            if (fxi != NULL)
            {
                for (int c = 0; c < fixItem->childCount(); c++)
                {
                    QTreeWidgetItem *chanItem = fixItem->child(c);
                    if (chanItem->checkState(KColumnGroup) == Qt::Checked)
                    {
                        m_chansGroup->addChannel(QString(chanItem->text(KColumnID)).toUInt(),
                                                 QString(chanItem->text(KColumnChIdx)).toUInt());
                        qDebug() << "Added channel with ID:" << chanItem->text(KColumnID) << ", and channel:" << chanItem->text(KColumnChIdx);
                    }
                }
            }
        }
    }

    m_chansGroup->setName(m_groupNameEdit->text());
    m_chansGroup->setInputSource(m_inputSource);
    QDialog::accept();
}

void AddChannelsGroup::slotItemChecked(QTreeWidgetItem *item, int col)
{
    if (m_isUpdating == true || col != KColumnGroup || item->text(KColumnID).isEmpty())
        return;

    m_isUpdating = true;

    if (m_applyAllCheck->isChecked() == false)
    {
        if (item->checkState(col) == Qt::Checked)
            m_checkedChannels++;
        else
            m_checkedChannels--;
    }
    else
    {
        Fixture *fixture = m_doc->fixture(item->text(KColumnID).toUInt());
        if (fixture == NULL)
            return;

        const QLCFixtureDef *def = fixture->fixtureDef();
        if (def == NULL)
            return;

        QString manufacturer = def->manufacturer();
        QString model = def->model();
        QString mode = fixture->fixtureMode() ? fixture->fixtureMode()->name() : "";

        int chIdx = item->text(KColumnChIdx).toInt();
        Qt::CheckState enable = item->checkState(KColumnGroup);

        qDebug() << "Manuf:" << manufacturer << ", model:" << model << ", ch:" << chIdx;

        for (int t = 0; t < m_tree->topLevelItemCount(); t++)
        {
            QTreeWidgetItem *uniItem = m_tree->topLevelItem(t);
            for (int f = 0; f < uniItem->childCount(); f++)
            {
                QTreeWidgetItem *fixItem = uniItem->child(f);
                quint32 fxID = fixItem->text(KColumnID).toUInt();
                Fixture *fxi = m_doc->fixture(fxID);
                if (fxi != NULL)
                {
                    QString tmpMode = fxi->fixtureMode() ? fxi->fixtureMode()->name() : "";
                    const QLCFixtureDef *tmpDef = fxi->fixtureDef();
                    if (tmpDef != NULL)
                    {
                        QString tmpManuf = tmpDef->manufacturer();
                        QString tmpModel = tmpDef->model();
                        if (tmpManuf == manufacturer && tmpModel == model && tmpMode == mode)
                        {
                            QTreeWidgetItem* item = fixItem->child(chIdx);
                            if (item != NULL)
                            {
                                item->setCheckState(KColumnGroup, enable);
                                if (enable == Qt::Checked)
                                    m_checkedChannels++;
                                else
                                    m_checkedChannels--;
                            }
                        }
                    }
                }
            }
        }
    }

    if (m_checkedChannels > 0)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    m_isUpdating = false;
}

void AddChannelsGroup::slotAutoDetectInputToggled(bool checked)
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

void AddChannelsGroup::slotInputValueChanged(quint32 universe, quint32 channel)
{
    m_inputSource = new QLCInputSource(universe, channel);
    updateInputSource();
}

void AddChannelsGroup::slotChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_inputSource = new QLCInputSource(sic.universe(), sic.channel());
        updateInputSource();
    }
}

void AddChannelsGroup::updateInputSource()
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


