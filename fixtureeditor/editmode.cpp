/*
  Q Light Controller - Fixture Editor
  editmode.cpp

  Copyright (C) Heikki Junnila

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
#include <QInputDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QTreeWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>
#include <QPoint>
#include <QDebug>
#include <QSize>
#include <QAction>

#include "addchannelsdialog.h"
#include "qlcfixturemode.h"
#include "qlcfixturehead.h"
#include "qlcfixturedef.h"
#include "qlcphysical.h"
#include "qlcchannel.h"

#include "editphysical.h"
#include "editmode.h"
#include "edithead.h"
#include "util.h"

#define KSettingsGeometry "editmode/geometry"

#define PROP_PTR Qt::UserRole
#define COL_NUM  0
#define COL_NAME 1
#define COL_ACTS_ON 2

EditMode::EditMode(QWidget *parent, QLCFixtureMode *mode)
    : QDialog(parent)
    , m_mode(new QLCFixtureMode(mode->fixtureDef(), mode))
{
    Q_ASSERT(mode != NULL);
    setupUi(this);
    init();
}

EditMode::EditMode(QWidget *parent, QLCFixtureDef *fixtureDef)
    : QDialog(parent)
    , m_mode(new QLCFixtureMode(fixtureDef))
{
    Q_ASSERT(fixtureDef != NULL);
    setupUi(this);
    init();
}

EditMode::~EditMode()
{
    QSettings settings;
    settings.setValue(KSettingsGeometry, saveGeometry());

    delete m_mode;
}

void EditMode::init()
{
    /* Channels page */
    connect(m_addChannelButton, SIGNAL(clicked()), this, SLOT(slotAddChannelClicked()));
    connect(m_removeChannelButton, SIGNAL(clicked()), this, SLOT(slotRemoveChannelClicked()));
    connect(m_raiseChannelButton, SIGNAL(clicked()), this, SLOT(slotRaiseChannelClicked()));
    connect(m_lowerChannelButton, SIGNAL(clicked()), this, SLOT(slotLowerChannelClicked()));

    m_modeNameEdit->setText(m_mode->name());
    m_modeNameEdit->setValidator(CAPS_VALIDATOR(this));
    refreshChannelList();

    /* Heads page */
    connect(m_addHeadButton, SIGNAL(clicked()), this, SLOT(slotAddHeadClicked()));
    connect(m_removeHeadButton, SIGNAL(clicked()), this, SLOT(slotRemoveHeadClicked()));
    connect(m_editHeadButton, SIGNAL(clicked()), this, SLOT(slotEditHeadClicked()));
    connect(m_raiseHeadButton, SIGNAL(clicked()), this, SLOT(slotRaiseHeadClicked()));
    connect(m_lowerHeadButton, SIGNAL(clicked()), this, SLOT(slotLowerHeadClicked()));

    refreshHeadList();

    /* Physical page */
    m_phyEdit = new EditPhysical(m_mode->physical(), this);
    m_phyEdit->show();
    physicalLayout->addWidget(m_phyEdit);

    if (m_mode->useGlobalPhysical() == false)
        m_overridePhyCheck->setChecked(true);
    slotPhysicalModeChanged();

    connect(m_globalPhyCheck, SIGNAL(clicked(bool)), this, SLOT(slotPhysicalModeChanged()));
    connect(m_overridePhyCheck, SIGNAL(clicked(bool)), this, SLOT(slotPhysicalModeChanged()));
    /* Forward copy/paste requests up to reach the main FixtureEditor clipboard */
    connect(m_phyEdit, SIGNAL(copyToClipboard(QLCPhysical)), this, SIGNAL(copyToClipboard(QLCPhysical)));
    connect(m_phyEdit, SIGNAL(requestPasteFromClipboard()), this, SIGNAL(requestPasteFromClipboard()));

    // Close shortcut
    QAction *action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    // Geometry
    QSettings settings;
    QVariant var = settings.value(KSettingsGeometry);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
}

QLCFixtureMode *EditMode::mode()
{
    return m_mode;
}

/****************************************************************************
 * Channels page
 ****************************************************************************/

