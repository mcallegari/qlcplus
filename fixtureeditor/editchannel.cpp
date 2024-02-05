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
#include <QAction>

#include "qlccapability.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#include "capabilitywizard.h"
#include "editchannel.h"
#include "util.h"

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

    m_invalidMinMax->setVisible(false);

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

    connect(m_removeCapabilityButton, SIGNAL(clicked()), this, SLOT(slotRemoveCapabilityClicked()));
    connect(m_wizardButton, SIGNAL(clicked()), this, SLOT(slotWizardClicked()));

    refreshCapabilities();

    /* Capability list connections */
    connect(m_capabilityList, SIGNAL(cellChanged(int,int)), this, SLOT(slotCapabilityCellChanged(int,int)));
    connect(m_capabilityList, SIGNAL(currentCellChanged(int,int,int,int)),
            this, SLOT(slotCapabilityCellSelected(int,int,int,int)));

    connect(m_capPresetCombo, SIGNAL(activated(int)), this, SLOT(slotCapabilityPresetActivated(int)));
    connect(m_pictureButton, SIGNAL(pressed()), this, SLOT(slotPictureButtonPressed()));
    connect(m_color1Button, SIGNAL(pressed()), this, SLOT(slotColor1ButtonPressed()));
    connect(m_color2Button, SIGNAL(pressed()), this, SLOT(slotColor2ButtonPressed()));
    connect(m_val1Spin, SIGNAL(valueChanged(double)), this, SLOT(slotValue1SpinChanged(double)));
    connect(m_val2Spin, SIGNAL(valueChanged(double)), this, SLOT(slotValue2SpinChanged(double)));

    updateCapabilityPresetGroup(false);

    m_nameEdit->setFocus();
}

void EditChannel::setupCapabilityGroup()
{
    m_invalidMinMax->setVisible(false);
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
    m_wizardButton->setEnabled(enable);

    if (index == m_channel->preset())
        return;

    if (index != 0)
        m_channel->setName("");

    m_channel->setPreset(QLCChannel::Preset(index));

    if (index == 0)
        return;

    if (m_nameEdit->text().isEmpty())
        m_nameEdit->setText(m_channel->name());
    else
        m_channel->setName(m_nameEdit->text());

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

    foreach (QLCCapability *cap, m_channel->capabilities())
        m_channel->removeCapability(cap);

    m_channel->addPresetCapability();

    refreshCapabilities();
}

