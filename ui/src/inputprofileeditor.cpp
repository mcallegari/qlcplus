/*
  Q Light Controller
  inputprofileeditor.cpp

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
#include <QColorDialog>
#include <QInputDialog>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QSettings>
#include <QCheckBox>
#include <QDialog>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QDir>

#include "qlcinputprofile.h"
#include "qlcchannel.h"

#include "inputchanneleditor.h"
#include "inputprofileeditor.h"
#include "inputoutputmap.h"
#include "apputil.h"

#define SETTINGS_GEOMETRY "inputprofileeditor/geometry"

#define KColumnNumber 0
#define KColumnName   1
#define KColumnType   2
#define KColumnValues 3

/****************************************************************************
 * Initialization
 ****************************************************************************/

InputProfileEditor::InputProfileEditor(QWidget* parent, QLCInputProfile* profile,
                                       InputOutputMap *ioMap)
    : QDialog(parent)
    , m_ioMap(ioMap)
    , m_wizardActive(false)
    , m_latestItem(NULL)
{
    Q_ASSERT(ioMap != NULL);

    setupUi(this);

    m_midiGroupSettings->setVisible(false);

    connect(m_typeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTypeComboChanged(int)));

    /* Connect the buttons to slots */
    connect(m_addButton, SIGNAL(clicked()),
            this, SLOT(slotAddClicked()));
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveClicked()));
    connect(m_editButton, SIGNAL(clicked()),
            this, SLOT(slotEditClicked()));
    connect(m_wizardButton, SIGNAL(clicked(bool)),
            this, SLOT(slotWizardClicked(bool)));
    connect(m_tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemClicked(QTreeWidgetItem*,int)));
    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditClicked()));
    connect(m_movementCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotMovementComboChanged(int)));
    connect(m_sensitivitySpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotSensitivitySpinChanged(int)));
    connect(m_extraPressCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotExtraPressChecked(bool)));
    connect(m_lowerSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotLowerValueSpinChanged(int)));
    connect(m_upperSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpperValueSpinChanged(int)));
    connect(m_midiChannelCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotMidiChannelComboChanged(int)));

    connect(m_addColorButton, SIGNAL(clicked()),
            this, SLOT(slotAddColor()));
    connect(m_removeColorButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveColor()));

    connect(m_addMidiChannelButton, SIGNAL(clicked()),
            this, SLOT(slotAddMidiChannel()));
    connect(m_removeMidiChannelButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveMidiChannel()));

    /* Listen to input data */
    connect(m_ioMap, SIGNAL(inputValueChanged(quint32, quint32, uchar, const QString&)),
            this, SLOT(slotInputValueChanged(quint32, quint32, uchar, const QString&)));

    if (profile == NULL)
    {
        m_profile = new QLCInputProfile();
    }
    else
    {
        m_profile = profile->createCopy();
        if ((QFile::permissions(m_profile->path()) &
                QFile::WriteUser) == 0)
        {
            QMessageBox::warning(this, tr("File not writable"),
                                 tr("You do not have permission to write to "
                                    "the file %1. You might not be able to "
                                    "save your modifications to the profile.")
                                 .arg(QDir::toNativeSeparators(
                                          m_profile->path())));
        }
    }

    QList<QLCInputProfile::Type> types = QLCInputProfile::types();
    for (int i = 0; i < types.size(); ++i)
    {
        const QLCInputProfile::Type type = types.at(i);
        m_typeCombo->addItem(QLCInputProfile::typeToString(type), type);
        if (m_profile->type() == type)
        {
            m_typeCombo->setCurrentIndex(i);
            if (type == QLCInputProfile::MIDI)
            {
                m_midiGroupSettings->setVisible(true);
                m_noteOffCheck->setChecked(m_profile->midiSendNoteOff());
            }
        }
    }

    /* Profile manufacturer & model */
    m_manufacturerEdit->setText(m_profile->manufacturer());
    m_modelEdit->setText(m_profile->model());

    m_behaviourBox->hide();
    m_feedbackGroup->hide();

    /* Fill up the tree with profile's channels */
    fillTree();

    /* Fill up the tree with color table */
    updateColorsTree();

    /* Fill up the tree with MIDI channel table */
    updateMidiChannelTree();

    /* Timer that clears the input data icon after a while */
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

