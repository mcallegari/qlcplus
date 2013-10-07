/*
  Q Light Controller
  vcpropertieseditor.cpp

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

#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>

#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "qlcioplugin.h"
#include "qlcfile.h"

#include "vcpropertieseditor.h"
#include "selectinputchannel.h"
#include "virtualconsole.h"
#include "vcproperties.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "vcframe.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCPropertiesEditor::VCPropertiesEditor(QWidget* parent, const VCProperties& properties,
                                       InputMap* inputMap)
    : QDialog(parent)
    , m_inputMap(inputMap)
{
    Q_ASSERT(inputMap != NULL);

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_properties = properties;

    /* General page */
    m_sizeXSpin->setValue(properties.size().width());
    m_sizeYSpin->setValue(properties.size().height());
    fillTapModifierCombo();

    /* Widgets page */
    QSettings settings;
    // ********************* BUTTON ****************************
    QVariant var = settings.value(SETTINGS_BUTTON_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_buttonWspin->setValue(size.width());
        m_buttonHspin->setValue(size.height());
    }
    else
    {
        m_buttonWspin->setValue(50);
        m_buttonHspin->setValue(50);
    }
    // ********************* BUTTON STATUS *********************
    var = settings.value(SETTINGS_BUTTON_STATUSLED);
    if (var.isValid() == true && var.toBool() == true)
            m_buttonStatusLEDRadio->setChecked(true);
    // ********************* SLIDER ****************************
    var = settings.value(SETTINGS_SLIDER_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_sliderWspin->setValue(size.width());
        m_sliderHspin->setValue(size.height());
    }
    else
    {
        m_sliderWspin->setValue(60);
        m_sliderHspin->setValue(200);
    }
    // ********************* SPEED DIAL ************************
    var = settings.value(SETTINGS_SPEEDDIAL_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_speedWspin->setValue(size.width());
        m_speedHspin->setValue(size.height());
    }
    else
    {
        m_speedWspin->setValue(200);
        m_speedHspin->setValue(175);
    }
    // ********************* SPEED DIAL VALUE ******************
    var = settings.value(SETTINGS_SPEEDDIAL_VALUE);
    if (var.isValid() == true)
        m_speedValueEdit->setText(Function::speedToString(var.toUInt()));
    else
        m_speedValueEdit->setText(Function::speedToString(0));

    connect(m_speedValueEdit, SIGNAL(editingFinished()),
            this, SLOT(slotSpeedDialConfirmed()));

    // ********************* XY PAD ****************************
    var = settings.value(SETTINGS_XYPAD_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_xypadWspin->setValue(size.width());
        m_xypadHspin->setValue(size.height());
    }
    else
    {
        m_xypadWspin->setValue(230);
        m_xypadHspin->setValue(230);
    }
    // ********************* CUE LIST **************************
    var = settings.value(SETTINGS_CUELIST_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_cuelistWspin->setValue(size.width());
        m_cuelistHspin->setValue(size.height());
    }
    else
    {
        m_cuelistWspin->setValue(300);
        m_cuelistHspin->setValue(220);
    }
    // ************************ FRAME **************************
    var = settings.value(SETTINGS_FRAME_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_frameWspin->setValue(size.width());
        m_frameHspin->setValue(size.height());
    }
    else
    {
        m_frameWspin->setValue(200);
        m_frameHspin->setValue(200);
    }
    // ********************* SOLO FRAME ************************
    var = settings.value(SETTINGS_SOLOFRAME_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_soloWspin->setValue(size.width());
        m_soloHspin->setValue(size.height());
    }
    else
    {
        m_soloWspin->setValue(200);
        m_soloHspin->setValue(200);
    }
    // ***************** AUDIO TRIGGERS ************************
    var = settings.value(SETTINGS_AUDIOTRIGGERS_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_audioWspin->setValue(size.width());
        m_audioHspin->setValue(size.height());
    }
    else
    {
        m_audioWspin->setValue(200);
        m_audioHspin->setValue(200);
    }

    /* Grand Master page */
    switch (properties.grandMasterChannelMode())
    {
    default:
    case UniverseArray::GMIntensity:
        m_gmIntensityRadio->setChecked(true);
        break;
    case UniverseArray::GMAllChannels:
        m_gmAllChannelsRadio->setChecked(true);
        break;
    }

    switch (properties.grandMasterValueMode())
    {
    default:
    case UniverseArray::GMReduce:
        m_gmReduceRadio->setChecked(true);
        break;
    case UniverseArray::GMLimit:
        m_gmLimitRadio->setChecked(true);
        break;
    }

    switch (properties.grandMasterSlideMode())
    {
    default:
    case UniverseArray::GMNormal:
        m_gmSliderModeNormalRadio->setChecked(true);
        break;
    case UniverseArray::GMInverted:
        m_gmSliderModeInvertedRadio->setChecked(true);
        break;
    }

    updateGrandMasterInputSource();
}

