/*
  Q Light Controller
  vcsliderproperties.cpp

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

#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QRadioButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "qlccapability.h"
#include "qlcchannel.h"

#include "vcsliderproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "mastertimer.h"
#include "inputpatch.h"
#include "vcslider.h"
#include "fixture.h"
#include "doc.h"

#define KColumnName  0
#define KColumnType  1
#define KColumnRange 2
#define KColumnID    3

VCSliderProperties::VCSliderProperties(VCSlider* slider, Doc* doc)
    : QDialog(slider)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(slider != NULL);
    m_slider = slider;

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    /* Level page connections */
    connect(m_levelLowLimitSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotLevelLowSpinChanged(int)));
    connect(m_levelHighLimitSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotLevelHighSpinChanged(int)));
    connect(m_levelCapabilityButton, SIGNAL(clicked()),
            this, SLOT(slotLevelCapabilityClicked()));
    connect(m_levelList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotLevelListClicked(QTreeWidgetItem*)));
    connect(m_levelAllButton, SIGNAL(clicked()),
            this, SLOT(slotLevelAllClicked()));
    connect(m_levelNoneButton, SIGNAL(clicked()),
            this, SLOT(slotLevelNoneClicked()));
    connect(m_levelInvertButton, SIGNAL(clicked()),
            this, SLOT(slotLevelInvertClicked()));
    connect(m_levelByGroupButton, SIGNAL(clicked()),
            this, SLOT(slotLevelByGroupClicked()));
    connect(m_switchToLevelModeButton, SIGNAL(clicked()),
            this, SLOT(slotModeLevelClicked()));

    /* Playback page connections */
    connect(m_switchToPlaybackModeButton, SIGNAL(clicked()),
            this, SLOT(slotModePlaybackClicked()));
    connect(m_attachPlaybackFunctionButton, SIGNAL(clicked()),
            this, SLOT(slotAttachPlaybackFunctionClicked()));
    connect(m_detachPlaybackFunctionButton, SIGNAL(clicked()),
            this, SLOT(slotDetachPlaybackFunctionClicked()));

    /* Submaster page connections */
    connect(m_switchToSubmasterModeButton, SIGNAL(clicked()),
            this, SLOT(slotModeSubmasterClicked()));

    /*********************************************************************
     * General page
     *********************************************************************/

    /* Name */
    m_nameEdit->setText(m_slider->caption());

    /* Widget appearance */
    if (m_slider->widgetStyle() == VCSlider::WKnob)
        m_widgetKnobRadio->setChecked(true);
    else
        m_widgetSliderRadio->setChecked(true);

    /* Slider mode */
    m_sliderMode = m_slider->sliderMode();
    switch (m_sliderMode)
    {
    default:
    case VCSlider::Level:
        slotModeLevelClicked();
        break;
    case VCSlider::Playback:
        slotModePlaybackClicked();
        break;
    case VCSlider::Submaster:
        slotModeSubmasterClicked();
        break;
    }

    /* Slider movement (Qt understands inverted appearance vice versa) */
    if (m_slider->invertedAppearance() == true)
        m_sliderMovementInvertedRadio->setChecked(true);
    else
        m_sliderMovementNormalRadio->setChecked(true);

    /* Value display style */
    switch (m_slider->valueDisplayStyle())
    {
    default:
    case VCSlider::ExactValue:
        m_valueExactRadio->setChecked(true);
        break;
    case VCSlider::PercentageValue:
        m_valuePercentageRadio->setChecked(true);
        break;
    }

    /********************************************************************
     * External input
     ********************************************************************/
    m_inputSource = m_slider->inputSource();
    updateInputSource();

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseInputClicked()));

    /*********************************************************************
     * Level page
     *********************************************************************/

    /* Level limit spins */
    m_levelLowLimitSpin->setValue(m_slider->levelLowLimit());
    m_levelHighLimitSpin->setValue(m_slider->levelHighLimit());

    /* Tree widget contents */
    levelUpdateFixtures();
    levelUpdateChannelSelections();

    connect(m_levelList, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotItemExpanded()));
    connect(m_levelList, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(slotItemExpanded()));

    m_monitorValuesCheck->setChecked(m_slider->channelsMonitorEnabled());

    /*********************************************************************
     * Playback page
     *********************************************************************/

    /* Function */
    m_playbackFunctionId = m_slider->playbackFunction();
    updatePlaybackFunctionName();


}