InputProfileEditor::~InputProfileEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    delete m_profile;
}

void InputProfileEditor::fillTree()
{
    m_tree->clear();

    QMapIterator <quint32,QLCInputChannel*> it(m_profile->channels());
    while (it.hasNext() == true)
    {
        it.next();
        updateChannelItem(new QTreeWidgetItem(m_tree), it.value());
    }
    m_tree->header()->resizeSections(QHeaderView::ResizeToContents);
}

void InputProfileEditor::updateColorsTree()
{
    m_colorTableTree->clear();

    QMapIterator <uchar, QPair<QString, QColor>> it(m_profile->colorTable());
    while (it.hasNext() == true)
    {
        it.next();
        QPair<QString, QColor> lc = it.value();
        QTreeWidgetItem *item = new QTreeWidgetItem(m_colorTableTree);
        item->setText(0, QString::number(it.key()));
        item->setText(1, lc.first);

        QLabel *colLabel = new QLabel();
        colLabel->setStyleSheet(QString("background-color: %1").arg(lc.second.name()));

        m_colorTableTree->setItemWidget(item, 2, colLabel);
    }
}

void InputProfileEditor::updateMidiChannelTree()
{
    m_midiChannelsTree->clear();
    m_midiChannelCombo->clear();

    if (m_profile->hasMidiChannelTable())
    {
        m_midiChannelCombo->show();
        m_midiChannelLabel->show();
        m_midiChannelCombo->addItem(tr("From plugin settings"));
    }
    else
    {
        m_midiChannelCombo->hide();
        m_midiChannelLabel->hide();
    }

    QMapIterator <uchar, QString> it(m_profile->midiChannelTable());
    while (it.hasNext() == true)
    {
        it.next();
        QTreeWidgetItem *item = new QTreeWidgetItem(m_midiChannelsTree);
        item->setText(0, QString::number(it.key() + 1));
        item->setText(1, it.value());

        m_midiChannelCombo->addItem(it.value());
    }
}

void InputProfileEditor::updateChannelItem(QTreeWidgetItem *item,
                                           QLCInputChannel *ch)
{
    quint32 num;

    Q_ASSERT(item != NULL);
    Q_ASSERT(ch != NULL);

    num = m_profile->channelNumber(ch);
    item->setText(KColumnNumber, QString("%1").arg(num + 1, 4, 10, QChar('0')));
    item->setText(KColumnName, ch->name());
    item->setText(KColumnType, QLCInputChannel::typeToString(ch->type()));
    item->setIcon(KColumnType, ch->icon());
}

void InputProfileEditor::setOptionsVisibility(QLCInputChannel::Type type)
{
    bool showBox = true;
    bool showMovement = false;
    bool showSensitivity = false;
    bool showButtonOpts = false;

    if (type == QLCInputChannel::Slider || type == QLCInputChannel::Knob)
    {
        showMovement = true;
        showSensitivity = true;
        m_sensitivitySpin->setRange(10, 100);
    }
    else if (type == QLCInputChannel::Encoder)
    {
        showSensitivity = true;
        m_sensitivitySpin->setRange(1, 20);
    }
    else if (type == QLCInputChannel::Button)
    {
        showButtonOpts = true;
    }
    else
        showBox = false;

    m_movementLabel->setVisible(showMovement);
    m_movementCombo->setVisible(showMovement);
    m_sensitivityLabel->setVisible(showSensitivity);
    m_sensitivitySpin->setVisible(showSensitivity);
    m_extraPressCheck->setVisible(showButtonOpts);
    m_feedbackGroup->setVisible(showButtonOpts);
    m_behaviourBox->setVisible(showBox);
}

