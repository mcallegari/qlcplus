/*
  Q Light Controller
  grandmasterslider.cpp

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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <cmath>

#include "grandmasterslider.h"
#include "virtualconsole.h"
#include "vcproperties.h"
#include "outputmap.h"
#include "inputmap.h"
#include "apputil.h"

GrandMasterSlider::GrandMasterSlider(QWidget* parent, OutputMap* outputMap, InputMap* inputMap)
    : QFrame(parent)
    , m_outputMap(outputMap)
    , m_inputMap(inputMap)
{
    Q_ASSERT(outputMap != NULL);
    Q_ASSERT(inputMap != NULL);

    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    setMinimumSize(QSize(40, 100));
    setMaximumSize(QSize(40, USHRT_MAX));

    new QVBoxLayout(this);
    layout()->setMargin(2);

    m_valueLabel = new QLabel(this);
    m_valueLabel->setAlignment(Qt::AlignHCenter);
    layout()->addWidget(m_valueLabel);

    m_slider = new QSlider(this);
    m_slider->setRange(0, UCHAR_MAX);
    m_slider->setStyle(AppUtil::saneStyle());
    m_slider->setMinimumSize(QSize(30, 50));
    m_slider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    layout()->addWidget(m_slider);
    layout()->setAlignment(m_slider, Qt::AlignHCenter);
    m_slider->setValue(255);
    connect(m_slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotValueChanged(int)));

    m_nameLabel = new QLabel(this);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setAlignment(Qt::AlignHCenter);
    m_nameLabel->setText(tr("GM"));
    layout()->addWidget(m_nameLabel);

    /* Listen to GM value changes */
    connect(m_outputMap, SIGNAL(grandMasterValueChanged(uchar)),
            this, SLOT(slotGrandMasterValueChanged(uchar)));
    connect(m_outputMap, SIGNAL(grandMasterValueModeChanged(GrandMaster::GMValueMode)),
            this, SLOT(slotGrandMasterValueModeChanged(GrandMaster::GMValueMode)));

    /* External input connection */
    connect(m_inputMap, SIGNAL(inputValueChanged(quint32, quint32, uchar)),
            this, SLOT(slotInputValueChanged(quint32, quint32, uchar)));

    updateTooltip();
    updateDisplayValue();
}

GrandMasterSlider::~GrandMasterSlider()
{
}

bool GrandMasterSlider::invertedAppearance() const
{
    Q_ASSERT(m_slider != NULL);
    return m_slider->invertedAppearance();
}

void GrandMasterSlider::setInvertedAppearance(bool invert)
{
    Q_ASSERT(m_slider != NULL);
    m_slider->setInvertedAppearance(invert);
}

void GrandMasterSlider::slotValueChanged(int value)
{
    // Update display value
    updateDisplayValue();

    // Avoid double calls triggered by slotGrandMasterValueChanged
    int curval = m_outputMap->grandMasterValue();
    if(value != curval)
    {
        // Write new grand master value to universes
        m_outputMap->setGrandMasterValue(value);
    }
}

void GrandMasterSlider::updateTooltip()
{
    QString tooltip;

    switch (m_outputMap->grandMasterValueMode())
    {
        case GrandMaster::GMLimit:
            tooltip += tr("Grand Master <B>limits</B> the maximum value of");
            break;
        case GrandMaster::GMReduce:
            tooltip += tr("Grand Master <B>reduces</B> the current value of");
            break;
    }

    tooltip += QString(" ");

    switch (m_outputMap->grandMasterChannelMode())
    {
        case GrandMaster::GMIntensity:
            tooltip += tr("intensity channels");
            break;
        case GrandMaster::GMAllChannels:
            tooltip += tr("all channels");
            break;
    }

    setToolTip(tooltip);
}

void GrandMasterSlider::updateDisplayValue()
{
    int value = m_slider->value();
    QString str;
    if (m_outputMap->grandMasterValueMode() == GrandMaster::GMLimit)
    {
        str = QString("%1").arg(value, 3, 10, QChar('0'));
    }
    else
    {
        int p = floor(((double(value) / double(UCHAR_MAX)) * double(100)) + 0.5);
        str = QString("%1%").arg(p, 2, 10, QChar('0'));
    }
    m_valueLabel->setText(str);
}

void GrandMasterSlider::slotGrandMasterValueChanged(uchar value)
{
    m_slider->setValue(value);
}

void GrandMasterSlider::slotGrandMasterValueModeChanged(GrandMaster::GMValueMode mode)
{
    Q_UNUSED(mode);
    updateTooltip();
    updateDisplayValue();
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void GrandMasterSlider::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    if (universe == VirtualConsole::instance()->properties().grandMasterInputUniverse() &&
        channel == VirtualConsole::instance()->properties().grandMasterInputChannel())
    {
        m_slider->setValue(value);
    }
}

