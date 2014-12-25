/*
  Q Light Controller
  addfixture.cpp

  Copyright (c) Heikki Junnila

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

#include <QDialogButtonBox>
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QHeaderView>
#include <QByteArray>
#include <QSettings>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QDebug>

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"

#include "addresstool.h"
#include "outputpatch.h"
#include "addfixture.h"
#include "apputil.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "addfixture/geometry"

#define KColumnName 0

AddFixture::AddFixture(QWidget* parent, const Doc* doc, const Fixture* fxi)
    : QDialog(parent)
    , m_doc(doc)
{
    m_addressValue = 0;
    m_universeValue = 0;
    m_amountValue = 1;
    m_gapValue = 0;
    m_channelsValue = 1;
    m_fixtureDef = NULL;
    m_mode = NULL;
    m_fxiCount = 0;
    m_fixtureID = Fixture::invalidId();
    m_invalidAddressFlag = false;

    setupUi(this);
    m_addrErrorLabel->hide();

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));
    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotTreeDoubleClicked(QTreeWidgetItem*)));
    connect(m_modeCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotModeActivated(const QString&)));
    connect(m_universeCombo, SIGNAL(activated(int)),
            this, SLOT(slotUniverseActivated(int)));
    connect(m_addressSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotAddressChanged(int)));
    connect(m_channelsSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotChannelsChanged(int)));
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_gapSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotGapSpinChanged(int)));
    connect(m_amountSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotAmountSpinChanged(int)));
    connect(m_searchEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotSearchFilterChanged(QString)));
    connect(m_diptoolButton, SIGNAL(clicked()),
            this, SLOT(slotDiptoolButtonClicked()));

    /* Fill fixture definition tree (and select a fixture def) */
    if (fxi != NULL && fxi->isDimmer() == false)
        fillTree(fxi->fixtureDef()->manufacturer(), fxi->fixtureDef()->model());
    else
        fillTree(KXMLFixtureGeneric, KXMLFixtureGeneric);

    m_fixturesCount->setText(tr("Fixtures found: %1").arg(m_fxiCount));

    /* Fill universe combo with available universes */
    m_universeCombo->addItems(m_doc->inputOutputMap()->universeNames());

    /* Simulate first selection and find the next free address */
    slotSelectionChanged();

    // Universe
    if (fxi != NULL)
    {
        m_fixtureID = fxi->id();
        m_universeCombo->setCurrentIndex(fxi->universe());
        slotUniverseActivated(fxi->universe());

        m_addressSpin->setValue(fxi->address() + 1);
        m_addressValue = fxi->address();

        m_multipleGroup->setEnabled(false);
    }
    else
    {
        slotUniverseActivated(0);
        findAddress();
    }

    // Name
    if (fxi != NULL)
    {
        m_nameEdit->setText(fxi->name());
        slotNameEdited(fxi->name());
        m_nameEdit->setModified(true); // Prevent auto-naming
    }

    // Mode
    if (fxi != NULL)
    {
        if (fxi->isDimmer() == false)
        {
            int index = m_modeCombo->findText(fxi->fixtureMode()->name());
            if (index != -1)
            {
                m_modeCombo->setCurrentIndex(index);
                slotModeActivated(m_modeCombo->itemText(index));
            }
        }
        else
        {
            m_channelsSpin->setValue(fxi->channels());
        }
    }
    else
    {
        m_channelsSpin->setValue(1);
    }

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

AddFixture::~AddFixture()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
   
    QList<QVariant> expanded;
    QTreeWidgetItem * root = m_tree->invisibleRootItem();

    for (int i=0; i < root->childCount(); i++)
    {
        QTreeWidgetItem * manuf = root->child(i);
        if (manuf->isExpanded())
        {
            expanded << manuf->text(KColumnName);
        }
    }

    settings.setValue(SETTINGS_EXPANDED, expanded);
}

/*****************************************************************************
 * Value getters
 *****************************************************************************/

QLCFixtureDef* AddFixture::fixtureDef() const
{
    return m_fixtureDef;
}

QLCFixtureMode *AddFixture::mode() const
{
    return m_mode;
}

QString AddFixture::name() const
{
    return m_nameValue;
}

quint32 AddFixture::address() const
{
    return m_addressValue;
}

quint32 AddFixture::universe() const
{
    return m_universeValue;
}

int AddFixture::amount() const
{
    return m_amountValue;
}

quint32 AddFixture::gap() const
{
    return m_gapValue;
}

quint32 AddFixture::channels() const
{
    return m_channelsValue;
}

bool AddFixture::invalidAddress()
{
    return m_invalidAddressFlag;
}