void InputProfileEditor::slotTypeComboChanged(int)
{
    bool showMidiSettings = false;

    if (currentProfileType() == QLCInputProfile::MIDI)
    {
        showMidiSettings = true;
        updateMidiChannelTree();
    }

    m_midiGroupSettings->setVisible(showMidiSettings);
}

/****************************************************************************
 * OK & Cancel
 ****************************************************************************/

void InputProfileEditor::reject()
{
    /* Don't allow closing the dialog in any way when the wizard is on */
    if (m_buttonBox->isEnabled() == false)
        return;

    QDialog::reject();
}

void InputProfileEditor::accept()
{
    /* Don't allow closing the dialog in any way when the wizard is on */
    if (m_buttonBox->isEnabled() == false)
        return;

    m_profile->setManufacturer(m_manufacturerEdit->text());
    m_profile->setModel(m_modelEdit->text());
    m_profile->setType(currentProfileType());

    if (currentProfileType() == QLCInputProfile::MIDI)
        m_profile->setMidiSendNoteOff(m_noteOffCheck->isChecked());

    /* Check that we have at least the bare necessities to save the profile */
    if (m_profile->manufacturer().isEmpty() == true ||
        m_profile->model().isEmpty() == true)
    {
        QMessageBox::warning(this, tr("Missing information"),
                             tr("Manufacturer and/or model name is missing."));
    }
    else
    {
        QDialog::accept();
    }
}

/****************************************************************************
 * Editing
 ****************************************************************************/

QList<QLCInputChannel *> InputProfileEditor::selectedChannels()
{
    QList<QLCInputChannel *> channels;

    QListIterator <QTreeWidgetItem*>it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem *item = it.next();
        Q_ASSERT(item != NULL);

        quint32 chnum = item->text(KColumnNumber).toUInt() - 1;
        QLCInputChannel *channel = m_profile->channel(chnum);
        Q_ASSERT(channel != NULL);

        channels.append(channel);
    }
    return channels;
}

void InputProfileEditor::slotAddClicked()
{
    QLCInputChannel* channel = new QLCInputChannel();
    InputChannelEditor ice(this, m_profile, channel, currentProfileType());
add:
    if (ice.exec() == QDialog::Accepted)
    {
        channel->setType(ice.type());
        channel->setName(ice.name());

        if (m_profile->channel(ice.channel()) == NULL)
        {
            m_profile->insertChannel(ice.channel(), channel);
            updateChannelItem(new QTreeWidgetItem(m_tree), channel);
        }
        else
        {
            QMessageBox::warning(this,
                                 tr("Channel already exists"),
                                 tr("Channel %1 already exists")
                                 .arg(ice.channel() + 1));
            goto add;
        }
    }
    else
    {
        delete channel;
    }
}

void InputProfileEditor::slotRemoveClicked()
{
    QList <QTreeWidgetItem*> selected;
    QTreeWidgetItem* next = NULL;

    /* Ask for confirmation if we're deleting more than one channel */
    selected = m_tree->selectedItems();
    if (selected.count() > 1)
    {
        int r;
        r = QMessageBox::question(this, tr("Delete channels"),
                                  tr("Delete all %1 selected channels?")
                                  .arg(selected.count()),
                                  QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::No)
            return;
    }

    /* Remove all selected channels */
    QMutableListIterator <QTreeWidgetItem*> it(selected);
    while (it.hasNext() == true)
    {
        QTreeWidgetItem *item = it.next();
        Q_ASSERT(item != NULL);

        /* Remove & Delete the channel object */
        quint32 chnum = item->text(KColumnNumber).toUInt() - 1;
        m_profile->removeChannel(chnum);

        /* Choose the closest item below or above the removed items
           as the one that is selected after the removal */
        next = m_tree->itemBelow(item);
        if (next == NULL)
            next = m_tree->itemAbove(item);

        delete item;
    }

    m_tree->setCurrentItem(next);
}

