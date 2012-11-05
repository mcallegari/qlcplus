/*
  Q Light Controller
  selectinputchannel.cpp

  Copyright (c) Heikki Junnila

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
#include <QTreeWidget>
#include <QDebug>

#include "selectinputchannel.h"
#include "qlcinputchannel.h"
#include "qlcinputprofile.h"
#include "qlcioplugin.h"
#include "inputpatch.h"
#include "inputmap.h"

#define KColumnName     0
#define KColumnUniverse 1
#define KColumnChannel  2

/****************************************************************************
 * Initialization
 ****************************************************************************/

SelectInputChannel::SelectInputChannel(QWidget* parent, InputMap* inputMap)
    : QDialog(parent)
    , m_inputMap(inputMap)
{
    Q_ASSERT(inputMap != NULL);

    m_universe = InputMap::invalidUniverse();
    m_channel = InputMap::invalidChannel();

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    fillTree();
}

SelectInputChannel::~SelectInputChannel()
{
}

void SelectInputChannel::accept()
{
    QTreeWidgetItem* item;

    /* Extract data from the selected item */
    item = m_tree->currentItem();
    if (item != NULL)
    {
        m_universe = item->text(KColumnUniverse).toUInt();
        m_channel = item->text(KColumnChannel).toUInt();
    }

    QDialog::accept();
}

/****************************************************************************
 * Selection
 ****************************************************************************/

quint32 SelectInputChannel::universe() const
{
    return m_universe;
}

quint32 SelectInputChannel::channel() const
{
    return m_channel;
}

/****************************************************************************
 * Tree widget
 ****************************************************************************/

void SelectInputChannel::fillTree()
{
    QLCInputChannel* channel;
    QTreeWidgetItem* uniItem;
    QTreeWidgetItem* chItem;
    QLCInputProfile* profile;
    quint32 uni;
    InputPatch* patch;

    /* Add an option to select no input at all */
    chItem = new QTreeWidgetItem(m_tree);
    chItem->setText(KColumnName, KInputNone);
    chItem->setText(KColumnUniverse, QString("%1")
                    .arg(InputMap::invalidUniverse()));
    chItem->setText(KColumnChannel, QString("%1")
                    .arg(InputMap::invalidChannel()));

    for (uni = 0; uni < m_inputMap->universes(); uni++)
    {
        /* Get the patch associated to the current universe */
        patch = m_inputMap->patch(uni);
        Q_ASSERT(patch != NULL);

        /* Make an item for each universe */
        uniItem = new QTreeWidgetItem(m_tree);
        updateUniverseItem(uniItem, uni, patch);

        if (patch->plugin() != NULL && patch->input() != QLCIOPlugin::invalidLine())
        {
            /* Add a manual option to each patched universe */
            chItem = new QTreeWidgetItem(uniItem);
            updateChannelItem(chItem, uni, NULL, NULL);
        }

        /* Add known channels from profile (if any) */
        profile = patch->profile();
        if (profile != NULL)
        {
            QMapIterator <quint32, QLCInputChannel*>
            it(profile->channels());
            while (it.hasNext() == true)
            {
                channel = it.next().value();
                Q_ASSERT(channel != NULL);

                chItem = new QTreeWidgetItem(uniItem);
                updateChannelItem(chItem, uni, channel,
                                  profile);
            }
        }
    }

    /* Listen to item changed signals so that we can catch user's
       manual input for <...> nodes. Connect AFTER filling the tree
       so all the initial item->setText()'s won't get caught here. */
    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

void SelectInputChannel::updateChannelItem(QTreeWidgetItem* item,
        quint32 universe,
        const QLCInputChannel* channel,
        const QLCInputProfile* profile)
{
    Q_ASSERT(item != NULL);

    /* Add a manual option to each universe */
    item->setText(KColumnUniverse, QString("%1").arg(universe));
    if (channel == NULL && profile == NULL)
    {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(KColumnName,
                      tr("<Double click here to enter channel number manually>"));
        item->setText(KColumnChannel,
                      QString("%1").arg(InputMap::invalidChannel()));
    }
    else
    {
        item->setText(KColumnName, QString("%1: %2")
                      .arg(profile->channelNumber(channel) + 1)
                      .arg(channel->name()));
        item->setText(KColumnChannel, QString("%1")
                      .arg(profile->channelNumber(channel)));

        /* Display nice icons to indicate channel type */
        if (channel->type() == QLCInputChannel::Slider)
            item->setIcon(KColumnName, QIcon(":/slider.png"));
        else if (channel->type() == QLCInputChannel::Knob)
            item->setIcon(KColumnName, QIcon(":/knob.png"));
        else if (channel->type() == QLCInputChannel::Button)
            item->setIcon(KColumnName, QIcon(":/button.png"));
    }
}

void SelectInputChannel::updateUniverseItem(QTreeWidgetItem* item,
        quint32 universe,
        InputPatch* patch)
{
    QString name;

    Q_ASSERT(item != NULL);

    if (patch == NULL || patch->plugin() == NULL)
    {
        /* The current universe doesn't have anything assigned to it */
        name = QString("%1: %2").arg(universe + 1).arg(KInputNone);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }
    else
    {
        /* The current universe has something assigned to it. Check,
           whether it has an input profile. */
        if (patch->profile() != NULL)
        {
            name = QString("%1: %2").arg(universe + 1)
                   .arg(patch->profileName());
        }
        else
        {
            name = QString("%1: %2 / %3").arg(universe + 1)
                   .arg(patch->pluginName())
                   .arg(patch->inputName());
        }
    }

    item->setText(KColumnName, name);
    item->setText(KColumnUniverse, QString("%1").arg(universe));
    item->setText(KColumnChannel, QString("%1").arg(InputMap::invalidChannel()));
}

void SelectInputChannel::slotItemChanged(QTreeWidgetItem* item, int column)
{
    quint32 channel;

    Q_ASSERT(item != NULL);
    if (column != KColumnName)
        return;

    /* Extract only numbers from the input data */
    channel = item->text(KColumnName).toUInt();

    /* Put the entered channel number also to the channel column */
    item->setText(KColumnChannel, QString("%1").arg(channel - 1));
}

void SelectInputChannel::slotItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);

    if (!(item->flags() & Qt::ItemIsEditable))
        accept();
}
