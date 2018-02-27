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
#include <QDebug>
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

EditChannel::EditChannel(QWidget *parent, QLCChannel *channel)
    : QDialog(parent)
{
    m_channel = channel ? channel->createCopy() : new QLCChannel();
    m_currentCapability = NULL;

    setupUi(this);
    init();

    QAction *action = new QAction(this);
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

QLCChannel *EditChannel::channel()
{
    return m_channel;
}

void EditChannel::init()
{
    Q_ASSERT(m_channel != NULL);

    /* Set window title */
    setWindowTitle(tr("Edit Channel: ") + m_channel->name());

    m_invalidMinMax->setStyleSheet("QLabel { color: red; }");

    /* Set name edit */
    m_nameEdit->setText(m_channel->name());
    m_nameEdit->setValidator(CAPS_VALIDATOR(this));
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameChanged(const QString&)));

    /* Get available presets and insert them into the groups combo */
    m_presetCombo->addItem(QIcon(":/edit.png"), "Custom");
    for (int i = QLCChannel::Custom + 1; i < QLCChannel::LastPreset; i++)
    {
        QLCChannel ch;
        ch.setPreset(QLCChannel::Preset(i));
        m_presetCombo->addItem(ch.getIcon(), ch.name() + " (" + ch.presetToString(QLCChannel::Preset(i)) + ")");
    }

    if (m_channel->preset() != QLCChannel::Custom)
    {
        m_typeCombo->setEnabled(false);
        m_msbRadio->setEnabled(false);
        m_lsbRadio->setEnabled(false);
        m_capabilityList->setEnabled(false);
        m_addCapabilityButton->setEnabled(false);
        m_presetCombo->setCurrentIndex(m_channel->preset());
    }

    connect(m_presetCombo, SIGNAL(activated(int)),
            this, SLOT(slotPresetActivated(int)));

    /* Get available groups/colors and insert them into the groups combo */
    QString selectedType = QLCChannel::groupToString(m_channel->group());
    int selectedIndex = 0;

    if (m_channel->group() == QLCChannel::Intensity && m_channel->colour() != QLCChannel::NoColour)
        selectedType = QLCChannel::colourToString(m_channel->colour());

    foreach (QString grp, QLCChannel::groupList())
    {
        QLCChannel ch;
        ch.setGroup(QLCChannel::stringToGroup(grp));
        m_typeCombo->addItem(ch.getIcon(), grp, ch.group());
        if (m_channel->group() == ch.group())
            selectedIndex = m_typeCombo->count() - 1;

        if (ch.group() == QLCChannel::Intensity)
        {
            foreach (QString color, QLCChannel::colourList())
            {
                QLCChannel cc;
                cc.setGroup(QLCChannel::Intensity);
                cc.setColour(QLCChannel::stringToColour(color));
                m_typeCombo->addItem(cc.getIcon(), color, cc.colour());
                if (m_channel->colour() == cc.colour())
                    selectedIndex = m_typeCombo->count() - 1;
            }
        }
    }

    m_typeCombo->setCurrentIndex(selectedIndex);
    slotGroupActivated(selectedIndex);

    m_defaultValSpin->setValue(m_channel->defaultValue());

    m_capPresetCombo->addItem(QIcon(":/edit.png"), "Custom");
    for (int i = QLCCapability::Custom + 1; i < QLCCapability::LastPreset; i++)
    {
        QLCCapability cap;
        cap.setPreset(QLCCapability::Preset(i));
        m_capPresetCombo->addItem(cap.presetToString(QLCCapability::Preset(i)));
    }

    connect(m_typeCombo, SIGNAL(activated(int)), this, SLOT(slotGroupActivated(int)));
    connect(m_defaultValSpin, SIGNAL(valueChanged(int)), this, SLOT(slotDefaultValueChanged(int)));
    connect(m_msbRadio, SIGNAL(toggled(bool)), this, SLOT(slotMsbRadioToggled(bool)));
    connect(m_lsbRadio, SIGNAL(toggled(bool)), this, SLOT(slotLsbRadioToggled(bool)));

    connect(m_addCapabilityButton, SIGNAL(clicked()), this, SLOT(slotAddCapabilityClicked()));
    connect(m_removeCapabilityButton, SIGNAL(clicked()), this, SLOT(slotRemoveCapabilityClicked()));
    connect(m_wizardButton, SIGNAL(clicked()), this, SLOT(slotWizardClicked()));

    /* Capability list connections */
    connect(m_capabilityList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotCapabilityListSelectionChanged(QTreeWidgetItem*)));
    connect(m_capabilityList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotEditCapabilityClicked()));

    connect(m_minSpin, SIGNAL(valueChanged(int)), this, SLOT(slotMinSpinChanged(int)));
    connect(m_maxSpin, SIGNAL(valueChanged(int)), this, SLOT(slotMaxSpinChanged(int)));
    connect(m_descriptionEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotDescriptionEdited(const QString&)));
    connect(m_capPresetCombo, SIGNAL(activated(int)), this, SLOT(slotCapabilityPresetActivated(int)));
    connect(m_pictureButton, SIGNAL(pressed()), this, SLOT(slotPictureButtonPressed()));
    connect(m_color1Button, SIGNAL(pressed()), this, SLOT(slotColor1ButtonPressed()));
    connect(m_color2Button, SIGNAL(pressed()), this, SLOT(slotColor2ButtonPressed()));
    connect(m_val1Spin, SIGNAL(valueChanged(double)), this, SLOT(slotValue1SpinChanged(double)));
    connect(m_val2Spin, SIGNAL(valueChanged(double)), this, SLOT(slotValue2SpinChanged(double)));

    refreshCapabilities();
    m_valueGroup->setVisible(false);
    updateCapabilityPresetGroup(false);
}