/*****************************************************************************
 * Fillers
 *****************************************************************************/

void AddFixture::fillTree(const QString& selectManufacturer,
                          const QString& selectModel)
{
    QTreeWidgetItem* parent = NULL;
    QTreeWidgetItem* child;
    QString manuf;
    QString model;
    QList<QVariant> expanded;

    QSettings settings;
    QVariant var = settings.value(SETTINGS_EXPANDED);
    if (var.isValid() == true)
    {
        expanded = var.toList();
    }

    /* Clear the tree of any previous data */
    m_tree->clear();

    QString filter = m_searchEdit->text().toLower();

    /* Add all known fixture definitions to the tree */
    QStringListIterator it(m_doc->fixtureDefCache()->manufacturers());
    while (it.hasNext() == true)
    {
        bool manufAdded = false;

        manuf = it.next();
        if (manuf == KXMLFixtureGeneric)
            continue;

        QStringListIterator modit(m_doc->fixtureDefCache()->models(manuf));
        while (modit.hasNext() == true)
        {
            model = modit.next();

            if (filter.isEmpty() == false &&
                manuf.toLower().contains(filter) == false &&
                model.toLower().contains(filter) == false)
                    continue;

            if (manufAdded == false)
            {
                parent = new QTreeWidgetItem(m_tree);
                parent->setText(KColumnName, manuf);
                manufAdded = true;
            }
            child = new QTreeWidgetItem(parent);
            child->setText(KColumnName, model);

            if (manuf == selectManufacturer &&
                    model == selectModel)
            {
                parent->setExpanded(true);
                m_tree->setCurrentItem(child);
            }
            else if(expanded.indexOf(manuf) != -1)
            {
                parent->setExpanded(true);
            }
            m_fxiCount++;
        }
    }

    /* Sort the tree A-Z BEFORE appending a generic entries */
    m_tree->sortItems(0, Qt::AscendingOrder);

    /* Create a parent for the generic devices */
    parent = new QTreeWidgetItem(m_tree);
    parent->setText(KColumnName, KXMLFixtureGeneric);
    QStringListIterator modit(m_doc->fixtureDefCache()->models(KXMLFixtureGeneric));
    while (modit.hasNext() == true)
    {
        model = modit.next();
        child = new QTreeWidgetItem(parent);
        child->setText(KColumnName, model);

        if (selectManufacturer == KXMLFixtureGeneric &&
                model == selectModel)
        {
            parent->setExpanded(true);
            m_tree->setCurrentItem(child);
        }
        else if(expanded.indexOf(manuf) != -1)
        {
            parent->setExpanded(true);
        }
        m_fxiCount++;
    }

    /* Create a child for generic dimmer device */
    child = new QTreeWidgetItem(parent);
    child->setText(KColumnName, KXMLFixtureGeneric);

    parent->sortChildren(0, Qt::AscendingOrder);

    /* Select generic dimmer by default */
    if (selectManufacturer == KXMLFixtureGeneric &&
            selectModel == KXMLFixtureGeneric)
    {
        parent->setExpanded(true);
        m_tree->setCurrentItem(child);
    }
}

void AddFixture::fillModeCombo(const QString& text)
{
    m_modeCombo->clear();

    if (m_fixtureDef == NULL)
    {
        m_modeCombo->setEnabled(false);
        m_modeCombo->addItem(text);
        m_modeCombo->setCurrentIndex(0);
        m_mode = NULL;
    }
    else
    {
        m_modeCombo->setEnabled(true);

        QListIterator <QLCFixtureMode*> it(m_fixtureDef->modes());
        while (it.hasNext() == true)
            m_modeCombo->addItem(it.next()->name());

        /* Select the first mode by default */
        m_modeCombo->setCurrentIndex(0);
        slotModeActivated(m_modeCombo->currentText());
    }
}

void AddFixture::findAddress()
{
    /* Find the next free address space for x fixtures, each taking y
       channels, leaving z channels gap in-between. */
    quint32 address = findAddress((m_channelsValue + m_gapValue) * m_amountValue,
                                  m_doc->fixtures(),
                                  m_doc->inputOutputMap()->universes());

    /* Set the address only if the channel space was really found */
    if (address != QLCChannel::invalid())
    {
        m_universeCombo->setCurrentIndex(address >> 9);
        m_addressSpin->setValue((address & 0x01FF) + 1);
    }
}