void InputProfileEditor::slotEditClicked()
{
    QLCInputChannel* channel;
    quint32 chnum;
    QTreeWidgetItem* item;

    if (m_tree->selectedItems().count() == 1)
    {
        /* Just one item selected. Edit that. */
        item = m_tree->currentItem();
        if (item == NULL)
            return;

        /* Find the channel object associated to the selected item */
        chnum = item->text(KColumnNumber).toUInt() - 1;
        channel = m_profile->channel(chnum);
        Q_ASSERT(channel != NULL);

        /* Edit the channel and update its item if necessary */
        InputChannelEditor ice(this, m_profile, channel, currentProfileType());
edit:
        if (ice.exec() == QDialog::Accepted)
        {
            QLCInputChannel* another;
            another = m_profile->channel(ice.channel());

            if (another == NULL || another == channel)
            {
                if (ice.channel() != QLCChannel::invalid())
                    m_profile->remapChannel(channel, ice.channel());
                if (ice.name().isEmpty() == false)
                    channel->setName(ice.name());
                if (ice.type() != QLCInputChannel::NoType)
                {
                    if (ice.type() != channel->type())
                        setOptionsVisibility(ice.type());
                    channel->setType(ice.type());
                    if (m_sensitivitySpin->isVisible())
                        m_sensitivitySpin->setValue(channel->movementSensitivity());
                }

                updateChannelItem(item, channel);
            }
            else
            {
                QMessageBox::warning(this,
                                     tr("Channel already exists"),
                                     tr("Channel %1 already exists")
                                     .arg(ice.channel() + 1));
                goto edit;
            }
        }
    }
    else if (m_tree->selectedItems().count() > 1)
    {
        /* Multiple channels selected. Apply changes to all of them */
        InputChannelEditor ice(this, NULL, NULL, QLCInputProfile::DMX);
        if (ice.exec() == QDialog::Accepted)
        {
            QListIterator <QTreeWidgetItem*>
            it(m_tree->selectedItems());
            while (it.hasNext() == true)
            {
                item = it.next();
                Q_ASSERT(item != NULL);

                chnum = item->text(KColumnNumber).toUInt() - 1;
                channel = m_profile->channel(chnum);
                Q_ASSERT(channel != NULL);

                /* Set only name and type and only if they
                   have been modified. */
                if (ice.name().isEmpty() == false)
                    channel->setName(ice.name());
                if (ice.type() != QLCInputChannel::NoType)
                    channel->setType(ice.type());

                updateChannelItem(item, channel);
            }
        }
    }
}

void InputProfileEditor::slotWizardClicked(bool checked)
{
    if (checked == true)
    {
        QMessageBox::information(this, tr("Channel wizard activated"),
                                 tr("You have enabled the input channel wizard. After "
                                    "clicking OK, wiggle your mapped input profile's "
                                    "controls. They should appear into the list. "
                                    "Click the wizard button again to stop channel "
                                    "auto-detection.\n\nNote that the wizard cannot "
                                    "tell the difference between a knob and a slider "
                                    "so you will have to do the change manually."));
        m_wizardActive = true;
    }
    else
    {
        m_wizardActive = false;
    }

    m_buttonBox->setEnabled(!checked);
    m_tab->setTabEnabled(0, !checked);
}

