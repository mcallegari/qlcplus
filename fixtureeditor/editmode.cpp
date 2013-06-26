/*
  Q Light Controller - Fixture Editor
  editmode.cpp

  Copyright (C) Heikki Junnila

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
    m_channelList->header()->setResizeMode(QHeaderView::ResizeToContents);
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

    m_headList->header()->setResizeMode(QHeaderView::ResizeToContents);
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
    QLCChannel* ch;

    /* Create a list of channels that haven't been added to this mode yet */
    QStringList chlist;
    QListIterator <QLCChannel*> it(m_mode->fixtureDef()->channels());
    while (it.hasNext() == true)
    {
        ch = it.next();
        if (m_mode->channel(ch->name()) != NULL)
            continue;
        else
            chlist << ch->name();
    }

    if (chlist.size() > 0)
    {
        bool ok = false;
        QString name = QInputDialog::getItem(this,
                                             tr("Add channel to mode"),
                                             tr("Select a channel to add"),
                                             chlist, 0, false, &ok);

        if (ok == true && name.isEmpty() == false)
        {
            ch = m_mode->fixtureDef()->channel(name);

            // Append the channel
            m_mode->insertChannel(ch, m_mode->channels().size());

            // Easier to refresh the whole list
            refreshChannelList();

            // Select the new channel
            selectChannel(ch->name());
        }
    }
    else
    {
        QMessageBox::information(this, tr("No more available channels"),
                                 tr("All available channels are present in the mode."));
    }
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
        item->setIcon(COL_NAME, ch->getIconFromGroup(ch->group()));
        item->setData(COL_NAME, PROP_PTR, (qulonglong) ch);
    }
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

            summary += QString::number(chnum);
            if (it.hasNext() == true)
                summary += QString(", ");
        }

        item->setText(0, QString("Head %1 (%2)").arg(i + 1).arg(summary));
    }
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
