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
#include <QColorDialog>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QTreeWidget>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QSettings>
#include <QPainter>
#include <QPoint>
#include <QSize>

#include "qlccapability.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#include "capabilitywizard.h"
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
    m_currentCapability = NULL;

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
    setWindowTitle(tr("Edit Channel: ") + m_channel->name());

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
    connect(m_wizardButton, SIGNAL(clicked()),
            this, SLOT(slotWizardClicked()));

    /* Capability list connections */
    connect(m_capabilityList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotCapabilityListSelectionChanged(QTreeWidgetItem*)));
    connect(m_capabilityList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotEditCapabilityClicked()));

    connect(m_minSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotMinSpinChanged(int)));
    connect(m_maxSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotMaxSpinChanged(int)));
    connect(m_descriptionEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotDescriptionEdited(const QString&)));
    connect(m_pictureButton, SIGNAL(pressed()),
            this, SLOT(slotPictureButtonPressed()));
    connect(m_color1Button, SIGNAL(pressed()),
            this, SLOT(slotColor1ButtonPressed()));
    connect(m_color2Button, SIGNAL(pressed()),
            this, SLOT(slotColor2ButtonPressed()));

    refreshCapabilities();
    m_valueGroup->setVisible(false);
    m_resourceGroup->setVisible(false);
}

void EditChannel::setupCapabilityGroup()
{
    m_valueGroup->setVisible(true);
    m_resourceGroup->setVisible(true);
    m_invalidMinMax->setVisible(false);

    if (m_channel->group() == QLCChannel::Gobo)
        m_resourceGroup->setTitle(tr("Gobo"));
    else if (m_channel->group() == QLCChannel::Colour)
        m_resourceGroup->setTitle(tr("Colour"));
    else if (m_channel->group() == QLCChannel::Effect)
        m_resourceGroup->setTitle(tr("Effect"));
    else
        m_resourceGroup->hide();

    // temporarily block signals to avoid a full tree refresh
    m_minSpin->blockSignals(true);
    m_maxSpin->blockSignals(true);
    m_descriptionEdit->blockSignals(true);

    m_minSpin->setRange(0, m_currentCapability->max());
    m_maxSpin->setRange(m_currentCapability->min(), 255);

    m_minSpin->setValue(m_currentCapability->min());
    m_maxSpin->setValue(m_currentCapability->max());

    m_descriptionEdit->setText(m_currentCapability->name());
    m_descriptionEdit->setValidator(CAPS_VALIDATOR(this));
    m_minSpin->setFocus();
    m_minSpin->selectAll();

    m_minSpin->blockSignals(false);
    m_maxSpin->blockSignals(false);
    m_descriptionEdit->blockSignals(false);

    if (m_currentCapability->resourceName().isEmpty() == false)
        m_resourceButton->setIcon(QIcon(m_currentCapability->resourceName()));
    else if (m_currentCapability->resourceColor1().isValid())
    {
        QPixmap pix(58, 58);
        if (m_currentCapability->resourceColor2().isValid())
        {
            QPainter painter(&pix);
            painter.fillRect(0, 0, 29, 58, m_currentCapability->resourceColor1());
            painter.fillRect(29, 0, 58, 58, m_currentCapability->resourceColor2());
        }
        else
            pix.fill(m_currentCapability->resourceColor1());
        m_color2Button->setEnabled(true);
        m_resourceButton->setIcon(pix);
    }
    else
    {
        QPixmap pix(58, 58);
        m_color2Button->setEnabled(false);
        pix.fill(QColor(Qt::transparent));
        m_resourceButton->setIcon(pix);
    }
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
        m_removeCapabilityButton->setEnabled(false);
    else
        m_removeCapabilityButton->setEnabled(true);
}

void EditChannel::slotAddCapabilityClicked()
{
    uchar minFound = 0;
    uchar maxFound = UCHAR_MAX;
    int idx = 0;

    foreach(QLCCapability *cap, m_channel->capabilities())
    {
        if (cap->min() > minFound + 1)
        {
            maxFound = cap->min() - 1;
            break;
        }
        if (cap->max() > minFound || cap->min() == cap->max())
            minFound = cap->max() + 1;
        idx++;
    }

    QLCCapability* newCapability = new QLCCapability();
    newCapability->setMin(minFound);
    newCapability->setMax(maxFound);
    if (m_channel->addCapability(newCapability) == false)
    {
        delete newCapability;
        return;
    }

    m_currentCapability = newCapability;
    refreshCapabilities();
    m_capabilityList->setCurrentItem(m_capabilityList->topLevelItem(idx));
    setupCapabilityGroup();
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
    m_currentCapability = currentCapability();
    if (m_currentCapability != NULL)
        setupCapabilityGroup();
    else
    {
        m_valueGroup->setVisible(false);
        m_resourceGroup->setVisible(false);
    }
}

void EditChannel::slotEditCapabilityClicked()
{
    m_currentCapability = currentCapability();
    if (m_currentCapability == NULL)
    {
        m_valueGroup->setVisible(false);
        m_resourceGroup->setVisible(false);
        return;
    }

    setupCapabilityGroup();
}