void InputProfileEditor::slotItemClicked(QTreeWidgetItem *item, int col)
{
    Q_UNUSED(col)

    quint32 chNum = item->text(KColumnNumber).toUInt() - 1;
    QLCInputChannel *ich = m_profile->channel(chNum);
    if (ich != NULL)
    {
        setOptionsVisibility(ich->type());

        if (ich->type() == QLCInputChannel::Slider || ich->type() == QLCInputChannel::Knob)
        {
            if (ich->movementType() == QLCInputChannel::Absolute)
            {
                m_movementCombo->setCurrentIndex(0);
                m_sensitivitySpin->setEnabled(false);
            }
            else
            {
                m_movementCombo->setCurrentIndex(1);
                m_sensitivitySpin->setValue(ich->movementSensitivity());
                m_sensitivitySpin->setEnabled(true);
            }
        }
        else if (ich->type() == QLCInputChannel::Encoder)
        {
            m_sensitivitySpin->setValue(ich->movementSensitivity());
            m_sensitivitySpin->setEnabled(true);
        }
        else if (ich->type() == QLCInputChannel::Button)
        {
            m_extraPressCheck->setChecked(ich->sendExtraPress());
            m_lowerSpin->blockSignals(true);
            m_upperSpin->blockSignals(true);
            m_midiChannelCombo->blockSignals(true);
            m_lowerSpin->setValue(ich->lowerValue());
            m_upperSpin->setValue(ich->upperValue());
            m_midiChannelCombo->setCurrentIndex(ich->lowerChannel() + 1);
            m_lowerSpin->blockSignals(false);
            m_upperSpin->blockSignals(false);
            m_midiChannelCombo->blockSignals(false);
        }
    }
    else
        setOptionsVisibility(QLCInputChannel::NoType);
}

void InputProfileEditor::slotMovementComboChanged(int index)
{
    if (index == 1)
        m_sensitivitySpin->setEnabled(true);
    else
        m_sensitivitySpin->setEnabled(false);

    foreach (QLCInputChannel *channel, selectedChannels())
    {
        if (channel->type() == QLCInputChannel::Slider ||
            channel->type() == QLCInputChannel::Knob)
        {
            if (index == 1)
                channel->setMovementType(QLCInputChannel::Relative);
            else
                channel->setMovementType(QLCInputChannel::Absolute);
        }
    }
}

void InputProfileEditor::slotSensitivitySpinChanged(int value)
{
    foreach (QLCInputChannel *channel, selectedChannels())
    {
        if ((channel->type() == QLCInputChannel::Slider ||
             channel->type() == QLCInputChannel::Knob) &&
            channel->movementType() == QLCInputChannel::Relative)
                channel->setMovementSensitivity(value);
        else if (channel->type() == QLCInputChannel::Encoder)
            channel->setMovementSensitivity(value);
    }
}

void InputProfileEditor::slotExtraPressChecked(bool checked)
{
    foreach (QLCInputChannel *channel, selectedChannels())
    {
        if (channel->type() == QLCInputChannel::Button)
            channel->setSendExtraPress(checked);
    }
}

void InputProfileEditor::slotLowerValueSpinChanged(int value)
{
    foreach (QLCInputChannel *channel, selectedChannels())
    {
        if (channel->type() == QLCInputChannel::Button)
            channel->setRange(uchar(value), uchar(m_upperSpin->value()));
    }
}

void InputProfileEditor::slotUpperValueSpinChanged(int value)
{
    foreach (QLCInputChannel *channel, selectedChannels())
    {
        if (channel->type() == QLCInputChannel::Button)
            channel->setRange(uchar(m_lowerSpin->value()), uchar(value));
    }
}

void InputProfileEditor::slotMidiChannelComboChanged(int index)
{
    foreach (QLCInputChannel *channel, selectedChannels())
    {
        if (channel->type() == QLCInputChannel::Button)
            channel->setLowerChannel(index - 1);
    }
}

void InputProfileEditor::slotAddColor()
{
    bool ok;
    int val = QInputDialog::getInt(this, tr("Enter value"), tr("Feedback value"), 0, 0, 255, 1, &ok);

    if (ok)
    {
        QColor color = QColorDialog::getColor();

        QString label = QInputDialog::getText(this, tr("Enter label"), tr("Color label"));
        m_profile->addColor(val, label, color);
        updateColorsTree();
        m_colorTableTree->scrollToBottom();
    }
}

void InputProfileEditor::slotRemoveColor()
{
    foreach (QTreeWidgetItem *item, m_colorTableTree->selectedItems())
    {
        uchar value = uchar(item->text(0).toInt());
        m_profile->removeColor(value);
    }
    updateColorsTree();
}