VCSliderProperties::~VCSliderProperties()
{
}

/*****************************************************************************
 * General page
 *****************************************************************************/

void VCSliderProperties::slotModeLevelClicked()
{
    m_sliderMode = VCSlider::Level;

    m_nameEdit->setEnabled(true);

    setLevelPageVisibility(true);
    setPlaybackPageVisibility(false);
    setSubmasterPageVisibility(false);

    int cngType = m_slider->clickAndGoType();
    switch(cngType)
    {
        case ClickAndGoWidget::Red:
        case ClickAndGoWidget::Green:
        case ClickAndGoWidget::Blue:
        case ClickAndGoWidget::Cyan:
        case ClickAndGoWidget::Magenta:
        case ClickAndGoWidget::Yellow:
        case ClickAndGoWidget::Amber:
        case ClickAndGoWidget::White:
        case ClickAndGoWidget::UV:
            m_cngColorCheck->setChecked(true);
        break;
        case ClickAndGoWidget::RGB:
            m_cngRGBCheck->setChecked(true);
        break;
        case ClickAndGoWidget::CMY:
            m_cngCMYCheck->setChecked(true);
        break;
        case ClickAndGoWidget::Preset:
            m_cngPresetCheck->setChecked(true);
        break;
        default:
        case ClickAndGoWidget::None:
            m_cngNoneCheck->setChecked(true);
        break;

    }
}

void VCSliderProperties::slotModePlaybackClicked()
{
    m_sliderMode = VCSlider::Playback;

    m_nameEdit->setEnabled(true);

    setLevelPageVisibility(false);
    setPlaybackPageVisibility(true);
    setSubmasterPageVisibility(false);
}

void VCSliderProperties::slotModeSubmasterClicked()
{
    m_sliderMode = VCSlider::Submaster;

    setLevelPageVisibility(false);
    setPlaybackPageVisibility(false);
    setSubmasterPageVisibility(true);
}

void VCSliderProperties::slotAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
}

void VCSliderProperties::slotInputValueChanged(quint32 universe, quint32 channel)
{
    if (m_inputSource != NULL)
        delete m_inputSource;
    m_inputSource = new QLCInputSource(universe, (m_slider->page() << 16) | channel);
    updateInputSource();
}

void VCSliderProperties::slotChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_inputSource != NULL)
            delete m_inputSource;
        m_inputSource = new QLCInputSource(sic.universe(), sic.channel());
        updateInputSource();
    }
}

void VCSliderProperties::updateInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_inputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_inputUniverseEdit->setText(uniName);
    m_inputChannelEdit->setText(chName);
}