void EditChannel::setupCapabilityGroup()
{
    m_valueGroup->setVisible(true);
    m_invalidMinMax->setVisible(false);

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

    updateCapabilityPresetGroup(true);
}

void EditChannel::slotNameChanged(const QString& name)
{
    m_channel->setName(name.simplified());
}

void EditChannel::slotPresetActivated(int index)
{
    bool enable = index == 0 ? true : false;

    m_typeCombo->setEnabled(enable);
    m_msbRadio->setEnabled(enable);
    m_lsbRadio->setEnabled(enable);
    m_capabilityList->setEnabled(enable);
    m_addCapabilityButton->setEnabled(enable);
    m_wizardButton->setEnabled(enable);

    if (index == m_channel->preset())
        return;

    if (index != 0)
        m_channel->setName("");

    m_channel->setPreset(QLCChannel::Preset(index));

    if (index == 0)
        return;

    m_nameEdit->setText(m_channel->name());

    /* Select the channel's group */
    for (int i = 0; i < m_typeCombo->count(); i++)
    {
        quint32 val = m_typeCombo->itemData(i).toUInt();

        if ((m_channel->colour() == QLCChannel::NoColour && val == m_channel->group()) ||
            (m_channel->group() == QLCChannel::Intensity && val == m_channel->colour()))
        {
            m_typeCombo->setCurrentIndex(i);
            slotGroupActivated(i);
            break;
        }
    }

    foreach(QLCCapability *cap, m_channel->capabilities())
        m_channel->removeCapability(cap);

    m_channel->addPresetCapability();

    refreshCapabilities();
}

void EditChannel::slotGroupActivated(int index)
{
    quint32 val = m_typeCombo->itemData(index).toUInt();

    if (val > QLCChannel::Maintenance && val < QLCChannel::NoGroup)
    {
        m_channel->setGroup(QLCChannel::Intensity);
        m_channel->setColour(QLCChannel::PrimaryColour(val));
    }
    else
    {
        m_channel->setGroup(QLCChannel::Group(val));
        m_channel->setColour(QLCChannel::NoColour);
    }

    if (m_channel->controlByte() == QLCChannel::MSB)
        m_msbRadio->setChecked(true);
    else
        m_lsbRadio->setChecked(true);
}

void EditChannel::slotDefaultValueChanged(int val)
{
    m_channel->setDefaultValue(uchar(val));
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

/****************************************************************************
 * Capability list functions
 ****************************************************************************/

void EditChannel::slotCapabilityListSelectionChanged(QTreeWidgetItem *item)
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

    QLCCapability *newCapability = new QLCCapability();
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
    QTreeWidgetItem *item;
    QTreeWidgetItem *next;

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
        updateCapabilityPresetGroup(false);
    }
}