VCPropertiesEditor::~VCPropertiesEditor()
{
}

VCProperties VCPropertiesEditor::properties() const
{
    return m_properties;
}

QSize VCPropertiesEditor::buttonSize()
{
    return QSize(m_buttonWspin->value(), m_buttonHspin->value());
}

bool VCPropertiesEditor::buttonStatusLED()
{
    if (m_buttonStatusLEDRadio->isChecked())
        return true;
    else
        return false;
}

QSize VCPropertiesEditor::sliderSize()
{
    return QSize(m_sliderWspin->value(), m_sliderHspin->value());
}

QSize VCPropertiesEditor::speedDialSize()
{
    return QSize(m_speedWspin->value(), m_speedHspin->value());
}

uint VCPropertiesEditor::speedDialValue()
{
    return Function::stringToSpeed(m_speedValueEdit->text());
}

QSize VCPropertiesEditor::xypadSize()
{
    return QSize(m_xypadWspin->value(), m_xypadHspin->value());
}

QSize VCPropertiesEditor::cuelistSize()
{
    return QSize(m_cuelistWspin->value(), m_cuelistHspin->value());
}

QSize VCPropertiesEditor::frameSize()
{
    return QSize(m_frameWspin->value(), m_frameHspin->value());
}

QSize VCPropertiesEditor::soloFrameSize()
{
    return QSize(m_soloWspin->value(), m_soloHspin->value());
}

QSize VCPropertiesEditor::audioTriggersSize()
{
    return QSize(m_audioWspin->value(), m_audioHspin->value());
}

/*****************************************************************************
 * Layout page
 *****************************************************************************/

void VCPropertiesEditor::fillTapModifierCombo()
{
    QList <int> mods;
    mods << Qt::ShiftModifier;
    mods << Qt::ControlModifier;
    mods << Qt::AltModifier;
    mods << Qt::MetaModifier;

    foreach (int mod, mods)
    {
        QKeySequence seq(mod);
        QString str(seq.toString(QKeySequence::NativeText));
#ifndef __APPLE__
        m_tapModifierCombo->addItem(str.remove(QRegExp("\\W")).trimmed(), mod);
#else
        m_tapModifierCombo->addItem(str, mod);
#endif
        if (mod == int(m_properties.tapModifier()))
            m_tapModifierCombo->setCurrentIndex(m_tapModifierCombo->count() - 1);
    }

    connect(m_tapModifierCombo, SIGNAL(activated(int)),
            this, SLOT(slotTapModifierActivated(int)));
}

void VCPropertiesEditor::slotSizeXChanged(int value)
{
    QSize sz(m_properties.size());
    sz.setWidth(value);
    m_properties.setSize(sz);
}

void VCPropertiesEditor::slotSizeYChanged(int value)
{
    QSize sz(m_properties.size());
    sz.setHeight(value);
    m_properties.setSize(sz);
}

void VCPropertiesEditor::slotTapModifierActivated(int index)
{
    m_properties.setTapModifier(Qt::KeyboardModifier(m_tapModifierCombo->itemData(index).toInt()));
}