void VCSliderProperties::setLevelPageVisibility(bool visible)
{
    m_levelValueRangeGroup->setVisible(visible);
    m_levelList->setVisible(visible);
    m_levelAllButton->setVisible(visible);
    m_levelNoneButton->setVisible(visible);
    m_levelInvertButton->setVisible(visible);
    m_levelByGroupButton->setVisible(visible);
    m_clickngoGroup->setVisible(visible);
    m_monitorValuesCheck->setVisible(visible);

    if (visible == true)
    {
        m_switchToLevelModeButton->hide();
        m_levelSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
    else
    {
        m_switchToLevelModeButton->show();
        m_levelSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
}

void VCSliderProperties::setPlaybackPageVisibility(bool visible)
{
    m_playbackFunctionGroup->setVisible(visible);

    if (visible == true)
    {
        m_switchToPlaybackModeButton->hide();
        m_playbackSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
    else
    {
        m_switchToPlaybackModeButton->show();
        m_playbackSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
}

void VCSliderProperties::setSubmasterPageVisibility(bool visible)
{
    m_submasterInfo->setVisible(visible);

    if (visible == true)
    {
        m_switchToSubmasterModeButton->hide();
        m_submasterSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
    else
    {
        m_switchToSubmasterModeButton->show();
        m_submasterSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
}

/*****************************************************************************
 * Level page
 *****************************************************************************/

void VCSliderProperties::levelUpdateFixtures()
{
    foreach(Fixture* fixture, m_doc->fixtures())
    {
        Q_ASSERT(fixture != NULL);
        levelUpdateFixtureNode(fixture->id());
    }
    m_levelList->resizeColumnToContents(KColumnName);
    m_levelList->resizeColumnToContents(KColumnType);
}

void VCSliderProperties::levelUpdateFixtureNode(quint32 id)
{
    QTreeWidgetItem* item;
    Fixture* fxi;
    QString str;

    fxi = m_doc->fixture(id);
    Q_ASSERT(fxi != NULL);

    item = levelFixtureNode(id);
    if (item == NULL)
    {
        item = new QTreeWidgetItem(m_levelList);
        item->setText(KColumnID, str.setNum(id));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable
                       | Qt::ItemIsTristate);
    }

    item->setText(KColumnName, fxi->name());
    item->setIcon(KColumnName, fxi->getIconFromType(fxi->type()));
    item->setText(KColumnType, fxi->type());

    levelUpdateChannels(item, fxi);

}

QTreeWidgetItem* VCSliderProperties::levelFixtureNode(quint32 id)
{
    QTreeWidgetItem* item;
    int i;

    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        item = m_levelList->topLevelItem(i);
        if (item->text(KColumnID).toUInt() == id)
            return item;
    }

    return NULL;
}

void VCSliderProperties::levelUpdateChannels(QTreeWidgetItem* parent,
                                             Fixture* fxi)
{
    quint32 channels = 0;
    quint32 ch = 0;

    Q_ASSERT(parent != NULL);
    Q_ASSERT(fxi != NULL);

    channels = fxi->channels();
    for (ch = 0; ch < channels; ch++)
        levelUpdateChannelNode(parent, fxi, ch);
}

void VCSliderProperties::levelUpdateChannelNode(QTreeWidgetItem* parent,
                                                Fixture* fxi, quint32 ch)
{
    Q_ASSERT(parent != NULL);

    if (fxi == NULL)
        return;

    const QLCChannel* channel = fxi->channel(ch);
    if (channel == NULL)
        return;

    QTreeWidgetItem* item = levelChannelNode(parent, ch);
    if (item == NULL)
    {
        item = new QTreeWidgetItem(parent);
        item->setText(KColumnID, QString::number(ch));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(KColumnName, Qt::Unchecked);
    }

    item->setText(KColumnName, QString("%1:%2").arg(ch + 1)
                  .arg(channel->name()));
    item->setIcon(KColumnName, channel->getIcon());
    if (channel->group() == QLCChannel::Intensity &&
        channel->colour() != QLCChannel::NoColour)
        item->setText(KColumnType, QLCChannel::colourToString(channel->colour()));
    else
        item->setText(KColumnType, QLCChannel::groupToString(channel->group()));


    levelUpdateCapabilities(item, channel);
}

QTreeWidgetItem* VCSliderProperties::levelChannelNode(QTreeWidgetItem* parent,
                                                      quint32 ch)
{
    Q_ASSERT(parent != NULL);

    for (int i = 0; i < parent->childCount(); i++)
    {
        QTreeWidgetItem* item = parent->child(i);
        if (item->text(KColumnID).toUInt() == ch)
            return item;
    }

    return NULL;
}

void VCSliderProperties::levelUpdateCapabilities(QTreeWidgetItem* parent,
                                                 const QLCChannel* channel)
{
    Q_ASSERT(parent != NULL);
    Q_ASSERT(channel != NULL);

    QListIterator <QLCCapability*> it(channel->capabilities());
    while (it.hasNext() == true)
        levelUpdateCapabilityNode(parent, it.next());
}

void VCSliderProperties::levelUpdateCapabilityNode(QTreeWidgetItem* parent,
        QLCCapability* cap)
{
    QTreeWidgetItem* item;
    QString str;

    Q_ASSERT(parent != NULL);
    Q_ASSERT(cap != NULL);

    item = new QTreeWidgetItem(parent);
    item->setText(KColumnName, cap->name());
    item->setText(KColumnRange, str.sprintf("%.3d - %.3d",
                                            cap->min(), cap->max()));
    item->setFlags(item->flags() & (~Qt::ItemIsUserCheckable));
}

void VCSliderProperties::levelUpdateChannelSelections()
{
    /* Check all items that are present in the slider's list of
       controlled channels. We don't need to set other items off,
       because this function is run only during init when everything
       is off. */
    QListIterator <VCSlider::LevelChannel> it(m_slider->m_levelChannels);
    while (it.hasNext() == true)
    {
        VCSlider::LevelChannel lch(it.next());

        QTreeWidgetItem* fxiNode = levelFixtureNode(lch.fixture);
        if (fxiNode == NULL)
            continue;

        QTreeWidgetItem* chNode = levelChannelNode(fxiNode, lch.channel);
        if (chNode == NULL)
            continue;

        chNode->setCheckState(KColumnName, Qt::Checked);
    }
}

void VCSliderProperties::levelSelectChannelsByGroup(QString group)
{
    QTreeWidgetItem* fxi_item;
    QTreeWidgetItem* ch_item;
    int i;
    int j;

    /* Go thru only channel items. Fixture items get (partially) selected
       according to their children's state */
    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        for (j = 0; j < fxi_item->childCount(); j++)
        {
            ch_item = fxi_item->child(j);
            Q_ASSERT(ch_item != NULL);

            if (ch_item->text(KColumnType) == group)
                ch_item->setCheckState(KColumnName, Qt::Checked);
        }
    }
}

void VCSliderProperties::slotLevelLowSpinChanged(int value)
{
    if (value >= m_levelHighLimitSpin->value())
        m_levelHighLimitSpin->setValue(value + 1);
}

void VCSliderProperties::slotLevelHighSpinChanged(int value)
{
    if (value <= m_levelLowLimitSpin->value())
        m_levelLowLimitSpin->setValue(value - 1);
}

void VCSliderProperties::slotLevelCapabilityClicked()
{
    QTreeWidgetItem* item;
    QStringList list;

    item = m_levelList->currentItem();
    if (item == NULL || item->parent() == NULL ||
            item->parent()->parent() == NULL)
        return;

    list = item->text(KColumnRange).split("-");
    Q_ASSERT(list.size() == 2);

    m_levelLowLimitSpin->setValue(list[0].toInt());
    m_levelHighLimitSpin->setValue(list[1].toInt());
}

void VCSliderProperties::slotLevelListClicked(QTreeWidgetItem* item)
{
    /* Enable the capability button if a capability has been selected */
    if (item != NULL && item->parent() != NULL &&
            item->parent()->parent() != NULL)
    {
        m_levelCapabilityButton->setEnabled(true);
    }
    else
    {
        m_levelCapabilityButton->setEnabled(false);
    }
}

void VCSliderProperties::slotLevelAllClicked()
{
    QTreeWidgetItem* fxi_item;
    int i;

    /* Set all fixture items selected, their children should get selected
       as well because the fixture items are Controller items. */
    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        fxi_item->setCheckState(KColumnName, Qt::Checked);
    }
}

void VCSliderProperties::slotLevelNoneClicked()
{
    QTreeWidgetItem* fxi_item;
    int i;

    /* Set all fixture items unselected, their children should get unselected
       as well because the fixture items are Controller items. */
    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        fxi_item->setCheckState(KColumnName, Qt::Unchecked);
    }
}

void VCSliderProperties::slotLevelInvertClicked()
{
    QTreeWidgetItem* fxi_item;
    QTreeWidgetItem* ch_item;
    int i;
    int j;

    /* Go thru only channel items. Fixture items get (partially) selected
       according to their children's state */
    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        for (j = 0; j < fxi_item->childCount(); j++)
        {
            ch_item = fxi_item->child(j);
            Q_ASSERT(ch_item != NULL);

            if (ch_item->checkState(KColumnName) == Qt::Checked)
                ch_item->setCheckState(KColumnName,
                                       Qt::Unchecked);
            else
                ch_item->setCheckState(KColumnName,
                                       Qt::Checked);
        }
    }
}

