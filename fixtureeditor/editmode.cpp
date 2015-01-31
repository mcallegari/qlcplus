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

#include "addchannelsdialog.h"
#include "qlcfixturemode.h"
#include "qlcfixturehead.h"
#include "qlcfixturedef.h"
#include "qlcphysical.h"
#include "qlcchannel.h"

#include "editmode.h"
#include "edithead.h"
#include "util.h"
#include "app.h"

#define KSettingsGeometry "editmode/geometry"

#define PROP_PTR Qt::UserRole
#define COL_NUM  0
#define COL_NAME 1

EditMode::EditMode(QWidget* parent, QLCFixtureMode* mode)
    : QDialog(parent)
    , m_mode(new QLCFixtureMode(mode->fixtureDef(), mode))
{
    Q_ASSERT(mode != NULL);
    setupUi(this);
    init();
}

EditMode::EditMode(QWidget* parent, QLCFixtureDef* fixtureDef)
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
    QString str;
    QLCPhysical physical = m_mode->physical();

    /* Channels page */
    connect(m_addChannelButton, SIGNAL(clicked()),
            this, SLOT(slotAddChannelClicked()));
    connect(m_removeChannelButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveChannelClicked()));
    connect(m_raiseChannelButton, SIGNAL(clicked()),
            this, SLOT(slotRaiseChannelClicked()));
    connect(m_lowerChannelButton, SIGNAL(clicked()),
            this, SLOT(slotLowerChannelClicked()));

    m_modeNameEdit->setText(m_mode->name());
    m_modeNameEdit->setValidator(CAPS_VALIDATOR(this));
    refreshChannelList();

    /* Heads page */
    connect(m_addHeadButton, SIGNAL(clicked()),
            this, SLOT(slotAddHeadClicked()));
    connect(m_removeHeadButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveHeadClicked()));
    connect(m_editHeadButton, SIGNAL(clicked()),
            this, SLOT(slotEditHeadClicked()));
    connect(m_raiseHeadButton, SIGNAL(clicked()),
            this, SLOT(slotRaiseHeadClicked()));
    connect(m_lowerHeadButton, SIGNAL(clicked()),
            this, SLOT(slotLowerHeadClicked()));

    refreshHeadList();

    /* Physical page */
    m_bulbTypeCombo->setEditText(physical.bulbType());
    m_bulbLumensSpin->setValue(physical.bulbLumens());
    m_bulbTempCombo->setEditText(str.setNum(physical.bulbColourTemperature()));

    m_weightSpin->setValue(physical.weight());
    m_widthSpin->setValue(physical.width());
    m_heightSpin->setValue(physical.height());
    m_depthSpin->setValue(physical.depth());

    m_lensNameCombo->setEditText(physical.lensName());
    m_lensDegreesMinSpin->setValue(physical.lensDegreesMin());
    m_lensDegreesMaxSpin->setValue(physical.lensDegreesMax());

    m_focusTypeCombo->setEditText(physical.focusType());
    m_panMaxSpin->setValue(physical.focusPanMax());
    m_tiltMaxSpin->setValue(physical.focusTiltMax());

    m_powerConsumptionSpin->setValue(physical.powerConsumption());
    m_dmxConnectorCombo->setEditText(physical.dmxConnector());

    connect(copyClipboardButton, SIGNAL(clicked()),
            this, SLOT(slotCopyToClipboard()));
    connect(pasteClipboardButton, SIGNAL(clicked()),
            this, SLOT(slotPasteFromClipboard()));

    // Close shortcut
    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    // Geometry
    QSettings settings;
    QVariant var = settings.value(KSettingsGeometry);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
}

/****************************************************************************
 * Channels page
 ****************************************************************************/

void EditMode::slotAddChannelClicked()
{
    AddChannelsDialog ach(m_mode->fixtureDef()->channels(), m_mode->channels());
    if (ach.exec() != QDialog::Accepted)
        return;

    QList <QLCChannel *> newChannelList = ach.getModeChannelsList();

    // clear the previous list
    m_mode->removeAllChannels();

    // Append the channels
    foreach(QLCChannel *ch, newChannelList)
        m_mode->insertChannel(ch, m_mode->channels().size());

    // Easier to refresh the whole list
    refreshChannelList();
}

void EditMode::slotRemoveChannelClicked()
{
    QLCChannel* ch = currentChannel();

    if (ch != NULL)
    {
        QTreeWidgetItem* item;
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
    QLCChannel* ch = currentChannel();
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
    QLCChannel* ch = currentChannel();
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
        QTreeWidgetItem* item = new QTreeWidgetItem(m_channelList);
        QLCChannel* ch = m_mode->channel(i);
        Q_ASSERT(ch != NULL);

        QString str;
        str.sprintf("%.3d", (i + 1));
        item->setText(COL_NUM, str);
        item->setText(COL_NAME, ch->name());
        item->setIcon(COL_NAME, ch->getIcon());
        item->setData(COL_NAME, PROP_PTR, (qulonglong) ch);
    }
    m_channelList->resizeColumnToContents(COL_NUM);
    m_channelList->resizeColumnToContents(COL_NAME);
}

QLCChannel* EditMode::currentChannel()
{
    QTreeWidgetItem* item;
    QLCChannel* ch = NULL;

    // Convert the string-form ulong to a QLCChannel pointer and return it
    item = m_channelList->currentItem();
    if (item != NULL)
        ch = (QLCChannel*) item->data(COL_NAME, PROP_PTR).toULongLong();

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
    QTreeWidgetItem* item = m_headList->currentItem();
    if (item == NULL)
        return;

    int index = m_headList->indexOfTopLevelItem(item);
    m_mode->removeHead(index);
    refreshHeadList();
}

