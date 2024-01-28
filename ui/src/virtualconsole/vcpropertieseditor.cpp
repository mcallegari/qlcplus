/*
  Q Light Controller
  vcpropertieseditor.cpp

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

#include <QRadioButton>
#include <QSettings>
#include <QCheckBox>
#include <QSpinBox>
#include <QAction>

#include "vcpropertieseditor.h"
#include "selectinputchannel.h"
#include "inputoutputmap.h"
#include "qlcinputsource.h"
#include "vcproperties.h"
#include "inputpatch.h"
#include "function.h"

#define SETTINGS_GEOMETRY "vcpropertieseditor/geometry"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCPropertiesEditor::VCPropertiesEditor(QWidget* parent, const VCProperties& properties,
                                       InputOutputMap *ioMap)
    : QDialog(parent)
    , m_ioMap(ioMap)
{
    Q_ASSERT(ioMap != NULL);

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_properties = properties;

    /* General page */
    m_sizeXSpin->setValue(properties.size().width());
    m_sizeYSpin->setValue(properties.size().height());

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
    // ***************** RGB MATRIX ************************
    var = settings.value(SETTINGS_RGBMATRIX_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_matrixWspin->setValue(size.width());
        m_matrixHspin->setValue(size.height());
    }
    else
    {
        m_matrixWspin->setValue(160);
        m_matrixHspin->setValue(120);
    }

    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    /* Grand Master page */
    switch (properties.grandMasterChannelMode())
    {
    default:
    case GrandMaster::Intensity:
        m_gmIntensityRadio->setChecked(true);
        break;
    case GrandMaster::AllChannels:
        m_gmAllChannelsRadio->setChecked(true);
        break;
    }

    switch (properties.grandMasterValueMode())
    {
    default:
    case GrandMaster::Reduce:
        m_gmReduceRadio->setChecked(true);
        break;
    case GrandMaster::Limit:
        m_gmLimitRadio->setChecked(true);
        break;
    }

    switch (properties.grandMasterSlideMode())
    {
    default:
    case GrandMaster::Normal:
        m_gmSliderModeNormalRadio->setChecked(true);
        break;
    case GrandMaster::Inverted:
        m_gmSliderModeInvertedRadio->setChecked(true);
        break;
    }

    updateGrandMasterInputSource();
}

VCPropertiesEditor::~VCPropertiesEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
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

QSize VCPropertiesEditor::rgbMatrixSize()
{
    return QSize(m_matrixWspin->value(), m_matrixHspin->value());
}

/*****************************************************************************
 * Layout page
 *****************************************************************************/

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
        m_properties.setGrandMasterChannelMode(GrandMaster::Intensity);
    else
        m_properties.setGrandMasterChannelMode(GrandMaster::AllChannels);
}

void VCPropertiesEditor::slotGrandMasterReduceToggled(bool checked)
{
    if (checked == true)
        m_properties.setGrandMasterValueMode(GrandMaster::Reduce);
    else
        m_properties.setGrandMasterValueMode(GrandMaster::Limit);
}

void VCPropertiesEditor::slotGrandMasterSliderNormalToggled(bool checked)
{
    if (checked == true)
        m_properties.setGrandMasterSliderMode(GrandMaster::Normal);
    else
        m_properties.setGrandMasterSliderMode(GrandMaster::Inverted);
}

void VCPropertiesEditor::slotAutoDetectGrandMasterInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_ioMap, SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotGrandMasterInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_ioMap, SIGNAL(inputValueChanged(quint32,quint32,uchar)),
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
    SelectInputChannel sic(this, m_ioMap);
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

    if (m_ioMap->inputSourceNames(
                QSharedPointer<QLCInputSource>(
                    new QLCInputSource(
                        m_properties.grandMasterInputUniverse(),
                        m_properties.grandMasterInputChannel())),
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