void VCSliderProperties::slotLevelByGroupClicked()
{
    bool ok = false;
    QString group;
    QStringList groups;

    foreach(Fixture* fixture, m_doc->fixtures())
    {
        Q_ASSERT(fixture != NULL);

        for(quint32 i = 0; i < fixture->channels(); i++)
        {
            QLCChannel channel = fixture->channel(i);

            QString property = QLCChannel::groupToString(channel.group());

            if (channel.group() == QLCChannel::Intensity &&
                channel.colour() != QLCChannel::NoColour)
            {
                property = QLCChannel::colourToString(channel.colour());
            }

            if (groups.contains(property) == false)
                groups.append(property);
        }
    }

    group = QInputDialog::getItem(this,
                                  tr("Select channels by group"),
                                  tr("Select a channel group"),
                                  groups, 0,
                                  false, &ok);

    if (ok == true)
        levelSelectChannelsByGroup(group);
}

void VCSliderProperties::slotItemExpanded()
{
    m_levelList->resizeColumnToContents(KColumnName);
    m_levelList->resizeColumnToContents(KColumnType);
}

/*****************************************************************************
 * Playback page
 *****************************************************************************/

void VCSliderProperties::slotAttachPlaybackFunctionClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(false);
    fs.setFilter(Function::Scene | Function::Chaser | Function::EFX | Function::Audio | Function::RGBMatrix
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                 | Function::Video
#endif
                 , true);

    if (fs.exec() != QDialog::Accepted)
        return;

    if (fs.selection().size() == 0)
        return;

    m_playbackFunctionId = fs.selection().first();
    updatePlaybackFunctionName();
}