quint32 AddFixture::findAddress(quint32 numChannels,
                                const QList <Fixture*> fixtures,
                                quint32 maxUniverses)
{
    /* Try to find contiguous space from one universe at a time */
    for (quint32 universe = 0; universe < maxUniverses; universe++)
    {
        quint32 ch = findAddress(universe, numChannels, fixtures);
        if (ch != QLCChannel::invalid())
            return ch;
    }

    return QLCChannel::invalid();
}

quint32 AddFixture::findAddress(quint32 universe, quint32 numChannels,
                                const QList <Fixture*> fixtures)
{
    quint32 freeSpace = 0;
    quint32 maxChannels = 512;

    /* Construct a map of unallocated channels */
    int map[maxChannels];
    std::fill(map, map + maxChannels, 0);

    QListIterator <Fixture*> fxit(fixtures);
    while (fxit.hasNext() == true)
    {
        Fixture* fxi(fxit.next());
        Q_ASSERT(fxi != NULL);

        if (fxi->universe() != universe)
            continue;

        for (quint32 ch = 0; ch < fxi->channels(); ch++)
            map[(fxi->universeAddress() & 0x01FF) + ch] = 1;
    }

    /* Try to find the next contiguous free address space */
    for (quint32 addr = 0; addr < maxChannels; addr++)
    {
        if (map[addr] == 0)
            freeSpace++;
        else
            freeSpace = 0;

        if (freeSpace == numChannels)
            return (addr - freeSpace + 1) | (universe << 9);
    }

    return QLCChannel::invalid();
}

void AddFixture::updateMaximumAmount()
{
    m_amountSpin->setRange(1, (512 - m_addressSpin->value()) /
                           (m_channelsSpin->value() + m_gapSpin->value()));
}

bool AddFixture::checkAddressAvailability(int value, int channels)
{
    qDebug() << "Check availability for address: " << value;
    for (int i = 0; i < channels; i++)
    {
        quint32 fid = m_doc->fixtureForAddress(value + i);
        if (fid != Fixture::invalidId() && fid != m_fixtureID)
            return false;
    }
    return true;
}

/*****************************************************************************
 * Slots
 *****************************************************************************/

void AddFixture::slotModeActivated(const QString& modeName)
{
    if (m_fixtureDef == NULL)
        return;

    m_mode = m_fixtureDef->mode(modeName);
    if (m_mode == NULL)
    {
        /* Generic dimmers don't have modes, so bail out */
        // slotSelectionChanged();
        return;
    }

    m_channelsSpin->setValue(m_mode->channels().size());

    /* Show all selected mode channels in the list */
    m_channelList->clear();
    for (int i = 0; i < m_mode->channels().size(); i++)
    {
        QLCChannel* channel = m_mode->channel(i);
        Q_ASSERT(channel != NULL);

        new QListWidgetItem(
            QString("%1: %2").arg(i + 1).arg(channel->name()),
            m_channelList);
    }

    int absAddress = ((m_addressSpin->value() - 1) & 0x01FF) | (m_universeValue << 9);
    if (checkAddressAvailability(absAddress, m_channelsSpin->value()) == false)
    {
        // turn the new address to red
        m_addrErrorLabel->show();
        m_invalidAddressFlag = true;
    }
    else
    {
        m_addrErrorLabel->hide();
        m_invalidAddressFlag = false;
    }
}

void AddFixture::slotUniverseActivated(int universe)
{
    m_universeValue = universe;

    /* Adjust the available address range */
    slotChannelsChanged(m_channelsValue);

    quint32 addr = findAddress(universe, m_channelsSpin->value(), m_doc->fixtures());
    if (addr != QLCChannel::invalid())
        m_addressSpin->setValue((addr & 0x01FF) + 1);
    else
        m_addressSpin->setValue(1);
}

void AddFixture::slotAddressChanged(int value)
{
    int absAddress = ((value - 1) & 0x01FF) | (m_universeValue << 9);
    if (checkAddressAvailability(absAddress, m_channelsSpin->value()) == false)
    {
        // turn the new address to red
        m_addrErrorLabel->show();
        m_invalidAddressFlag = true;
        return;
    }
    else
    {
        m_addrErrorLabel->hide();
        m_invalidAddressFlag = false;
    }

    m_addressValue = value - 1;

    /* Set the maximum number of fixtures */
    updateMaximumAmount();
}

void AddFixture::slotChannelsChanged(int value)
{
    m_channelsValue = value;

    /* Set the maximum possible address so that channels cannot overflow
       beyond DMX's range of 512 channels */
    m_addressSpin->setRange(1, 513 - value);

    /* Set the maximum number of fixtures */
    updateMaximumAmount();
}