void EditChannel::slotEditCapabilityClicked()
{
    m_currentCapability = currentCapability();
    if (m_currentCapability == NULL)
    {
        m_valueGroup->setVisible(false);
        updateCapabilityPresetGroup(false);
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
            QLCCapability *cap = it.next()->createCopy();
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

void EditChannel::slotCapabilityPresetActivated(int index)
{
    m_currentCapability->setPreset(QLCCapability::Preset(index));
    updateCapabilityPresetGroup(true);
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
    m_currentCapability->setResource(0, filename);
}

void EditChannel::slotColor1ButtonPressed()
{
    QColorDialog dialog(this);
    QColor col1 = m_currentCapability->resource(0).value<QColor>();
    if (col1.isValid())
        dialog.setCurrentColor(col1);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QColor color = dialog.selectedColor();
    QPixmap pix(58, 58);
    pix.fill(color);
    m_resourceButton->setIcon(pix);
    m_currentCapability->setResource(0, color);
    m_color2Button->setEnabled(true);
}

void EditChannel::slotColor2ButtonPressed()
{
    QColorDialog dialog(this);
    QColor col1 = m_currentCapability->resource(0).value<QColor>();
    QColor col2 = m_currentCapability->resource(1).value<QColor>();
    if (col2.isValid())
        dialog.setCurrentColor(col2);
    if (dialog.exec() != QDialog::Accepted)
        return;

    col2 = dialog.selectedColor();
    QPixmap pix(58, 58);
    QPainter painter(&pix);
    painter.fillRect(0, 0, 29, 58, col1);
    painter.fillRect(29, 0, 58, 58, col2);

    m_resourceButton->setIcon(pix);
    m_currentCapability->setResource(1, col2);
}

void EditChannel::slotValue1SpinChanged(double value)
{
    m_currentCapability->setResource(0, value);
}

void EditChannel::slotValue2SpinChanged(double value)
{
    m_currentCapability->setResource(1, value);
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

        if (cap->presetType() == QLCCapability::Picture && cap->resource(0).isValid())
        {
            QString path = cap->resource(0).toString();
            QFile gobo(path);
            if (gobo.exists() == false)
            {
                QString descr = QString("[%1, %2] - %3 (%4)").arg(cap->min()).arg(cap->max())
                        .arg(cap->name()).arg(path);
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

QLCCapability *EditChannel::currentCapability()
{
    QTreeWidgetItem *item;
    QLCCapability *cap = NULL;

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

void EditChannel::updateCapabilityPresetGroup(bool show)
{
    bool showColor1 = false;
    bool showColor2 = false;
    bool showPicture = false;
    bool showPreview = false;
    bool showValue1 = false;
    bool showValue2 = false;

    if (show && m_currentCapability)
    {
        if (m_capPresetCombo->currentIndex() != m_currentCapability->preset())
            m_capPresetCombo->setCurrentIndex(m_currentCapability->preset());

        QLCCapability::PresetType type = m_currentCapability->presetType();

        switch (type)
        {
            case QLCCapability::Picture:
                m_resourceButton->setIcon(QIcon(m_currentCapability->resource(0).toString()));
                showPicture = true;
                showPreview = true;
            break;
            case QLCCapability::SingleColor:
            {
                QPixmap pix(58, 58);
                QColor col1 = m_currentCapability->resource(0).value<QColor>();
                if (col1.isValid())
                    pix.fill(col1);
                else
                    pix.fill(QColor(Qt::transparent));
                m_resourceButton->setIcon(pix);

                showColor1 = true;
                showPreview = true;
            }
            break;
            case QLCCapability::DoubleColor:
            {
                QPixmap pix(58, 58);
                QColor col1 = m_currentCapability->resource(0).value<QColor>();

                if (col1.isValid())
                {
                    QColor col2 = m_currentCapability->resource(1).value<QColor>();
                    pix.fill(col1);

                    if (col2.isValid())
                    {
                        QPainter painter(&pix);
                        painter.fillRect(29, 0, 58, 58, col2);
                    }
                }
                else
                {
                    pix.fill(QColor(Qt::transparent));
                }

                m_resourceButton->setIcon(pix);
                m_color2Button->setEnabled(true);

                showColor1 = true;
                showColor2 = true;
                showPreview = true;
            }
            break;
            case QLCCapability::SingleValue:
                showValue1 = true;
            break;
            case QLCCapability::DoubleValue:
                showValue1 = true;
                showValue2 = true;
            break;
            default:
            break;
        }
    }

    m_capPresetLabel->setVisible(show);
    m_capPresetCombo->setVisible(show);
    m_color1Label->setVisible(showColor1);
    m_color1Button->setVisible(showColor1);
    m_color2Label->setVisible(showColor2);
    m_color2Button->setVisible(showColor2);
    m_pictureLabel->setVisible(showPicture);
    m_pictureButton->setVisible(showPicture);
    m_resourceGroup->setVisible(showPreview);
    m_val1Label->setVisible(showValue1);
    m_val1Spin->setVisible(showValue1);
    m_val2Label->setVisible(showValue2);
    m_val2Spin->setVisible(showValue2);
}