void VCSliderProperties::slotDetachPlaybackFunctionClicked()
{
    m_playbackFunctionId = Function::invalidId();
    updatePlaybackFunctionName();
}

void VCSliderProperties::updatePlaybackFunctionName()
{
    Function* function = m_doc->function(m_playbackFunctionId);
    if (function != NULL)
    {
        m_playbackFunctionEdit->setText(function->name());
        if (m_nameEdit->text().simplified().isEmpty() == true)
            m_nameEdit->setText(function->name());
    }
    else
    {
        m_playbackFunctionId = Function::invalidId(); // Ensure
        m_playbackFunctionEdit->setText(tr("No function"));
    }
}

/*****************************************************************************
 * OK & Cancel
 *****************************************************************************/

void VCSliderProperties::checkMajorColor(int *comp, int *max, int type)
{
    if (*comp > *max)
    {
        *max = *comp;
        m_slider->setClickAndGoType((ClickAndGoWidget::ClickAndGo)type);
    }
}

void VCSliderProperties::storeLevelChannels()
{
    int red = 0, green = 0, blue = 0;
    int cyan = 0, magenta = 0, yellow = 0, amber = 0, white = 0, uv = 0;
    int majorColor = 0;
    /* Clear all channels from the slider first */
    m_slider->clearLevelChannels();

    /* Go thru all fixtures and their channels, add checked channels */
    for (int i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        quint32 fxi_id = fxi_item->text(KColumnID).toUInt();
        Fixture *fxi = m_doc->fixture(fxi_id);

        for (int j = 0; j < fxi_item->childCount(); j++)
        {
            QTreeWidgetItem* ch_item = fxi_item->child(j);
            Q_ASSERT(ch_item != NULL);

            if (ch_item->checkState(KColumnName) == Qt::Checked)
            {
                quint32 ch_num = ch_item->text(KColumnID).toUInt();
                if (fxi != NULL)
                {
                    const QLCChannel *ch = fxi->channel(ch_num);
                    if (ch->group() == QLCChannel::Intensity &&
                        m_cngColorCheck->isChecked())
                    {
                        if (ch->colour() == QLCChannel::Red)
                        {
                            red++;
                            checkMajorColor(&red, &majorColor, ClickAndGoWidget::Red);
                        }
                        else if (ch->colour() == QLCChannel::Green)
                        {
                            green++;
                            checkMajorColor(&green, &majorColor, ClickAndGoWidget::Green);
                        }
                        else if (ch->colour() == QLCChannel::Blue)
                        {
                            blue++;
                            checkMajorColor(&blue, &majorColor, ClickAndGoWidget::Blue);
                        }
                        else if (ch->colour() == QLCChannel::Cyan)
                        {
                            cyan++;
                            checkMajorColor(&cyan, &majorColor, ClickAndGoWidget::Cyan);
                        }
                        else if (ch->colour() == QLCChannel::Magenta)
                        {
                            magenta++;
                            checkMajorColor(&magenta, &majorColor, ClickAndGoWidget::Magenta);
                        }
                        else if (ch->colour() == QLCChannel::Yellow)
                        {
                            yellow++;
                            checkMajorColor(&yellow, &majorColor, ClickAndGoWidget::Yellow);
                        }
                        else if (ch->colour() == QLCChannel::Amber)
                        {
                            amber++;
                            checkMajorColor(&amber, &majorColor, ClickAndGoWidget::Amber);
                        }
                        else if (ch->colour() == QLCChannel::White)
                        {
                            white++;
                            checkMajorColor(&white, &majorColor, ClickAndGoWidget::White);
                        }
                        else if (ch->colour() == QLCChannel::UV)
                        {
                            uv++;
                            checkMajorColor(&uv, &majorColor, ClickAndGoWidget::UV);
                        }
                    }
                }
                m_slider->addLevelChannel(fxi_id, ch_num);
            }
        }
    }
}