void EditChannel::slotGroupActivated(int index)
{
    quint32 val = m_typeCombo->itemData(index).toUInt();

    if (val > QLCChannel::Nothing && val < QLCChannel::NoGroup)
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

void EditChannel::slotCapabilityCellChanged(int row, int column)
{
    QTableWidgetItem *item = m_capabilityList->item(row, column);
    if (item == NULL)
        return;

    QLCCapability *cap = getRowCapability(row);
    if (cap == NULL)
        return;

    bool restore = false;

    if (column == COL_MIN || column == COL_MAX)
    {
        bool ok = false;
        int newValue = item->text().toInt(&ok);
        if (ok == false || newValue < 0 || newValue > 255)
        {
            // restore the original value
            restore = true;
        }
        else
        {
            if (column == COL_MIN)
            {
                if (newValue < cap->max())
                    cap->setMin(newValue);
                else
                    restore = true;
            }
            else
            {
                if (newValue >= cap->min())
                    cap->setMax(newValue);
                else
                    restore = true;
            }
        }
    }
    else
    {
        cap->setName(item->text());
    }

    if (restore)
        item->setText(column == COL_MIN ? QString::number(cap->min()) : QString::number(cap->max()));

    checkOverlapping();
}

void EditChannel::slotCapabilityCellSelected(int currentRow, int currentColumn,
                                             int previousRow, int previousColumn)
{
    QTableWidgetItem *prevItem = m_capabilityList->item(previousRow, previousColumn);
    QTableWidgetItem *currItem = m_capabilityList->item(currentRow, currentColumn);

    Q_UNUSED(prevItem)

    if (currItem)
    {
        m_currentCapability = getRowCapability(currentRow);
    }
    else
    {
        uchar min = 0;
        QString str;

        if (currentRow > 0)
        {
            for (int i = currentRow; i >= 0; i--)
            {
                QLCCapability *cap = getRowCapability(i);
                if (cap)
                {
                    // maximum already reached. Nothing to do here
                    if (min == 255)
                    {
                        m_currentCapability = NULL;
                        updateCapabilityPresetGroup(false);
                        return;
                    }
                    min = cap->max() + 1;
                    break;
                }
            }
        }

        QLCCapability *cap = new QLCCapability(min, UCHAR_MAX);
        if (m_channel->addCapability(cap) == false)
        {
            delete cap;
            return;
        }

        QTableWidgetItem *item = new QTableWidgetItem(str.asprintf("%.3d", cap->min()));
        m_capabilityList->setItem(currentRow, COL_MIN, item);

        item = new QTableWidgetItem(str.asprintf("%.3d", cap->max()));
        m_capabilityList->setItem(currentRow, COL_MAX, item);

        item = new QTableWidgetItem(cap->name());
        m_capabilityList->setItem(currentRow, COL_NAME, item);

        // QLCCapability reference
        item->setData(Qt::UserRole, QVariant::fromValue((void *)cap));

        m_currentCapability = cap;
    }

    updateCapabilityPresetGroup(true);

    if (currentRow == m_capabilityList->rowCount() - 1)
        m_capabilityList->setRowCount(m_capabilityList->rowCount() + 1);

}

/****************************************************************************
 * Capability list functions
 ****************************************************************************/

void EditChannel::slotRemoveCapabilityClicked()
{
    QTableWidgetItem *item;

    int row = m_capabilityList->currentRow();

    item = m_capabilityList->item(row, COL_MIN);
    if (item != NULL)
        delete item;

    item = m_capabilityList->item(row, COL_MAX);
    if (item != NULL)
        delete item;

    item = m_capabilityList->item(row, COL_NAME);
    if (item != NULL)
    {
        // This also deletes the capability
        QLCCapability *cap = (QLCCapability *) item->data(Qt::UserRole).value<void *>();
        m_channel->removeCapability(cap);
        delete item;
    }

    m_currentCapability = currentCapability();
    if (m_currentCapability != NULL)
        setupCapabilityGroup();
    else
    {
        updateCapabilityPresetGroup(false);
    }
}

void EditChannel::slotEditCapabilityClicked()
{
    m_currentCapability = currentCapability();
    if (m_currentCapability == NULL)
    {
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
    dialog.setNameFilter((tr("Gobo pictures") + " (*.jpg *.jpeg *.png *.bmp *.svg)"));

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
    m_currentCapability->setResource(0, m_val1Spin->value());
    m_currentCapability->setResource(1, value);
}

void EditChannel::refreshCapabilities()
{
    m_channel->sortCapabilities();

    QListIterator <QLCCapability*> it(m_channel->capabilities());
    QString str;
    int i = 0;

    m_capabilityList->clear();
    m_capabilityList->setHorizontalHeaderLabels(QStringList() << tr("Minimum value") << tr("Maximum value") << tr("Description"));

    QStringList goboErrors;

    /* Fill capabilities */
    while (it.hasNext() == true)
    {
        QLCCapability *cap = it.next();

        // Min
        QTableWidgetItem *item = new QTableWidgetItem(str.asprintf("%.3d", cap->min()));
        m_capabilityList->setItem(i, COL_MIN, item);

        // Max
        item = new QTableWidgetItem(str.asprintf("%.3d", cap->max()));
        m_capabilityList->setItem(i, COL_MAX, item);

        // Name
        item = new QTableWidgetItem(cap->name());
        m_capabilityList->setItem(i, COL_NAME, item);

        // QLCCapability reference
        item->setData(Qt::UserRole, QVariant::fromValue((void *)cap));

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
        i++;
        if (i == m_capabilityList->rowCount())
            m_capabilityList->setRowCount(i + 1);
    }

    if (goboErrors.isEmpty() == false)
    {
        QMessageBox::warning(this,
                             tr("Missing resources"),
                             tr("Some gobos are missing:\n\n") + goboErrors.join("\n\n"));
    }

    m_capabilityList->resizeColumnToContents(COL_MIN);
    m_capabilityList->resizeColumnToContents(COL_MAX);
}

QLCCapability *EditChannel::currentCapability()
{
    QLCCapability *cap = NULL;
    int row = m_capabilityList->currentRow();
    QTableWidgetItem *item = m_capabilityList->item(row, COL_NAME);

    if (item != NULL)
        cap = (QLCCapability*) item->data(Qt::UserRole).value<void *>();

    return cap;
}

QLCCapability *EditChannel::getRowCapability(int row)
{
    QLCCapability *cap = NULL;
    QTableWidgetItem *item = m_capabilityList->item(row, COL_NAME);
    if (item)
        cap = (QLCCapability*) item->data(Qt::UserRole).value<void *>();

    return cap;
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
                m_val1Spin->setValue(m_currentCapability->resource(0).toFloat());
                m_val1Spin->setSuffix(m_currentCapability->presetUnits());
                showValue1 = true;
            break;
            case QLCCapability::DoubleValue:
                m_val1Spin->setValue(m_currentCapability->resource(0).toFloat());
                m_val2Spin->setValue(m_currentCapability->resource(1).toFloat());
                m_val1Spin->setSuffix(m_currentCapability->presetUnits());
                m_val2Spin->setSuffix(m_currentCapability->presetUnits());
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

void EditChannel::checkOverlapping()
{
    char valArray[256];
    memset(valArray, 0, 256);

    m_invalidMinMax->setVisible(false);

    QListIterator <QLCCapability*> it(m_channel->capabilities());
    while (it.hasNext() == true)
    {
        QLCCapability *cap = it.next();
        for (int i = cap->min(); i <= cap->max(); i++)
        {
            if (valArray[i])
                m_invalidMinMax->setVisible(true);
        }
        memset(valArray + cap->min(), 1, (cap->max() - cap->min()) + 1);
    }
}