void InputProfileEditor::slotAddMidiChannel()
{
    bool ok;
    int val = QInputDialog::getInt(this, tr("Enter value"), tr("MIDI channel"), 1, 1, 16, 1, &ok);

    if (ok)
    {
        QString label = QInputDialog::getText(this, tr("Enter label"), tr("MIDI channel label"));
        m_profile->addMidiChannel(val - 1, label);
        updateMidiChannelTree();
    }
}

void InputProfileEditor::slotRemoveMidiChannel()
{
    foreach (QTreeWidgetItem *item, m_midiChannelsTree->selectedItems())
    {
        uchar value = uchar(item->text(0).toInt());
        m_profile->removeMidiChannel(value);
    }
    updateMidiChannelTree();
}

void InputProfileEditor::slotInputValueChanged(quint32 universe,
                                               quint32 channel,
                                               uchar value,
                                               const QString& key)
{
    QTreeWidgetItem* latestItem = NULL;

    Q_UNUSED(universe);

    /* Get a list of items that represent the given channel. Basically
       the list should always contain just one item. */
    QList <QTreeWidgetItem*> list;
    if (channel == UINT_MAX && key.isEmpty() == false)
        list = m_tree->findItems(key, Qt::MatchExactly, KColumnName);
    else
        list = m_tree->findItems(QString("%1").arg(channel + 1, 4, 10, QChar('0')), Qt::MatchExactly,
                             KColumnNumber);
    if (list.size() != 0)
        latestItem = list.first();

    if (list.size() == 0 && m_wizardActive == true)
    {
        /* No channel items found. Create a new channel to the
           profile and display it also in the tree widget */
        QLCInputChannel* ch = new QLCInputChannel();
        if (key.isEmpty())
            ch->setName(tr("Button %1").arg(channel + 1));
        else
            ch->setName(key);
        ch->setType(QLCInputChannel::Button);
        m_profile->insertChannel(channel, ch);

        latestItem = new QTreeWidgetItem(m_tree);
        updateChannelItem(latestItem, ch);
    }
    else if (m_wizardActive == true)
    {
        /* Existing channel & item found. Modify their contents. */
        latestItem = list.first();
        QVariant var = latestItem->data(KColumnValues, Qt::UserRole);
        QStringList values(var.toStringList());

        if (values.size() > 3)
        {
            /* No need to collect any more values, since this channel has
               been judged to be a slider when count == 3 (see below). */
        }
        else if (values.contains(QString("%1").arg(value)) == false)
        {
            values << QString("%1").arg(value);
            values.sort();
            latestItem->setData(KColumnValues, Qt::UserRole, values);
        }

        /* Change the channel type only the one time when its value
           count goes over 2. I.e. when a channel can have more than
           two distinct values, it can no longer be a button. */
        if (values.size() == 3)
        {
            QLCInputChannel* ch = m_profile->channel(channel);
            Q_ASSERT(ch != NULL);

            if (ch->type() == QLCInputChannel::Button)
            {
                ch->setType(QLCInputChannel::Slider);
                if (key.isEmpty())
                    ch->setName(tr("Slider %1").arg(channel + 1));
                else
                    ch->setName(key);
                updateChannelItem(latestItem, ch);
            }
        }
    }

    if (latestItem != NULL)
    {
        if (m_latestItem != NULL)
            m_latestItem->setIcon(KColumnNumber, QIcon());
        m_latestItem = latestItem;
        m_latestItem->setIcon(KColumnNumber, QIcon(":/input.png"));
        m_tree->scrollToItem(m_latestItem);
        m_timer->start(250);
    }
}

void InputProfileEditor::slotTimerTimeout()
{
    if (m_latestItem != NULL)
        m_latestItem->setIcon(KColumnNumber, QIcon());
    m_latestItem = NULL;
}

/****************************************************************************
 * Profile
 ****************************************************************************/

QLCInputProfile* InputProfileEditor::profile()
{
    return m_profile;
}

QLCInputProfile::Type InputProfileEditor::currentProfileType() const
{
    return static_cast<QLCInputProfile::Type>(m_typeCombo->itemData(m_typeCombo->currentIndex()).toInt());
}
