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
#include "clickandgoslider.h"
#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "vcproperties.h"
#include "inputpatch.h"
#include "apputil.h"

GrandMasterSlider::GrandMasterSlider(QWidget* parent, InputOutputMap *ioMap)
    : QFrame(parent)
    , m_ioMap(ioMap)
{
    Q_ASSERT(ioMap != NULL);

    QString sStyle = AppUtil::getStyleSheet("GRANDMASTER");
    if (sStyle.isEmpty())
        sStyle = "QFrame { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D6D2D0, stop: 1 #AFACAB); "
                 "border: 1px solid gray; border-radius: 4px; }";
    setStyleSheet(sStyle);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    setMinimumSize(QSize(40, 100));
    setMaximumSize(QSize(40, USHRT_MAX));

    new QVBoxLayout(this);
    layout()->setContentsMargins(2, 2, 2, 2);

    m_valueLabel = new QLabel(this);
    m_valueLabel->setAlignment(Qt::AlignHCenter);
    m_valueLabel->setStyleSheet("QFrame { background-color: transparent; border: 0px; border-radius: 0px; }");
    layout()->addWidget(m_valueLabel);

    m_slider = new ClickAndGoSlider(this);
    m_slider->setRange(0, UCHAR_MAX);
    m_slider->setStyleSheet(
        "QSlider::groove:vertical { background: transparent; width: 28px; } "

        "QSlider::handle:vertical { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #A81919, stop:0.45 #DB2020, stop:0.50 #000, stop:0.55 #DB2020, stop:1 #A81919);"
        "border: 1px solid #5c5c5c;"
        "border-radius: 4px; margin: 0 -1px; height: 20px; }"

        "QSlider::handle:vertical:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #DB2020, stop:0.45 #F51C1C, stop:0.50 #fff, stop:0.55 #F51C1C, stop:1 #DB2020);"
        "border: 1px solid #000; }"

        "QSlider::add-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #78d, stop: 1 #97CDEC );"
        "border: 1px solid #5288A7; margin: 0 11px; }"

        "QSlider::sub-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #888, stop: 1 #ddd );"
        "border: 1px solid #8E8A86; margin: 0 11px; }"

        "QSlider::handle:vertical:disabled { background: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ddd, stop:0.45 #888, stop:0.50 #444, stop:0.55 #888, stop:1 #999);"
        "border: 1px solid #666; }"
        );
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
    connect(m_ioMap, SIGNAL(grandMasterValueChanged(uchar)),
            this, SLOT(slotGrandMasterValueChanged(uchar)));
    connect(m_ioMap, SIGNAL(grandMasterValueModeChanged(GrandMaster::ValueMode)),
            this, SLOT(slotGrandMasterValueModeChanged(GrandMaster::ValueMode)));

    /* External input connection */
    connect(m_ioMap, SIGNAL(inputValueChanged(quint32, quint32, uchar)),
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
    sendFeedback();
}

void GrandMasterSlider::slotValueChanged(int value)
{
    // Update display value
    updateDisplayValue();

    // Avoid double calls triggered by slotGrandMasterValueChanged
    int curval = m_ioMap->grandMasterValue();
    if (value != curval)
    {
        // Write new grand master value to universes
        m_ioMap->setGrandMasterValue(value);
    }
}

void GrandMasterSlider::updateTooltip()
{
    QString tooltip;

    switch (m_ioMap->grandMasterValueMode())
    {
        case GrandMaster::Limit:
            tooltip += tr("Grand Master <B>limits</B> the maximum value of");
            break;
        case GrandMaster::Reduce:
            tooltip += tr("Grand Master <B>reduces</B> the current value of");
            break;
    }

    tooltip += QString(" ");

    switch (m_ioMap->grandMasterChannelMode())
    {
        case GrandMaster::Intensity:
            tooltip += tr("intensity channels");
            break;
        case GrandMaster::AllChannels:
            tooltip += tr("all channels");
            break;
    }

    setToolTip(tooltip);
}

void GrandMasterSlider::updateDisplayValue()
{
    int value = m_slider->value();
    QString str;
    if (m_ioMap->grandMasterValueMode() == GrandMaster::Limit)
    {
        str = QString("%1").arg(value, 3, 10, QChar('0'));
    }
    else
    {
        int p = floor(((double(value) / double(UCHAR_MAX)) * double(100)) + 0.5);
        str = QString("%1%").arg(p, 2, 10, QChar('0'));
    }
    m_valueLabel->setText(str);
    sendFeedback();
}

void GrandMasterSlider::slotGrandMasterValueChanged(uchar value)
{
    m_slider->blockSignals(true);
    m_slider->setValue(value);
    m_slider->blockSignals(false);

    updateDisplayValue();
}

void GrandMasterSlider::slotGrandMasterValueModeChanged(GrandMaster::ValueMode mode)
{
    Q_UNUSED(mode);
    updateTooltip();
    updateDisplayValue();
}

void GrandMasterSlider::sendFeedback()
{
    quint32 universe = VirtualConsole::instance()->properties().grandMasterInputUniverse();
    quint32 channel = VirtualConsole::instance()->properties().grandMasterInputChannel();
    QString chName;

    if (universe == InputOutputMap::invalidUniverse() || channel == QLCChannel::invalid())
        return;

    InputPatch* pat = m_ioMap->inputPatch(universe);
    if (pat != NULL)
    {
        QLCInputProfile* profile = pat->profile();
        if (profile != NULL)
        {
            QLCInputChannel* ich = profile->channel(channel);
            if (ich != NULL)
                chName = ich->name();
        }
    }
    if (m_slider->invertedAppearance())
        m_ioMap->sendFeedBack(universe, channel, UCHAR_MAX - m_slider->value(), chName);
    else
        m_ioMap->sendFeedBack(universe, channel, m_slider->value(), chName);
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