void EditMode::slotAddChannelClicked()
{
    AddChannelsDialog ach(m_mode->fixtureDef()->channels(), m_mode->channels());
    if (ach.exec() != QDialog::Accepted)
        return;

    QList<QLCChannel *> newChannelList = ach.getModeChannelsList();

    // clear the previous list
    m_mode->removeAllChannels();

    // Append the channels
    foreach (QLCChannel *ch, newChannelList)
        m_mode->insertChannel(ch, m_mode->channels().size());

    // Easier to refresh the whole list
    refreshChannelList();
}

void EditMode::slotRemoveChannelClicked()
{
    QLCChannel *ch = currentChannel();

    if (ch != NULL)
    {
        QTreeWidgetItem *item;
        QString select;

        // Pick the item above or below to be selected next
        item = m_channelList->itemAbove(m_channelList->currentItem());
        if (item == NULL)
            item = m_channelList->itemBelow(m_channelList->currentItem());
        if (item != NULL)
            select = item->text(COL_NAME);

        // Remove the channel and the listview item
        m_mode->removeChannel(ch);
        delete m_channelList->currentItem();

        // Easier to refresh the whole list than to decrement all
        // channel numbers after the inserted item
        refreshChannelList();

        // Select another channel
        selectChannel(select);
    }
}

void EditMode::slotRaiseChannelClicked()
{
    QLCChannel *ch = currentChannel();
    int index = 0;

    if (ch == NULL)
        return;

    index = m_mode->channelNumber(ch) - 1;

    // Don't move beyond the beginning of the list
    if (index < 0)
        return;

    m_mode->removeChannel(ch);
    m_mode->insertChannel(ch, index);

    refreshChannelList();
    selectChannel(ch->name());
}

void EditMode::slotLowerChannelClicked()
{
    QLCChannel *ch = currentChannel();
    int index = 0;

    if (ch == NULL)
        return;

    index = m_mode->channelNumber(ch) + 1;

    // Don't move beyond the end of the list
    if (index >= m_mode->channels().size())
        return;

    m_mode->removeChannel(ch);
    m_mode->insertChannel(ch, index);

    refreshChannelList();
    selectChannel(ch->name());
}

void EditMode::refreshChannelList()
{
    m_channelList->clear();

    for (int i = 0; i < m_mode->channels().size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_channelList);
        QLCChannel *ch = m_mode->channel(i);
        quint32 actsOnChannelIndex = m_mode->channelActsOn(i);

        Q_ASSERT(ch != NULL);

        QString str;
        item->setText(COL_NUM, str.asprintf("%.3d", (i + 1)));
        item->setText(COL_NAME, ch->name());
        item->setIcon(COL_NAME, ch->getIcon());
        item->setData(COL_NAME, PROP_PTR, (qulonglong) ch);

        QStringList comboList;

        comboList << "-";

        for (int index = 0; index < m_mode->channels().size(); index++)
        {
            QLCChannel *currentChannel = m_mode->channels().at(index);
            comboList << QString::number(index + 1) + " - " + currentChannel->name();
        }

        QComboBox *comboBox = new QComboBox(this);
        comboBox->addItems(comboList);

        if (actsOnChannelIndex != QLCChannel::invalid())
            comboBox->setCurrentIndex(actsOnChannelIndex + 1);

        m_channelList->setItemWidget(item, COL_ACTS_ON, comboBox);

        connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setActsOnChannel(int)));
    }
    m_channelList->header()->resizeSections(QHeaderView::ResizeToContents);
}

QLCChannel* EditMode::currentChannel()
{
    QTreeWidgetItem *item;
    QLCChannel *ch = NULL;

    // Convert the string-form ulong to a QLCChannel pointer and return it
    item = m_channelList->currentItem();
    if (item != NULL)
        ch = (QLCChannel*)item->data(COL_NAME, PROP_PTR).toULongLong();

    return ch;
}

void EditMode::selectChannel(const QString &name)
{
    QTreeWidgetItemIterator it(m_channelList);
    while (*it != NULL)
    {
        if ((*it)->text(COL_NAME) == name)
        {
            m_channelList->setCurrentItem((*it));
            break;
        }

        ++it;
    }
}

void EditMode::setActsOnChannel(int index)
{
    quint32 chIndex = m_mode->channelNumber(currentChannel());
    quint32 actsOnChannel = index == 0 ? QLCChannel::invalid() : index - 1;

    m_mode->setChannelActsOn(chIndex, actsOnChannel);
}

/****************************************************************************
 * Heads page
 ****************************************************************************/