void AddFixture::slotNameEdited(const QString &text)
{
    /* If the user clears the text in the name field,
       start substituting the name with the model again. */
    if (text.length() == 0)
        m_nameEdit->setModified(false);
    else
        m_nameEdit->setModified(true);
    m_nameValue = text;
}

void AddFixture::slotAmountSpinChanged(int value)
{
    m_amountValue = value;
}

void AddFixture::slotGapSpinChanged(int value)
{
    m_gapValue = value;

    /* Set the maximum number of fixtures */
    updateMaximumAmount();
}

void AddFixture::slotSearchFilterChanged(QString)
{
    m_tree->blockSignals(true);
    fillTree("", "");
    m_tree->blockSignals(false);
}

void AddFixture::slotSelectionChanged()
{
    /* If there is no valid selection (user has selected only a
       manufacturer or nothing at all) don't let him press OK. */
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL || item->parent() == NULL)
    {
        /* Reset the selected fixture pointer */
        m_fixtureDef = NULL;

        /* Since there is no m_fixtureDef, mode combo is cleared */
        fillModeCombo();

        /* Clear the name box unless it has been modified by user */
        if (m_nameEdit->isModified() == false)
        {
            m_nameEdit->setText(QString());
            slotNameEdited(QString());
            m_nameEdit->setModified(false);
        }
        m_nameEdit->setEnabled(false);

        m_channelsSpin->setValue(0);
        m_channelList->clear();
        m_addressSpin->setEnabled(false);
        m_universeCombo->setEnabled(false);

        m_multipleGroup->setEnabled(false);
        m_amountSpin->setEnabled(false);
        m_gapSpin->setEnabled(false);
        m_channelsSpin->setEnabled(false);

        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

        return;
    }

    /* Item & its parent should be valid here */
    QString manuf(item->parent()->text(KColumnName));
    QString model(item->text(KColumnName));
    if (manuf == KXMLFixtureGeneric && model == KXMLFixtureGeneric)
    {
        /* Generic dimmer selected. User enters number of channels. */
        m_fixtureDef = NULL;
        m_mode = NULL;
        fillModeCombo(KXMLFixtureGeneric);
        m_channelsSpin->setEnabled(true);
        m_channelList->clear();

        /* Set the model name as the fixture's friendly name ONLY
           if the user hasn't modified the friendly name field. */
        if (m_nameEdit->isModified() == false)
        {
            m_nameEdit->setText(tr("Dimmers"));
            slotNameEdited(m_nameEdit->text());
            m_nameEdit->setModified(false);
        }
        m_nameEdit->setEnabled(true);
    }
    else
    {
        /* Specific fixture definition selected. */
        m_fixtureDef = m_doc->fixtureDefCache()->fixtureDef(manuf, model);
        Q_ASSERT(m_fixtureDef != NULL);

        /* Put fixture def's modes to the mode combo */
        fillModeCombo();

        /* Fixture def contains number of channels, so disable the
           spin box to prevent user from modifying it. */
        m_channelsSpin->setEnabled(false);

        /* Set the model name as the fixture's friendly name ONLY
           if the user hasn't modified the friendly name field. */
        if (m_nameEdit->isModified() == false)
        {
            m_nameEdit->setText(m_fixtureDef->model());
            slotNameEdited(m_nameEdit->text());
            m_nameEdit->setModified(false);
        }
        m_nameEdit->setEnabled(true);
    }

    /* Set the maximum number of fixtures */
    updateMaximumAmount();

    /* Guide the user to edit the friendly name field */
    m_nameEdit->setSelection(0, m_nameEdit->text().length());
    m_nameEdit->setFocus();

    m_addressSpin->setEnabled(true);
    m_universeCombo->setEnabled(true);

    m_multipleGroup->setEnabled(true);
    m_amountSpin->setEnabled(true);
    m_gapSpin->setEnabled(true);

    /* Recalculate the first available address for the newly selected fixture */
    quint32 addr = findAddress(m_universeValue, m_channelsSpin->value(), m_doc->fixtures());
    if (addr != QLCChannel::invalid())
        m_addressSpin->setValue((addr & 0x01FF) + 1);
    else
        m_addressSpin->setValue(1);

    /* OK is again possible */
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel);
}

void AddFixture::slotTreeDoubleClicked(QTreeWidgetItem* item)
{
    /* Select and accept (click OK for the user) */
    slotSelectionChanged();
    if (item != NULL && item->parent() != NULL)
        accept();
}

void AddFixture::slotDiptoolButtonClicked()
{
    AddressTool at(this, m_addressSpin->value());
    at.exec();
    m_addressSpin->setValue(at.getAddress());
}
