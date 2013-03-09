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