void EditMode::slotAddHeadClicked()
{
    EditHead eh(this, QLCFixtureHead(), m_mode);
    if (eh.exec() == QDialog::Accepted)
    {
        m_mode->insertHead(-1, eh.head());
        refreshHeadList();
    }
}

void EditMode::slotRemoveHeadClicked()
{
    QTreeWidgetItem *item = m_headList->currentItem();
    if (item == NULL)
        return;

    int index = m_headList->indexOfTopLevelItem(item);
    m_mode->removeHead(index);
    refreshHeadList();
}

void EditMode::slotEditHeadClicked()
{
    QTreeWidgetItem *item = m_headList->currentItem();
    if (item == NULL)
        return;

    EditHead eh(this, currentHead(), m_mode);
    if (eh.exec() == QDialog::Accepted)
    {
        int index = m_headList->indexOfTopLevelItem(item);
        m_mode->replaceHead(index, eh.head());
        refreshHeadList();
    }
}

void EditMode::slotRaiseHeadClicked()
{
    QTreeWidgetItem *item = m_headList->currentItem();
    if (item == NULL)
        return;

    int index = m_headList->indexOfTopLevelItem(item);

    // Don't move beyond the beginning of the list
    if ((index - 1) < 0)
        return;

    QLCFixtureHead head = currentHead();
    m_mode->removeHead(index);
    m_mode->insertHead(index - 1, head);

    refreshHeadList();
    selectHead(index - 1);
}

void EditMode::slotLowerHeadClicked()
{
    QTreeWidgetItem *item = m_headList->currentItem();
    if (item == NULL)
        return;

    int index = m_headList->indexOfTopLevelItem(item);

    // Don't move beyond the end of the list
    if ((index + 1) >= m_mode->heads().size())
        return;

    QLCFixtureHead head = currentHead();
    m_mode->removeHead(index);
    m_mode->insertHead(index + 1, head);

    refreshHeadList();
    selectHead(index + 1);
}

void EditMode::refreshHeadList()
{
    m_headList->clear();

    for (int i = 0; i < m_mode->heads().size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_headList);
        QLCFixtureHead head = m_mode->heads().at(i);
        QList <quint32> channels(head.channels());
        QString summary;

        std::sort(channels.begin(), channels.end());      

        QListIterator <quint32> it(channels);
        while (it.hasNext() == true)
        {
            quint32 chnum = it.next();
            const QLCChannel *ch = m_mode->channel(chnum);
            QTreeWidgetItem *chitem = new QTreeWidgetItem(item);
            if (ch != NULL)
                chitem->setText(0, QString("%1: %2").arg(chnum + 1).arg(ch->name()));
            else
                chitem->setText(0, QString("%1: INVALID!"));
            chitem->setFlags(Qt::NoItemFlags); // Disable channel selection inside heads

            summary += QString::number(chnum + 1);
            if (it.hasNext() == true)
                summary += QString(", ");
        }

        item->setText(0, QString("Head %1 (%2)").arg(i + 1).arg(summary));
    }
    m_headList->resizeColumnToContents(0);
}

QLCFixtureHead EditMode::currentHead()
{
    QTreeWidgetItem *item = m_headList->currentItem();
    if (item == NULL)
        return QLCFixtureHead();

    int index = m_headList->indexOfTopLevelItem(item);
    return m_mode->heads().at(index);
}

void EditMode::selectHead(int index)
{
    if (index >= m_headList->topLevelItemCount())
        return;

    QTreeWidgetItem *item = m_headList->topLevelItem(index);
    m_headList->setCurrentItem(item);
}

void EditMode::pasteFromClipboard(QLCPhysical clipboard)
{
    m_phyEdit->pasteFromClipboard(clipboard);
}

void EditMode::slotPhysicalModeChanged()
{
    m_phyEdit->setEnabled(m_globalPhyCheck->isChecked() ? false : true);
    if (m_globalPhyCheck->isChecked())
        m_mode->resetPhysical();
    else
        m_mode->setPhysical(m_phyEdit->physical());
}

/*****************************************************************************
 * Accept
 *****************************************************************************/

void EditMode::accept()
{
    m_mode->setName(m_modeNameEdit->text());
    if (m_overridePhyCheck->isChecked())
        m_mode->setPhysical(m_phyEdit->physical());

    QDialog::accept();
}
