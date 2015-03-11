/*
  Q Light Controller - Fixture Definition Editor
  editchannel.cpp

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
#include <QRadioButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QTreeWidget>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QSettings>
#include <QPoint>
#include <QSize>

#include "qlccapability.h"
#include "qlcchannel.h"

#include "capabilitywizard.h"
#include "editcapability.h"
#include "editchannel.h"
#include "util.h"
#include "app.h"

#define SETTINGS_GEOMETRY "editchannel/geometry"
#define PROP_PTR Qt::UserRole

#define COL_MIN  0
#define COL_MAX  1
#define COL_NAME 2

EditChannel::EditChannel(QWidget* parent, QLCChannel* channel)
    : QDialog(parent)
{
    m_channel = new QLCChannel(channel);

    setupUi(this);
    init();

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
}

EditChannel::~EditChannel()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    if (m_channel != NULL)
        delete m_channel;
}

void EditChannel::init()
{
    Q_ASSERT(m_channel != NULL);

    /* Set window title */
    setWindowTitle(QString("Edit Channel: ") + m_channel->name());

    /* Set name edit */
    m_nameEdit->setText(m_channel->name());
    m_nameEdit->setValidator(CAPS_VALIDATOR(this));
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameChanged(const QString&)));

    /* Get available groups and insert them into the groups combo */
    m_groupCombo->addItems(QLCChannel::groupList());
    m_groupCombo->setIconSize(QSize(24, 24));
    for (int i = 0; i < m_groupCombo->count(); i++)
    {
        QLCChannel ch;
        ch.setGroup(QLCChannel::stringToGroup(m_groupCombo->itemText(i)));
        m_groupCombo->setItemIcon(i, ch.getIcon());
    }

    connect(m_groupCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotGroupActivated(const QString&)));
    connect(m_msbRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotMsbRadioToggled(bool)));
    connect(m_lsbRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotLsbRadioToggled(bool)));

    /* Select the channel's group */
    for (int i = 0; i < m_groupCombo->count(); i++)
    {
        if (m_groupCombo->itemText(i) == QLCChannel::groupToString(m_channel->group()))
        {
            m_groupCombo->setCurrentIndex(i);
            slotGroupActivated(QLCChannel::groupToString(m_channel->group()));
            break;
        }
    }

    /* Get available colours and insert them into the colour combo */
    m_colourCombo->addItems(QLCChannel::colourList());
    m_colourCombo->setIconSize(QSize(24, 24));
    for (int i = 0; i < m_colourCombo->count(); i++)
    {
        QLCChannel ch;
        ch.setName(m_colourCombo->itemText(i));
        ch.setGroup(QLCChannel::Intensity);
        ch.setColour(QLCChannel::stringToColour(m_colourCombo->itemText(i)));
        m_colourCombo->setItemIcon(i, ch.getIcon());
    }
    connect(m_colourCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotColourActivated(const QString&)));

    /* Select the channel's colour */
    for (int i = 0; i < m_colourCombo->count(); i++)
    {
        if (m_colourCombo->itemText(i) == QLCChannel::colourToString(m_channel->colour()))
        {
            m_colourCombo->setCurrentIndex(i);
            slotColourActivated(QLCChannel::colourToString(m_channel->colour()));
            break;
        }
    }

    connect(m_addCapabilityButton, SIGNAL(clicked()),
            this, SLOT(slotAddCapabilityClicked()));
    connect(m_removeCapabilityButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveCapabilityClicked()));
    connect(m_editCapabilityButton, SIGNAL(clicked()),
            this, SLOT(slotEditCapabilityClicked()));
    connect(m_wizardButton, SIGNAL(clicked()),
            this, SLOT(slotWizardClicked()));

    /* Capability list connections */
    connect(m_capabilityList,
            SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this,
            SLOT(slotCapabilityListSelectionChanged(QTreeWidgetItem*)));
    connect(m_capabilityList,
            SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this,
            SLOT(slotEditCapabilityClicked()));

    refreshCapabilities();
}

void EditChannel::slotNameChanged(const QString& name)
{
    m_channel->setName(name.simplified());
}

void EditChannel::slotGroupActivated(const QString& group)
{
    m_channel->setGroup(QLCChannel::stringToGroup(group));

    if (m_channel->controlByte() == 0)
        m_msbRadio->click();
    else
        m_lsbRadio->click();

    if (m_channel->group() == QLCChannel::Intensity)
    {
        m_colourLabel->show();
        m_colourCombo->show();
    }
    else
    {
        m_colourLabel->hide();
        m_colourCombo->hide();
    }
}

void EditChannel::slotMsbRadioToggled(bool toggled)
{
    if (toggled == true)
        m_channel->setControlByte(QLCChannel::MSB);
    else
        m_channel->setControlByte(QLCChannel::LSB);
}

void EditChannel::slotLsbRadioToggled(bool toggled)
{
    if (toggled == true)
        m_channel->setControlByte(QLCChannel::LSB);
    else
        m_channel->setControlByte(QLCChannel::MSB);
}

void EditChannel::slotColourActivated(const QString& colour)
{
    m_channel->setColour(QLCChannel::stringToColour(colour));
}

/****************************************************************************
 * Capability list functions
 ****************************************************************************/