void VCPropertiesEditor::slotSpeedDialConfirmed()
{
    if (m_speedValueEdit->text().contains(".") == false)
    {
        m_speedValueEdit->setText(Function::speedToString(m_speedValueEdit->text().toUInt()));
    }
}

/*****************************************************************************
 * Grand Master page
 *****************************************************************************/

void VCPropertiesEditor::slotGrandMasterIntensityToggled(bool checked)
{
    if (checked == true)
        m_properties.setGrandMasterChannelMode(UniverseArray::GMIntensity);
    else
        m_properties.setGrandMasterChannelMode(UniverseArray::GMAllChannels);
}

void VCPropertiesEditor::slotGrandMasterReduceToggled(bool checked)
{
    if (checked == true)
        m_properties.setGrandMasterValueMode(UniverseArray::GMReduce);
    else
        m_properties.setGrandMasterValueMode(UniverseArray::GMLimit);
}

void VCPropertiesEditor::slotGrandMasterSliderNormalToggled(bool checked)
{
    if (checked == true)
        m_properties.setGrandMasterSliderMode(UniverseArray::GMNormal);
    else
        m_properties.setGrandMasterSliderMode(UniverseArray::GMInverted);
}

void VCPropertiesEditor::slotAutoDetectGrandMasterInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_inputMap, SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotGrandMasterInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_inputMap, SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotGrandMasterInputValueChanged(quint32,quint32)));
    }
}

void VCPropertiesEditor::slotGrandMasterInputValueChanged(quint32 universe,
                                                          quint32 channel)
{
    m_properties.setGrandMasterInputSource(universe, channel);
    updateGrandMasterInputSource();
}

void VCPropertiesEditor::slotChooseGrandMasterInputClicked()
{
    SelectInputChannel sic(this, m_inputMap);
    if (sic.exec() == QDialog::Accepted)
    {
        m_properties.setGrandMasterInputSource(sic.universe(), sic.channel());
        updateGrandMasterInputSource();
    }
}

void VCPropertiesEditor::updateGrandMasterInputSource()
{
    QString uniName;
    QString chName;

    if (inputSourceNames(m_properties.grandMasterInputUniverse(),
                         m_properties.grandMasterInputChannel(),
                         uniName, chName) == true)
    {
        /* Display the gathered information */
        m_gmInputUniverseEdit->setText(uniName);
        m_gmInputChannelEdit->setText(chName);
    }
    else
    {
        m_gmInputUniverseEdit->setText(KInputNone);
        m_gmInputChannelEdit->setText(KInputNone);
    }
}

/*****************************************************************************
 * Input Source helper
 *****************************************************************************/

bool VCPropertiesEditor::inputSourceNames(quint32 universe, quint32 channel,
                                          QString& uniName, QString& chName) const
{
    if (universe == InputMap::invalidUniverse() || channel == InputMap::invalidChannel())
    {
        /* Nothing selected for input universe and/or channel */
        return false;
    }

    InputPatch* patch = m_inputMap->patch(universe);
    if (patch == NULL || patch->plugin() == NULL)
    {
        /* There is no patch for the given universe */
        return false;
    }

    QLCInputProfile* profile = patch->profile();
    if (profile == NULL)
    {
        /* There is no profile. Display plugin name and channel number.
           Boring. */
        uniName = patch->plugin()->name();
        chName = tr("%1: Unknown").arg(channel + 1);
    }
    else
    {
        QLCInputChannel* ich;
        QString name;

        /* Display profile name for universe */
        uniName = QString("%1: %2").arg(universe + 1).arg(profile->name());

        /* User can input the channel number by hand, so put something
           rational to the channel name in those cases as well. */
        ich = profile->channel(channel);
        if (ich != NULL)
            name = ich->name();
        else
            name = tr("Unknown");

        /* Display channel name */
        chName = QString("%1: %2").arg(channel + 1).arg(name);
    }

    return true;
}