void EditMode::slotEditHeadClicked()
{
    QTreeWidgetItem* item = m_headList->currentItem();
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
    QTreeWidgetItem* item = m_headList->currentItem();
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
    QTreeWidgetItem* item = m_headList->currentItem();
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
        QTreeWidgetItem* item = new QTreeWidgetItem(m_headList);

        QLCFixtureHead head = m_mode->heads().at(i);

        QList <quint32> channels(head.channels().toList());
        qSort(channels.begin(), channels.end());

        QString summary;

        QListIterator <quint32> it(channels);
        while (it.hasNext() == true)
        {
            quint32 chnum = it.next();
            const QLCChannel* ch = m_mode->channel(chnum);
            QTreeWidgetItem* chitem = new QTreeWidgetItem(item);
            if (ch != NULL)
                chitem->setText(0, QString("%1: %2").arg(chnum).arg(ch->name()));
            else
                chitem->setText(0, QString("%1: INVALID!"));
            chitem->setFlags(0); // Disable channel selection inside heads

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
    QTreeWidgetItem* item = m_headList->currentItem();
    if (item == NULL)
        return QLCFixtureHead();

    int index = m_headList->indexOfTopLevelItem(item);
    return m_mode->heads().at(index);
}

void EditMode::selectHead(int index)
{
    if (index >= m_headList->topLevelItemCount())
        return;

    QTreeWidgetItem* item = m_headList->topLevelItem(index);
    m_headList->setCurrentItem(item);
}

/*********************************************************************
 * Clipboard
 *********************************************************************/

QLCPhysical EditMode::getClipboard()
{
    return m_clipboard;
}

void EditMode::setClipboard(QLCPhysical physical)
{
    m_clipboard = physical;
}

void EditMode::slotCopyToClipboard()
{
    m_clipboard.setBulbType(m_bulbTypeCombo->currentText());
    m_clipboard.setBulbLumens(m_bulbLumensSpin->value());
    m_clipboard.setBulbColourTemperature(m_bulbTempCombo->currentText().toInt());
    m_clipboard.setWeight(m_weightSpin->value());
    m_clipboard.setWidth(m_widthSpin->value());
    m_clipboard.setHeight(m_heightSpin->value());
    m_clipboard.setDepth(m_depthSpin->value());
    m_clipboard.setLensName(m_lensNameCombo->currentText());
    m_clipboard.setLensDegreesMin(m_lensDegreesMinSpin->value());
    m_clipboard.setLensDegreesMax(m_lensDegreesMaxSpin->value());
    m_clipboard.setFocusType(m_focusTypeCombo->currentText());
    m_clipboard.setFocusPanMax(m_panMaxSpin->value());
    m_clipboard.setFocusTiltMax(m_tiltMaxSpin->value());
    m_clipboard.setPowerConsumption(m_powerConsumptionSpin->value());
    m_clipboard.setDmxConnector(m_dmxConnectorCombo->currentText());
}

void EditMode::slotPasteFromClipboard()
{
    m_bulbLumensSpin->setValue(m_clipboard.bulbLumens());
    m_weightSpin->setValue(m_clipboard.weight());
    m_widthSpin->setValue(m_clipboard.width());
    m_heightSpin->setValue(m_clipboard.height());
    m_depthSpin->setValue(m_clipboard.depth());
    m_lensDegreesMinSpin->setValue(m_clipboard.lensDegreesMin());
    m_lensDegreesMaxSpin->setValue(m_clipboard.lensDegreesMax());
    m_panMaxSpin->setValue(m_clipboard.focusPanMax());
    m_tiltMaxSpin->setValue(m_clipboard.focusTiltMax());
    m_powerConsumptionSpin->setValue(m_clipboard.powerConsumption());

    m_bulbTypeCombo->setEditText(m_clipboard.bulbType());
    m_bulbTempCombo->setEditText(QString::number(m_clipboard.bulbColourTemperature()));
    m_lensNameCombo->setEditText(m_clipboard.lensName());
    m_focusTypeCombo->setEditText(m_clipboard.focusType());
    m_dmxConnectorCombo->setEditText(m_clipboard.dmxConnector());
}



/*****************************************************************************
 * Accept
 *****************************************************************************/

void EditMode::accept()
{
    QLCPhysical physical = m_mode->physical();

    physical.setBulbType(m_bulbTypeCombo->currentText());
    physical.setBulbLumens(m_bulbLumensSpin->value());
    physical.setBulbColourTemperature(m_bulbTempCombo->currentText().toInt());
    physical.setWeight(m_weightSpin->value());
    physical.setWidth(m_widthSpin->value());
    physical.setHeight(m_heightSpin->value());
    physical.setDepth(m_depthSpin->value());
    physical.setLensName(m_lensNameCombo->currentText());
    physical.setLensDegreesMin(m_lensDegreesMinSpin->value());
    physical.setLensDegreesMax(m_lensDegreesMaxSpin->value());
    physical.setFocusType(m_focusTypeCombo->currentText());
    physical.setFocusPanMax(m_panMaxSpin->value());
    physical.setFocusTiltMax(m_tiltMaxSpin->value());
    physical.setPowerConsumption(m_powerConsumptionSpin->value());
    physical.setDmxConnector(m_dmxConnectorCombo->currentText());

    m_mode->setPhysical(physical);
    m_mode->setName(m_modeNameEdit->text());

    QDialog::accept();
}