void EditChannel::slotCapabilityListSelectionChanged(QTreeWidgetItem* item)
{
    if (item == NULL)
    {
        m_removeCapabilityButton->setEnabled(false);
        m_editCapabilityButton->setEnabled(false);
    }
    else
    {
        m_removeCapabilityButton->setEnabled(true);
        m_editCapabilityButton->setEnabled(true);
    }
}

void EditChannel::slotAddCapabilityClicked()
{
    EditCapability* ec = NULL;
    QLCCapability* cap = NULL;
    bool ok = false;
    uchar minFound = 0;

    foreach(QLCCapability *cap, m_channel->capabilities())
    {
        if (cap->max() > minFound)
            minFound = cap->max() + 1;
    }

    ec = new EditCapability(this, NULL, m_channel->group(), minFound);

    while (ok == false)
    {
        if (ec->exec() == QDialog::Accepted)
        {
            cap = new QLCCapability(ec->capability());

            if (m_channel->addCapability(cap) == false)
            {
                QMessageBox::warning(this,
                                     tr("Overlapping values"),
                                     tr("The capability's values overlap with another capability!"));
                delete cap;
                ok = false;
            }
            else
            {
                refreshCapabilities();
                ok = true;
            }
        }
        else
        {
            ok = true;
        }
    }

    delete ec;
}

void EditChannel::slotRemoveCapabilityClicked()
{
    QTreeWidgetItem* item;
    QTreeWidgetItem* next;

    item = m_capabilityList->currentItem();
    if (item == NULL)
        return;

    if (m_capabilityList->itemBelow(item) != NULL)
        next = m_capabilityList->itemBelow(item);
    else if (m_capabilityList->itemAbove(item) != NULL)
        next = m_capabilityList->itemAbove(item);
    else
        next = NULL;

    // This also deletes the capability
    m_channel->removeCapability(currentCapability());
    delete item;
    m_capabilityList->setCurrentItem(next);
}

void EditChannel::slotEditCapabilityClicked()
{
    EditCapability* ec = NULL;
    QLCCapability* real = NULL;
    QLCCapability* min = NULL;
    QLCCapability* max = NULL;
    bool ok = false;

    real = currentCapability();
    if (real == NULL)
        return;

    ec = new EditCapability(this, real, m_channel->group());

    while (ok == false)
    {
        if (ec->exec() == QDialog::Accepted)
        {
            min = m_channel->searchCapability(ec->capability()->min());
            max = m_channel->searchCapability(ec->capability()->max());
            if ((min != NULL && min != real) ||
                    (max != NULL && max != real))
            {
                QMessageBox::warning(this,
                                     tr("Overlapping values"),
                                     tr("The capability's values overlap with another capability!"));
                ok = false;
            }
            else
            {
                *real = *ec->capability();
                refreshCapabilities();
                ok = true;
            }
        }
        else
        {
            ok = true;
        }
    }

    delete ec;
}

void EditChannel::slotWizardClicked()
{
    bool overlap = false;

    CapabilityWizard cw(this, m_channel);
    if (cw.exec() == QDialog::Accepted)
    {
        QListIterator <QLCCapability*> it(cw.capabilities());
        while (it.hasNext() == true)
        {
            QLCCapability* cap;
            cap = new QLCCapability(it.next());
            if (m_channel->addCapability(cap) == false)
            {
                delete cap;
                overlap = true;
            }

            refreshCapabilities();
        }

        if (overlap == true)
        {
            QMessageBox::warning(this,
                                 tr("Overlapping values"),
                                 tr("Some capabilities could not be created "
                                    "because of overlapping values."));
        }
    }
}

void EditChannel::refreshCapabilities()
{
    m_channel->sortCapabilities();

    QListIterator <QLCCapability*> it(m_channel->capabilities());
    QLCCapability* cap = NULL;
    QTreeWidgetItem* item = NULL;
    QString str;

    m_capabilityList->clear();

    /* Fill capabilities */
    while (it.hasNext() == true)
    {
        cap = it.next();

        item = new QTreeWidgetItem(m_capabilityList);

        // Min
        str.sprintf("%.3d", cap->min());
        item->setText(COL_MIN, str);

        // Max
        str.sprintf("%.3d", cap->max());
        item->setText(COL_MAX, str);

        // Name
        item->setText(COL_NAME, cap->name());

        // Pointer
        item->setData(COL_NAME, PROP_PTR, (qulonglong) cap);
    }

    m_capabilityList->sortItems(COL_MIN, Qt::AscendingOrder);
    m_capabilityList->resizeColumnToContents(COL_MIN);
    m_capabilityList->resizeColumnToContents(COL_MAX);
    m_capabilityList->resizeColumnToContents(COL_NAME);

    slotCapabilityListSelectionChanged(m_capabilityList->currentItem());
}

QLCCapability* EditChannel::currentCapability()
{
    QTreeWidgetItem* item;
    QLCCapability* cap = NULL;

    // Convert the string-form ulong to a QLCChannel pointer and return it
    item = m_capabilityList->currentItem();
    if (item != NULL)
        cap = (QLCCapability*) item->data(COL_NAME, PROP_PTR).toULongLong();

    return cap;
}