void EditChannel::slotWizardClicked()
{
    CapabilityWizard cw(this, m_channel);

    if (cw.exec() == QDialog::Accepted)
    {
        bool overlap = false;
        QListIterator <QLCCapability*> it(cw.capabilities());
        while (it.hasNext() == true)
        {
            QLCCapability* cap = it.next()->createCopy();
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

void EditChannel::slotMinSpinChanged(int value)
{
    if (!m_channel->setCapabilityRange(m_currentCapability, value, m_maxSpin->value()))
    {
        m_invalidMinMax->setVisible(true);
        return;
    }
    m_invalidMinMax->setVisible(false);

    m_maxSpin->setRange(m_currentCapability->min(), 255);

    QTreeWidgetItem *item = m_capabilityList->currentItem();
    if (item != NULL)
    {
        QString str;
        str.sprintf("%.3d", value);
        item->setText(COL_MIN, str);
    }
}

void EditChannel::slotMaxSpinChanged(int value)
{
    if (!m_channel->setCapabilityRange(m_currentCapability, m_minSpin->value(), value))
    {
        m_invalidMinMax->setVisible(true);
        return;
    }
    m_invalidMinMax->setVisible(false);

    m_minSpin->setRange(0, m_currentCapability->max());

    QTreeWidgetItem *item = m_capabilityList->currentItem();
    if (item != NULL)
    {
        QString str;
        str.sprintf("%.3d", value);
        item->setText(COL_MAX, str);
    }
}

void EditChannel::slotDescriptionEdited(const QString& text)
{
    m_currentCapability->setName(text);

    QTreeWidgetItem *item = m_capabilityList->currentItem();
    if (item != NULL)
        item->setText(COL_NAME, text);
}

void EditChannel::slotPictureButtonPressed()
{
    QFileDialog dialog(this);
    QDir dir = QLCFile::systemDirectory(GOBODIR);
    dialog.setWindowTitle(tr("Open Gobo File"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory(dir);
    dialog.setNameFilter((tr("Gobo pictures") + " (*.jpg *.jpeg *.png *.bmp)"));

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString filename = dialog.selectedFiles().first();
    if (filename.isEmpty() == true)
        return;

    m_resourceButton->setIcon(QIcon(filename));
    m_currentCapability->setResourceName(filename);
}

void EditChannel::slotColor1ButtonPressed()
{
    QColorDialog dialog(this);
    if (m_currentCapability->resourceColor1().isValid())
        dialog.setCurrentColor(m_currentCapability->resourceColor1());
    if (dialog.exec() != QDialog::Accepted)
        return;

    QColor color = dialog.selectedColor();
    QPixmap pix(58, 58);
    pix.fill(color);
    m_resourceButton->setIcon(pix);
    m_currentCapability->setResourceColors(color, QColor());
    m_color2Button->setEnabled(true);
}

void EditChannel::slotColor2ButtonPressed()
{
    QColorDialog dialog(this);
    if (m_currentCapability->resourceColor2().isValid())
        dialog.setCurrentColor(m_currentCapability->resourceColor2());
    if (dialog.exec() != QDialog::Accepted)
        return;

    QColor color = dialog.selectedColor();
    QColor firstColor = m_currentCapability->resourceColor1();
    QPixmap pix(58, 58);
    QPainter painter(&pix);
    painter.fillRect(0, 0, 29, 58, firstColor);
    painter.fillRect(29, 0, 58, 58, color);

    m_resourceButton->setIcon(pix);
    m_currentCapability->setResourceColors(firstColor, color);
}


void EditChannel::refreshCapabilities()
{
    m_channel->sortCapabilities();

    QListIterator <QLCCapability*> it(m_channel->capabilities());
    QString str;

    m_capabilityList->clear();

    QStringList goboErrors;

    /* Fill capabilities */
    while (it.hasNext() == true)
    {
        QLCCapability *cap = it.next();
        QTreeWidgetItem *item = new QTreeWidgetItem(m_capabilityList);

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

        if (cap->resourceName().isEmpty() == false)
        {
            QFile gobo(cap->resourceName());
            if (gobo.exists() == false)
            {
                QString descr = QString("[%1, %2] - %3 (%4)").arg(cap->min()).arg(cap->max())
                        .arg(cap->name()).arg(cap->resourceName());
                goboErrors.append(descr);
            }
        }
    }

    if (goboErrors.isEmpty() == false)
    {
        QMessageBox::warning(this,
                             tr("Missing resources"),
                             tr("Some gobos are missing:\n\n") + goboErrors.join("\n\n"));
    }

    m_capabilityList->sortItems(COL_MIN, Qt::AscendingOrder);
    m_capabilityList->header()->resizeSections(QHeaderView::ResizeToContents);

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

int EditChannel::currentCapabilityIndex()
{
    if (m_capabilityList->currentItem() != NULL)
        return m_capabilityList->indexOfTopLevelItem(m_capabilityList->currentItem());

    return 0;
}