void VCSliderProperties::accept()
{
    /* Widget appearance */
    if (m_widgetKnobRadio->isChecked())
        m_slider->setWidgetStyle(VCSlider::WKnob);
    else
        m_slider->setWidgetStyle(VCSlider::WSlider);

    /* Level page */
    bool limitDiff =
        (m_slider->levelLowLimit() != m_levelLowLimitSpin->value()) ||
        (m_slider->levelHighLimit() != m_levelLowLimitSpin->value());
    m_slider->setLevelLowLimit(m_levelLowLimitSpin->value());
    m_slider->setLevelHighLimit(m_levelHighLimitSpin->value());
    storeLevelChannels();

    /* Click & Go group */
    /* Color doesn't have a case cause it is calculated
     * in storeLevelChannels */
    if (m_cngNoneCheck->isChecked())
        m_slider->setClickAndGoType(ClickAndGoWidget::None);
    else if (m_cngRGBCheck->isChecked())
        m_slider->setClickAndGoType(ClickAndGoWidget::RGB);
    else if (m_cngCMYCheck->isChecked())
        m_slider->setClickAndGoType(ClickAndGoWidget::CMY);
    else if (m_cngPresetCheck->isChecked())
        m_slider->setClickAndGoType(ClickAndGoWidget::Preset);

    /* Playback page */
    m_slider->setPlaybackFunction(m_playbackFunctionId);

    /* Slider mode */
    if (m_slider->sliderMode() != m_sliderMode)
    {
        m_slider->setSliderMode(VCSlider::SliderMode(m_sliderMode));
        if (m_slider->sliderMode() == VCSlider::Submaster)
        {
            m_slider->setLevelValue(UCHAR_MAX);
            m_slider->setSliderValue(UCHAR_MAX);
        }
    }
    else if (limitDiff && m_slider->sliderMode() == VCSlider::Level)
    {
        // Force the refresh of the slider range
        m_slider->setSliderMode(VCSlider::Level);
    }

    if (m_slider->sliderMode() == VCSlider::Level)
        m_slider->setChannelsMonitorEnabled(m_monitorValuesCheck->isChecked());

    m_slider->setCaption(m_nameEdit->text());

    /* Value style */
    if (m_valueExactRadio->isChecked() == true)
        m_slider->setValueDisplayStyle(VCSlider::ExactValue);
    else
        m_slider->setValueDisplayStyle(VCSlider::PercentageValue);

    /* Slider movement */
    if (m_sliderMovementNormalRadio->isChecked() == true)
        m_slider->setInvertedAppearance(false);
    else
        m_slider->setInvertedAppearance(true);

    /* External input */
    m_slider->setInputSource(m_inputSource);

    /* Close dialog */
    QDialog::accept();
}
